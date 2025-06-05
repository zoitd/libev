/*
 * 关于kqueue的总体说明：
 *
 * a) kqueue作为BSD系统的事件通知机制，相比epoll具有更简洁的API设计和更一致的行为。
 * b) kqueue能够监控多种事件源（文件、socket、信号、进程等），而epoll仅限于文件描述符。
 * c) kqueue提供的过滤器（filter）机制比epoll的事件掩码更灵活，能更精确地指定感兴趣的事件。
 * d) kqueue与epoll一样会静默地从集合中移除已关闭的文件描述符，但重新添加的代价较小。
 * e) kqueue在处理fork特性上有特殊问题，某些BSD内核在fork后会不仅销毁kqueue实例，
 *    还会关闭其文件描述符，这一特性需要特殊处理。
 * f) kqueue支持EV_CLEAR标志实现边缘触发，类似于epoll的EPOLLET，但设计更加清晰。
 * g) kqueue的错误处理更加直观，通过事件标志EV_ERROR直接报告问题。
 *
 * 本文件实现了libev对kqueue后端的支持，处理了BSD特有的行为特性，并确保在各种
 * BSD变种（FreeBSD、NetBSD、OpenBSD、Darwin/macOS）上的兼容性。
 */

#include <errno.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include "ev.h"
#include "ev_wrap.h"
#include "ev_vars.h"
typedef double ev_tstamp;

/*
 * 向kqueue更改队列添加一个事件更改请求
 * @param loop 事件循环实例
 * @param fd 要监控的文件描述符
 * @param filter 要应用的过滤器类型（EVFILT_READ或EVFILT_WRITE）
 * @param flags 操作标志（EV_ADD、EV_DELETE等）
 * @param fflags 过滤器特定标志（如NOTE_EOF）
 *
 * 该函数将新的事件更改请求添加到更改队列中，但不会立即提交到内核。
 * 所有更改会在下一次poll操作时批量提交，这种设计可以减少系统调用次数。
 */

[[maybe_unused]] void static kqueue_change(struct ev_loop *loop, int fd, int filter, int flags, int fflags)
{
    ++loop->kqueue_changecnt;
    // 增加计数器
    // ++kqueue_changecnt;
    /*
    kevent: 事件类型，
    kqueue_changes: evlib中的事件数组的基地址
    kqueue_changemax: 最大事件数
    kqueue_changecnt: 当前事件数
    EMPTY2: 接受两个参数，但是不执行任何操作。用于消除某些警告
    */
    array_needsize<kevent>(kqueue_changes, kqueue_changemax, kqueue_changemax);
    // array_needsize(struct kevent, kqueue_changes, kqueue_changemax, kqueue_changemax, EMPTY2);
    // 为新增的变量设置值。
    EV_SET(&kqueue_changes[kqueue_changecnt - 1], fd, filter, flags, fflags, 0, 0);
}

/* OS X at least needs this */
#ifndef EV_ENABLE
#define EV_ENABLE 0
#endif
#ifndef NOTE_EOF
#define NOTE_EOF 0
#endif

/*
 * 修改文件描述符的kqueue监控事件
 * @param loop 事件循环实例
 * @param fd 要修改的文件描述符
 * @param oev 原事件掩码
 * @param nev 新事件掩码
 *
 * 该函数处理kqueue事件注册的核心逻辑，包括：
 * - 当事件变化时删除旧的监控事件
 * - 为需要监控的事件添加新的监控请求
 * - 即使事件未变化也重新添加请求，以可靠检测文件描述符关闭/重新打开
 * - 使用NOTE_EOF标志来检测连接关闭
 *
 * 与epoll不同，kqueue允许单独添加/删除读写事件，而不需要同时指定完整的事件集合，
 * 这简化了事件管理并减少了出错可能性。
 */
static void kqueue_modify(struct ev_loop *loop, int fd, int oev, int nev)
{
    if (oev != nev)
    {
        if (oev & EV_READ)
            kqueue_change(loop, fd, EVFILT_READ, EV_DELETE, 0);

        if (oev & EV_WRITE)
            kqueue_change(loop, fd, EVFILT_WRITE, EV_DELETE, 0);
    }

    /* to detect close/reopen reliably, we have to re-add */
    /* event requests even when oev == nev */

    if (nev & EV_READ)
        kqueue_change(loop, fd, EVFILT_READ, EV_ADD | EV_ENABLE, NOTE_EOF);

    if (nev & EV_WRITE)
        kqueue_change(loop, fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, NOTE_EOF);
}

/*
 * 执行kevent调用并处理返回的事件
 * @param loop 事件循环实例
 * @param timeout 最大等待时间(秒)
 *
 * 该函数负责：
 * - 确保事件数组足够大以容纳所有更改和可能的错误
 * - 调用kevent系统调用提交更改并获取就绪事件
 * - 处理错误事件（kqueue将错误直接作为事件返回）
 * - 分发读写事件给相应的处理函数
 * - 动态调整事件数组大小
 *
 * kqueue的一个优势是将事件更改和事件获取合并为一个系统调用，
 * 并直接通过事件返回值报告错误，这种设计比epoll更直观且效率更高。
 */
