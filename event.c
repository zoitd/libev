#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef EV_EVENT_H
#include EV_EVENT_H
#else
#include "event.h"
#endif

#if EV_MULTIPLICITY
#define dLOOPev struct ev_loop *loop = (struct ev_loop *)ev->ev_base
#define dLOOPbase struct ev_loop *loop = (struct ev_loop *)base
#else
#define dLOOPev
#define dLOOPbase
#endif

/* 从未被访问过，始终会与ev_loop类型进行相互转换 */
struct event_base
{
    int dummy;
};

#define ev_tstamp double

static struct event_base *ev_x_cur;

/*
 * 将timeval结构转换为ev_tstamp类型的时间值
 * @param tv 输入的时间结构指针，可为NULL
 * @return 转换后的时间值(秒)，如果tv为NULL返回-1
 */
static ev_tstamp ev_tv_get(struct timeval *tv)
{
    if (tv)
    {
        ev_tstamp after = tv->tv_sec + tv->tv_usec * 1e-6;
        return after ? after : 1e-6;
    }
    else
        return -1.;
}

#define EVENT_STRINGIFY(s) #s
#define EVENT_VERSION(a, b) EVENT_STRINGIFY(a) "." EVENT_STRINGIFY(b)

/*
 * 获取libev的版本号字符串
 * @return 返回版本号字符串(格式为"主版本号.次版本号")
 */
const char *event_get_version(void)
{
    /* returns ABI, not API or library, version */
    return EVENT_VERSION(EV_VERSION_MAJOR, EV_VERSION_MINOR);
}

/*
 * 获取事件循环实现的方法名称
 * @return 返回"libev"字符串
 */
inline const char *event_get_method(void)
{
    return "libev";
}

/*
 * 初始化事件循环
 * @return 返回事件循环基址指针
 * @note 非线程安全，遵循libevent的工作方式
 */
void *event_init(void)
{
#if EV_MULTIPLICITY
    if (ev_x_cur)
        ev_x_cur = (struct event_base *)ev_loop_new(EVFLAG_AUTO);
    else
        ev_x_cur = (struct event_base *)ev_default_loop(EVFLAG_AUTO);
#else
    assert(("libev: multiple event bases not supported when not compiled with EV_MULTIPLICITY", !ev_x_cur));

    ev_x_cur = (struct event_base *)(long)ev_default_loop(EVFLAG_AUTO);
#endif

    return ev_x_cur;
}

/*
 * 获取事件循环实现的方法名称
 * @param base 事件循环基址指针
 * @return 返回"libev"字符串
 */
inline const char *event_base_get_method(const struct event_base *base)
{
    return "libev";
}

/*
 * 创建新的事件循环
 * @return 返回新事件循环基址指针，失败返回NULL
 * @note 未编译EV_MULTIPLICITY时不支持多事件循环
 */
struct event_base *event_base_new(void)
{
#if EV_MULTIPLICITY
    return (struct event_base *)ev_loop_new(EVFLAG_AUTO);
#else
    assert(("libev: multiple event bases not supported when not compiled with EV_MULTIPLICITY"));
    return NULL;
#endif
}

/*
 * 释放事件循环资源
 * @param base 要释放的事件循环基址指针
 */
void event_base_free(struct event_base *base)
{
    dLOOPbase;

#if EV_MULTIPLICITY
    if (!ev_is_default_loop(loop))
        ev_loop_destroy(loop);
#endif
}

/*
 * 启动默认事件循环
 * @return 成功返回0，失败返回非0
 */
int event_dispatch(void)
{
    return event_base_dispatch(ev_x_cur);
}

#ifdef EV_STANDALONE
void event_set_log_callback(event_log_cb cb)
{
    /* nop */
}
#endif

/*
 * 启动事件循环
 * @param flags 控制循环行为的标志位
 * @return 成功返回0，失败返回非0
 */
int event_loop(int flags)
{
    return event_base_loop(ev_x_cur, flags);
}

/*
 * 在指定时间后退出事件循环
 * @param tv 指定退出时间的timeval结构指针
 * @return 成功返回0，失败返回非0
 */
int event_loopexit(struct timeval *tv)
{
    return event_base_loopexit(ev_x_cur, tv);
}

/*
 * 获取事件的回调函数
 * @param ev 事件指针
 * @return 返回事件回调函数指针
 */
event_callback_fn event_get_callback(const struct event *ev)
{
    return ev->ev_callback;
}

static void ev_x_cb(struct event *ev, int revents)
{
    revents &= EV_READ | EV_WRITE | EV_TIMER | EV_SIGNAL;

    ev->ev_res = revents;
    ev->ev_callback(ev->ev_fd, (short)revents, ev->ev_arg);
}