static void kqueue_poll(struct ev_loop *loop, ev_tstamp timeout)
{
    int res, i;
    struct timespec ts;

    /* need to resize so there is enough space for errors */
    if (kqueue_changecnt > kqueue_eventmax)
    {
        ev_free(kqueue_events);
        kqueue_eventmax = array_nextsize(sizeof(struct kevent), kqueue_eventmax, kqueue_changecnt);
        kqueue_events = (struct kevent *)ev_malloc(sizeof(struct kevent) * kqueue_eventmax);
    }

    EV_RELEASE_CB;
    EV_TS_SET(ts, timeout);
    res = kevent(backend_fd, kqueue_changes, kqueue_changecnt, kqueue_events, kqueue_eventmax, &ts);
    EV_ACQUIRE_CB;
    kqueue_changecnt = 0;

    if (expect_false(res < 0))
    {
        if (errno != EINTR)
            ev_syserr("(libev) kevent");

        return;
    }

    for (i = 0; i < res; ++i)
    {
        int fd = kqueue_events[i].ident;

        if (expect_false(kqueue_events[i].flags & EV_ERROR))
        {
            int err = kqueue_events[i].data;

            /* we are only interested in errors for fds that we are interested in :) */
            if (anfds[fd].events)
            {
                if (err == ENOENT) /* resubmit changes on ENOENT */
                    kqueue_modify(loop, fd, 0, anfds[fd].events);
                else if (err == EBADF) /* on EBADF, we re-check the fd */
                {
                    if (fd_valid(fd))
                        kqueue_modify(loop, fd, 0, anfds[fd].events);
                    else
                        fd_kill(loop, fd);
                }
                else /* on all other errors, we error out on the fd */
                    fd_kill(loop, fd);
            }
        }
        else
            fd_event(
                loop,
                fd,
                kqueue_events[i].filter == EVFILT_READ    ? EV_READ
                : kqueue_events[i].filter == EVFILT_WRITE ? EV_WRITE
                                                          : 0);
    }

    if (expect_false(res == kqueue_eventmax))
    {
        ev_free(kqueue_events);
        kqueue_eventmax = array_nextsize(sizeof(struct kevent), kqueue_eventmax, kqueue_eventmax + 1);
        kqueue_events = (struct kevent *)ev_malloc(sizeof(struct kevent) * kqueue_eventmax);
    }
}

/*
 * 初始化kqueue后端
 * @param loop 事件循环实例
 * @param flags 初始化标志
 * @return 成功返回EVBACKEND_KQUEUE，失败返回0
 *
 * 该函数负责：
 * - 创建kqueue文件描述符并记录当前进程ID
 * - 设置CLOEXEC标志防止fd泄漏
 * - 设置后端函数指针和最小时间精度
 * - 初始化事件数组和更改队列
 *
 * kqueue在时间精度方面优于epoll，支持纳秒级精度(1e-9)，
 * 而epoll受限于毫秒级(1e-3)。
 */
int static inline kqueue_init(struct ev_loop *loop, int flags)
{
    /* initialize the kernel queue */
    kqueue_fd_pid = getpid();
    if ((backend_fd = kqueue()) < 0)
        return 0;

    fcntl(backend_fd, F_SETFD, FD_CLOEXEC); /* not sure if necessary, hopefully doesn't hurt */

    backend_mintime = 1e-9; /* apparently, they did the right thing in freebsd */
    backend_modify = kqueue_modify;
    backend_poll = kqueue_poll;

    kqueue_eventmax = 64; /* initial number of events receivable per poll */
    kqueue_events = (struct kevent *)ev_malloc(sizeof(struct kevent) * kqueue_eventmax);

    kqueue_changes = 0;
    kqueue_changemax = 0;
    kqueue_changecnt = 0;

    return EVBACKEND_KQUEUE;
}

/*
 * 销毁kqueue后端资源
 * @param loop 事件循环实例
 *
 * 释放分配的事件数组和更改队列数组
 */
void static inline kqueue_destroy(struct ev_loop *loop)
{
    ev_free(kqueue_events);
    ev_free(kqueue_changes);
}

/*
 * 处理fork后的kqueue状态重建
 * @param loop 事件循环实例
 *
 * 该函数处理fork后kqueue的特殊情况，主要解决BSD内核对kqueue处理的不一致性：
 * - 记录并检查进程ID变化，以处理BSD特有的kqueue fork行为
 * - 仅在进程ID未变化时关闭旧fd，避免某些BSD变种中的双重关闭问题
 * - 创建新的kqueue实例
 * - 重新注册所有文件描述符监听事件
 *
 * 与epoll不同，kqueue在fork后的行为在不同BSD变种间存在差异：
 * - 某些BSD实现在fork后会自动关闭子进程中的kqueue fd
 * - 而其他实现则保持fd打开但使其无效
 * 这种不一致性需要特殊处理逻辑来避免fd泄漏或双重关闭错误。
 */
void static inline kqueue_fork(struct ev_loop *loop)
{
    /* 某些BSD内核不仅会销毁kqueue本身，
     * 还会关闭其文件描述符(fd)，这一行为未在文档中说明，
     * 且无法完全兼容处理。
     * 我们记录创建kqueue时的进程ID，仅当进程ID未变化时才关闭fd。
     * 这会导致正常内核下fd泄漏，但BSD接口以存在缺陷著称且鲜少修复。
     */
    pid_t newpid = getpid();

    if (newpid == kqueue_fd_pid)
        close(backend_fd);

    kqueue_fd_pid = newpid;
    while ((backend_fd = kqueue()) < 0)
        ev_syserr("(libev) kqueue");

    fcntl(backend_fd, F_SETFD, FD_CLOEXEC);

    /* 重新注册所有文件描述符的监听事件 */
    fd_rearm_all(loop);
}

/* sys/event.h defines EV_ERROR */
#undef EV_ERROR