static void ev_x_cb_sig(struct ev_loop *loop, struct ev_signal *w, int revents)
{
    struct event *ev = (struct event *)(((char *)w) - offsetof(struct event, iosig.sig));

    if (revents & EV_ERROR)
        event_del(ev);

    ev_x_cb(ev, revents);
}

static void ev_x_cb_io(struct ev_loop *loop, struct ev_io *w, int revents)
{
    struct event *ev = (struct event *)(((char *)w) - offsetof(struct event, iosig.io));

    if ((revents & EV_ERROR) || !(ev->ev_events & EV_PERSIST))
        event_del(ev);

    ev_x_cb(ev, revents);
}

static void ev_x_cb_to(struct ev_loop *loop, struct ev_timer *w, int revents)
{
    struct event *ev = (struct event *)(((char *)w) - offsetof(struct event, to));

    event_del(ev);

    ev_x_cb(ev, revents);
}

/*
 * 设置事件参数
 * @param ev 要设置的事件指针
 * @param fd 关联的文件描述符
 * @param events 关注的事件类型(如EV_READ|EV_WRITE)
 * @param cb 事件触发时的回调函数
 * @param arg 传递给回调函数的参数
 */
void event_set(struct event *ev, int fd, short events, void (*cb)(int, short, void *), void *arg)
{
    if (events & EV_SIGNAL)
        ev_init(&ev->iosig.sig, ev_x_cb_sig);
    else
        ev_init(&ev->iosig.io, ev_x_cb_io);

    ev_init(&ev->to, ev_x_cb_to);

    ev->ev_base = ev_x_cur; /* not threadsafe, but it's how libevent works */
    ev->ev_fd = fd;
    ev->ev_events = events;
    ev->ev_pri = 0;
    ev->ev_callback = cb;
    ev->ev_arg = arg;
    ev->ev_res = 0;
    ev->ev_flags = EVLIST_INIT;
}

/*
 * 设置一次性事件
 * @param fd 关联的文件描述符
 * @param events 关注的事件类型
 * @param cb 事件触发时的回调函数
 * @param arg 传递给回调函数的参数
 * @param tv 超时时间结构指针
 * @return 成功返回0，失败返回非0
 */
int event_once(int fd, short events, void (*cb)(int, short, void *), void *arg, struct timeval *tv)
{
    return event_base_once(ev_x_cur, fd, events, cb, arg, tv);
}

/*
 * 添加事件到事件循环
 * @param ev 要添加的事件指针
 * @param tv 超时时间结构指针(可选)
 * @return 成功返回0，失败返回非0
 */
int event_add(struct event *ev, struct timeval *tv)
{
    dLOOPev;

    if (ev->ev_events & EV_SIGNAL)
    {
        if (!ev_is_active(&ev->iosig.sig))
        {
            ev_signal_set(&ev->iosig.sig, ev->ev_fd);
            ev_signal_start(loop, &ev->iosig.sig);

            ev->ev_flags |= EVLIST_SIGNAL;
        }
    }
    else if (ev->ev_events & (EV_READ | EV_WRITE))
    {
        if (!ev_is_active(&ev->iosig.io))
        {
            ev_io_set(&ev->iosig.io, ev->ev_fd, ev->ev_events & (EV_READ | EV_WRITE));
            ev_io_start(loop, &ev->iosig.io);

            ev->ev_flags |= EVLIST_INSERTED;
        }
    }

    if (tv)
    {
        ev->to.repeat = ev_tv_get(tv);
        ev_timer_again(loop, &ev->to);
        ev->ev_flags |= EVLIST_TIMEOUT;
    }
    else
    {
        ev_timer_stop(loop, &ev->to);
        ev->ev_flags &= ~EVLIST_TIMEOUT;
    }

    ev->ev_flags |= EVLIST_ACTIVE;

    return 0;
}

/*
 * 从事件循环中删除事件
 * @param ev 要删除的事件指针
 * @return 成功返回0，失败返回非0
 */
int event_del(struct event *ev)
{
    dLOOPev;

    if (ev->ev_events & EV_SIGNAL)
        ev_signal_stop(loop, &ev->iosig.sig);
    else if (ev->ev_events & (EV_READ | EV_WRITE))
        ev_io_stop(loop, &ev->iosig.io);

    if (ev_is_active(&ev->to))
        ev_timer_stop(loop, &ev->to);

    ev->ev_flags = EVLIST_INIT;

    return 0;
}

/*
 * 手动激活事件
 * @param ev 要激活的事件指针
 * @param res 触发的事件类型
 * @param ncalls 保留参数(未使用)
 */
void event_active(struct event *ev, int res, short ncalls)
{
    dLOOPev;

    if (res & EV_TIMEOUT)
        ev_feed_event(loop, &ev->to, res & EV_TIMEOUT);

    if (res & EV_SIGNAL)
        ev_feed_event(loop, &ev->iosig.sig, res & EV_SIGNAL);

    if (res & (EV_READ | EV_WRITE))
        ev_feed_event(loop, &ev->iosig.io, res & (EV_READ | EV_WRITE));
}

/*
 * 检查事件是否处于待处理状态
 * @param ev 要检查的事件指针
 * @param events 要检查的事件类型
 * @param tv 用于存储超时时间的结构指针(可选)
 * @return 返回匹配的事件类型
 */
int event_pending(struct event *ev, short events, struct timeval *tv)
{
    short revents = 0;
    dLOOPev;

    if (ev->ev_events & EV_SIGNAL)
    {
        /* sig */
        if (ev_is_active(&ev->iosig.sig) || ev_is_pending(&ev->iosig.sig))
            revents |= EV_SIGNAL;
    }
    else if (ev->ev_events & (EV_READ | EV_WRITE))
    {
        /* io */
        if (ev_is_active(&ev->iosig.io) || ev_is_pending(&ev->iosig.io))
            revents |= ev->ev_events & (EV_READ | EV_WRITE);
    }

    if (ev->ev_events & EV_TIMEOUT || ev_is_active(&ev->to) || ev_is_pending(&ev->to))
    {
        revents |= EV_TIMEOUT;

        if (tv)
        {
            ev_tstamp at = ev_now(loop);

            tv->tv_sec = (long)at;
            tv->tv_usec = (long)((at - (ev_tstamp)tv->tv_sec) * 1e6);
        }
    }

    return events & revents;
}

/*
 * 初始化事件优先级系统
 * @param npri 优先级数量
 * @return 成功返回0，失败返回非0
 */
int event_priority_init(int npri)
{
    return event_base_priority_init(ev_x_cur, npri);
}

/*
 * 设置事件优先级
 * @param ev 要设置的事件指针
 * @param pri 优先级值
 * @return 成功返回0，失败返回非0
 */
int event_priority_set(struct event *ev, int pri)
{
    ev->ev_pri = pri;

    return 0;
}

/*
 * 设置事件所属的事件循环
 * @param base 事件循环基址指针
 * @param ev 要设置的事件指针
 * @return 成功返回0，失败返回非0
 */
int event_base_set(struct event_base *base, struct event *ev)
{
    ev->ev_base = base;

    return 0;
}

/*
 * 启动指定事件循环
 * @param base 事件循环基址指针
 * @param flags 控制循环行为的标志位
 * @return 成功返回0，失败返回非0
 */
int event_base_loop(struct event_base *base, int flags)
{
    dLOOPbase;

    return !ev_run(loop, flags);
}

/*
 * 启动指定事件循环(默认标志位)
 * @param base 事件循环基址指针
 * @return 成功返回0，失败返回非0
 */
int event_base_dispatch(struct event_base *base)
{
    return event_base_loop(base, 0);
}

static void ev_x_loopexit_cb(int revents, void *base)
{
    dLOOPbase;

    ev_break(loop, EVBREAK_ONE);
}

/*
 * 在指定时间后退出指定事件循环
 * @param base 事件循环基址指针
 * @param tv 指定退出时间的timeval结构指针
 * @return 成功返回0，失败返回非0
 */
int event_base_loopexit(struct event_base *base, struct timeval *tv)
{
    ev_tstamp after = ev_tv_get(tv);
    dLOOPbase;

    ev_once(loop, -1, 0, after >= 0. ? after : 0., ev_x_loopexit_cb, (void *)base);

    return 0;
}

struct ev_x_once
{
    int fd;
    void (*cb)(int, short, void *);
    void *arg;
};

static void ev_x_once_cb(int revents, void *arg)
{
    struct ev_x_once *once = (struct ev_x_once *)arg;

    once->cb(once->fd, (short)revents, once->arg);
    free(once);
}

/*
 * 在指定事件循环上设置一次性事件
 * @param base 事件循环基址指针
 * @param fd 关联的文件描述符
 * @param events 关注的事件类型
 * @param cb 事件触发时的回调函数
 * @param arg 传递给回调函数的参数
 * @param tv 超时时间结构指针
 * @return 成功返回0，失败返回非0
 */
int event_base_once(struct event_base *base, int fd, short events, void (*cb)(int, short, void *), void *arg, struct timeval *tv)
{
    struct ev_x_once *once = (struct ev_x_once *)malloc(sizeof(struct ev_x_once));
    dLOOPbase;

    if (!once)
        return -1;

    once->fd = fd;
    once->cb = cb;
    once->arg = arg;

    ev_once(loop, fd, events & (EV_READ | EV_WRITE), ev_tv_get(tv), ev_x_once_cb, (void *)once);

    return 0;
}

/*
 * 初始化指定事件循环的优先级系统
 * @param base 事件循环基址指针
 * @param npri 优先级数量
 * @return 成功返回0，失败返回非0
 */
int event_base_priority_init(struct event_base *base, int npri)
{
    /*dLOOPbase;*/

    return 0;
}
