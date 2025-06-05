/*
 * libev事件库头文件
 * 提供高性能的事件循环实现，支持多种后端(select/poll/epoll/kqueue等)
 */
#ifndef EV_H_
#define EV_H_

#define __cplusplus 202302L

/* C++支持相关的宏定义 */
#ifdef __cplusplus
#define EV_CPP(x) x /* 在C++模式下展开为x，在C模式下为空 */
#if __cplusplus >= 201103L
#define EV_THROW noexcept /* C++11及以上使用noexcept */
#else
#define EV_THROW throw() /* C++11之前使用throw() */
#endif
#else
#define EV_CPP(x) /* C模式下为空 */
#define EV_THROW  /* C模式下为空 */
#endif

EV_CPP(extern "C" {)
/*****************************************************************************/

/*
 * 功能特性配置
 */
/* pre-4.0 兼容性设置 */
// #define EV_COMPAT3 0
#ifndef EV_COMPAT3
#define EV_COMPAT3 1 /* 默认启用3.x版本兼容性 */
#endif

/* 功能特性掩码配置 */
#ifndef EV_FEATURES
#if defined __OPTIMIZE_SIZE__
#define EV_FEATURES 0x7c /* 优化大小时禁用部分特性 */
#else
#define EV_FEATURES 0x7f /* 默认启用所有特性 */
#endif
#endif

/* 功能特性位定义 */
#define EV_FEATURE_CODE ((EV_FEATURES) & 1)      // 核心代码优化：是否启用了高效的内部实现
#define EV_FEATURE_DATA ((EV_FEATURES) & 2)      // 数据模型支持：是否支持复杂数据结构的监视器（如自定义内存管理）
#define EV_FEATURE_CONFIG ((EV_FEATURES) & 4)    // 配置灵活性：是否允许运行时动态配置（如修改事件循环参数）
#define EV_FEATURE_API ((EV_FEATURES) & 8)       // 扩展API：是否提供额外API（如非标准的事件操作函数）
#define EV_FEATURE_WATCHERS ((EV_FEATURES) & 16) // 监视器支持：是否支持高级监视器（如ev_embed、ev_async等）
#define EV_FEATURE_BACKENDS ((EV_FEATURES) & 32) // 后端支持：是否支持多事件后端（如epoll、kqueue、poll等）
#define EV_FEATURE_OS ((EV_FEATURES) & 64)       // 操作系统特性：是否依赖特定OS功能（如eventfd、inotify等）

/* 优先级配置 */
#ifndef EV_MINPRI
#define EV_MINPRI (EV_FEATURE_CONFIG ? -2 : 0) /* 最低优先级，仅在支持配置时可为负 */
#endif
#ifndef EV_MAXPRI
#define EV_MAXPRI (EV_FEATURE_CONFIG ? +2 : 0) /* 最高优先级，仅在支持配置时可为正 */
#endif

/* 多事件循环支持 */
#ifndef EV_MULTIPLICITY
#define EV_MULTIPLICITY EV_FEATURE_CONFIG /* 是否支持多个事件循环实例 */
#endif

/* 各类监视器的支持开关 */
#ifndef EV_PERIODIC_ENABLE
#define EV_PERIODIC_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_STAT_ENABLE
#define EV_STAT_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_PREPARE_ENABLE
#define EV_PREPARE_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_CHECK_ENABLE
#define EV_CHECK_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_IDLE_ENABLE
#define EV_IDLE_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_FORK_ENABLE
#define EV_FORK_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_CLEANUP_ENABLE
#define EV_CLEANUP_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_SIGNAL_ENABLE
#define EV_SIGNAL_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_CHILD_ENABLE
#ifdef _WIN32
#define EV_CHILD_ENABLE 0
#else
#define EV_CHILD_ENABLE EV_FEATURE_WATCHERS
#endif
#endif

#ifndef EV_ASYNC_ENABLE
#define EV_ASYNC_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_EMBED_ENABLE
#define EV_EMBED_ENABLE EV_FEATURE_WATCHERS
#endif

#ifndef EV_WALK_ENABLE
#define EV_WALK_ENABLE 0 /* not yet */
#endif

#if EV_CHILD_ENABLE && !EV_SIGNAL_ENABLE
#undef EV_SIGNAL_ENABLE
#define EV_SIGNAL_ENABLE 1
#endif

/*****************************************************************************/

/* 时间戳类型定义，使用双精度浮点数表示，单位为秒 */
typedef double ev_tstamp;

#include <string.h> /* for memmove */

#ifndef EV_ATOMIC_T
#include <signal.h>
#define EV_ATOMIC_T sig_atomic_t volatile // 强制必须每次从内存取值
#endif

#if EV_STAT_ENABLE
#ifdef _WIN32
#include <sys/types.h>
#include <time.h>
#endif
#include <sys/stat.h>
#endif

/* support multiple event loops? */
#if EV_MULTIPLICITY
    struct ev_loop;
#define EV_P struct ev_loop *loop           /* a loop as sole parameter in a declaration */
#define EV_P_ , EV_P,                       /* a loop as first of multiple parameters */
#define EV_A loop                           /* a loop as sole argument to a function call */
#define EV_A_ EV_A,                         /* a loop as first of multiple arguments */
#define EV_DEFAULT_UC ev_default_loop_uc_() /* the default loop, if initialised, as sole arg */
#define EV_DEFAULT_UC_ EV_DEFAULT_UC,       /* the default loop as first of multiple arguments */
#define EV_DEFAULT ev_default_loop(0)       /* the default loop as sole arg */
#define EV_DEFAULT_ EV_DEFAULT,             /* the default loop as first of multiple arguments */
#else
#define EV_P void
#define EV_P_ ,
#define EV_A
#define loop ,
#define EV_DEFAULT
#define EV_DEFAULT_
#define EV_DEFAULT_UC
#define EV_DEFAULT_UC_
#undef EV_EMBED_ENABLE
#endif

/* EV_INLINE is used for functions in header files */
#if __STDC_VERSION__ >= 199901L || __GNUC__ >= 3
#define EV_INLINE static inline
#else
#define EV_INLINE static
#endif

#ifdef EV_API_STATIC
#define EV_API_DECL static
#else
#define EV_API_DECL extern
#endif

/* EV_PROTOTYPES can be used to switch of prototype declarations */
#ifndef EV_PROTOTYPES
#define EV_PROTOTYPES 1
#endif

    /*****************************************************************************/

#define EV_VERSION_MAJOR 4
#define EV_VERSION_MINOR 22

    /* 事件掩码，返回事件，事件类型... */
    enum {
        EV_UNDEF = (int)0xFFFFFFFF, /* 保证无效的值 */
        EV_NONE = 0x00,             /* 无事件 */
        EV_READ = 0x01,             /* ev_io检测到读取操作不会阻塞 */
        EV_WRITE = 0x02,            /* ev_io检测到写入操作不会阻塞 */
        EV__IOFDSET = 0x80,         /* 仅内部使用 */
        EV_IO = EV_READ,            /* 用于类型检测的别名 */
        EV_TIMER = 0x00000100,      /* 定时器超时 */
#if EV_COMPAT3
        EV_TIMEOUT = EV_TIMER, /* 4.0之前API的兼容性定义 */
#endif
        EV_PERIODIC = 0x00000200,  /* 周期性定时器超时 */
        EV_SIGNAL = 0x00000400,    /* 接收到信号 */
        EV_CHILD = 0x00000800,     /* 子进程/PID状态改变 */
        EV_STAT = 0x00001000,      /* 统计信息数据变更 */
        EV_IDLE = 0x00002000,      /* 事件循环处于空闲状态 */
        EV_PREPARE = 0x00004000,   /* 事件循环即将轮询 */
        EV_CHECK = 0x00008000,     /* 事件循环完成轮询 */
        EV_EMBED = 0x00010000,     /* 嵌入式事件循环需要清理 */
        EV_FORK = 0x00020000,      /* 子进程中恢复事件循环 */
        EV_CLEANUP = 0x00040000,   /* 子进程中恢复事件循环 */
        EV_ASYNC = 0x00080000,     /* 循环内异步信号 */
        EV_CUSTOM = 0x01000000,    /* 供用户代码使用 */
        EV_ERROR = (int)0x80000000 /* 发生错误时发送 */
    };

/* can be used to add custom fields to all watchers, while losing binary compatibility */
#ifndef EV_COMMON
#define EV_COMMON void *data;
#endif

#ifndef EV_CB_DECLARE
#define EV_CB_DECLARE(type) void (*cb)(struct ev_loop * loop, struct type * w, int revents);
#endif
#ifndef EV_CB_INVOKE
#define EV_CB_INVOKE(watcher, revents) (watcher)->cb(loop, (watcher), (revents))
#endif

/* not official, do not use */
#define EV_CB(type, name) void name(struct ev_loop *loop, struct ev_##type *w, int revents)
// 是否定义优先级
#if EV_MINPRI == EV_MAXPRI
#define EV_DECL_PRIORITY
#elif !defined(EV_DECL_PRIORITY)
#define EV_DECL_PRIORITY int priority
#endif

/* shared by all watchers */
#define EV_WATCHER(type)              \
    int active;         /* private */ \
    int pending;        /* private */ \
    EV_DECL_PRIORITY;   /* private */ \
    EV_COMMON           /* rw */      \
    EV_CB_DECLARE(type) /* private */

#define EV_WATCHER_LIST(type) \
    EV_WATCHER(type)          \
    struct ev_watcher_list *next; /* private */

#define EV_WATCHER_TIME(type) \
    EV_WATCHER(type)          \
    ev_tstamp at; /* private */

    /* 基础类，除非你要继承它，否则无需关注 */
    /* 
 * 基础监视器结构体
 * 所有具体类型的监视器都是从这个基础结构扩展而来
 */
typedef struct ev_watcher {
        int active;   /* 监视器是否激活 */
        int pending;  /* 是否有待处理的事件 */
        int priority; /* 优先级 */
        void *data;   /* 用户数据 */
        void (*cb)(struct ev_loop *loop, struct ev_watcher *w, int revents); /* 回调函数 */
    } ev_watcher;

    /* 
     * 链表型监视器基础结构体
     * 用于需要链表功能的监视器类型
     */
    typedef struct ev_watcher_list {
        int active;   /* 监视器是否激活 */
        int pending;  /* 是否有待处理的事件 */
        int priority; /* 优先级 */
        void *data;   /* 用户数据 */
        void (*cb)(struct ev_loop *loop, struct ev_watcher_list *w, int revents); /* 回调函数 */
        struct ev_watcher_list *next; /* 链表下一个节点 */
    } ev_watcher_list;

    /* 
     * 定时器监视器基础结构体
     * 用于需要时间相关功能的监视器类型
     */
    typedef struct ev_watcher_time {
        int active;    /* 监视器是否激活; 0-未激活, 1-已激活 */
        int pending;   /* 在pending数组中的位置 */
        int priority;  /* 优先级 */
        void *data;    /* 用户数据 */
        void (*cb)(struct ev_loop *loop, struct ev_watcher_time *w, int revents); /* 回调函数 */
        ev_tstamp at;  /* 下次触发的时间戳 */
    } ev_watcher_time;

    /* 当文件描述符(fd)可读(EV_READ)或可写(EV_WRITE)时被调用 */
    /* 事件类型：可读事件、可写事件 */
    typedef struct ev_io {
        EV_WATCHER_LIST(ev_io)
        int fd;     /* ro */
        int events; /* ro */
    } ev_io;

    /* 在特定时间后调用，可重复（基于单调时钟）, 重新触发事件 EV_TIMEOUT */
    typedef struct ev_timer {
        EV_WATCHER_TIME(ev_timer)
        ev_tstamp repeat; /* 可读写 */
    } ev_timer;

    /* 在特定时间触发，可能基于UTC时间定期重复执行 */
    /* 事件类型：EV_PERIODIC（周期性事件） */
    typedef struct ev_periodic {
        EV_WATCHER_TIME(ev_periodic) /* 基础时间监视器宏 */

        ev_tstamp offset;                                                          /* 可读写 时间偏移量 */
        ev_tstamp interval;                                                        /* 可读写 触发间隔 */
        ev_tstamp (*reschedule_cb)(struct ev_periodic *w, ev_tstamp now) noexcept; /* 可读写 重新调度回调函数 */
    } ev_periodic;

    /* 当接收到指定信号时被调用 */
    /* 重新事件 EV_SIGNAL */
    typedef struct ev_signal {
        EV_WATCHER_LIST(ev_signal)
        int signum; /* 只读 */
    } ev_signal;

    /* 当接收到SIGCHLD信号且waitpid返回指定pid时触发 */
    /* 事件类型为EV_CHILD, 不支持优先级设置 */
    typedef struct ev_child {
        EV_WATCHER_LIST(ev_child) /* 监控器链表宏 */

        int flags;   /* 私有标志位 */
        int pid;     /* 只读，监控的子进程ID */
        int rpid;    /* 可读写，实际接收到的进程ID */
        int rstatus; /* 可读写，保存退出状态，需使用sys/wait.h中的宏进行解析 */
    } ev_child;

#if EV_STAT_ENABLE
/* st_nlink = 0 means missing file or other error */
#ifdef _WIN32
    typedef struct _stati64 ev_statdata;
#else
    typedef struct stat ev_statdata;
#endif

    /* 每次文件元数据发生变化时通知 EV_STAT */
    typedef struct ev_stat {
        EV_WATCHER_LIST(ev_stat) /* 监控器列表宏 */

        ev_timer timer;     /* 内部使用的定时器 */
        ev_tstamp interval; /* 只读属性：监控间隔 */
        const char *path;   /* 只读属性：监控路径 */
        ev_statdata prev;   /* 只读属性：前次状态数据 */
        ev_statdata attr;   /* 只读属性：当前属性数据 */

        int wd; /* inotify机制使用的监控描述符/kqueue机制使用的文件描述符 */
    } ev_stat;
#endif

#if EV_IDLE_ENABLE
    /* 当没有其他任务需要处理时调用，防止进程阻塞，重新触发EV_IDLE事件 */
    typedef struct ev_idle {
        EV_WATCHER(ev_idle)
    } ev_idle;
#endif

    /* 每次主循环运行前调用，恰好在阻塞调用之前 */
    /* 此时仍可任意修改事件设置，事件类型：EV_PREPARE */
    typedef struct ev_prepare {
        EV_WATCHER(ev_prepare)
    } ev_prepare;

    /* 每次主循环运行后立即调用，紧跟在阻塞调用之后，事件类型：EV_CHECK */
    typedef struct ev_check {
        EV_WATCHER(ev_check) /* 事件监视器宏 */
    } ev_check;

#if EV_FORK_ENABLE
    /* 当检测到fork操作时，该回调函数会在子进程中进行检查前被调用 */
    /* 事件类型：EV_FORK */
    typedef struct ev_fork {
        EV_WATCHER(ev_fork)
    } ev_fork;
#endif

#if EV_CLEANUP_ENABLE
    /* 该回调函数恰好在事件循环被销毁前调用 */
    /* 事件类型：EV_CLEANUP */
    typedef struct ev_cleanup {
        EV_WATCHER(ev_cleanup)
    } ev_cleanup;
#endif

#if EV_EMBED_ENABLE
    /* 用于将一个事件循环嵌入到另一个事件循环中 */
    /* 当事件循环处理完事件后会调用该回调函数，该回调可设为0 */
    typedef struct ev_embed {
        EV_WATCHER(ev_embed)

        struct ev_loop *other; /* 只读 */
        ev_io io;              /* 私有 */
        ev_prepare prepare;    /* 私有 */
        ev_check check;        /* 未使用 */
        ev_timer timer;        /* 未使用 */
        ev_periodic periodic;  /* 未使用 */
        ev_idle idle;          /* 未使用 */
        ev_fork fork;          /* 私有 */
#if EV_CLEANUP_ENABLE
        ev_cleanup cleanup; /* 未使用 */
#endif
    } ev_embed;
#endif

#if EV_ASYNC_ENABLE
    /* invoked when somebody calls ev_async_send on the watcher */
    /* revent EV_ASYNC */
    typedef struct ev_async {
        EV_WATCHER(ev_async)

        EV_ATOMIC_T sent; /* private */
    } ev_async;

#define ev_async_pending(w) (+(w)->sent)
#endif

    /* the presence of this union forces similar struct layout */
    union ev_any_watcher {
        struct ev_watcher w;
        struct ev_watcher_list wl;

        struct ev_io io;
        struct ev_timer timer;
        struct ev_periodic periodic;
        struct ev_signal signal;
        struct ev_child child;
#if EV_STAT_ENABLE
        struct ev_stat stat;
#endif
#if EV_IDLE_ENABLE
        struct ev_idle idle;
#endif
        struct ev_prepare prepare;
        struct ev_check check;
#if EV_FORK_ENABLE
        struct ev_fork fork;
#endif
#if EV_CLEANUP_ENABLE
        struct ev_cleanup cleanup;
#endif
#if EV_EMBED_ENABLE
        struct ev_embed embed;
#endif
#if EV_ASYNC_ENABLE
        struct ev_async async;
#endif
    };

    /* 用于ev_default_loop和ev_loop_new的标志位 */
    enum {
        /* 默认值 */
        EVFLAG_AUTO = 0x00000000U, /* 非严格意义上的掩码 */
        /* 标志位 */
        EVFLAG_NOENV = 0x01000000U,     /* 不读取环境变量 */
        EVFLAG_FORKCHECK = 0x02000000U, /* 每次迭代检查fork事件 */
        /* 调试/功能禁用 */
        EVFLAG_NOINOTIFY = 0x00100000U, /* 禁止使用inotify */
#if EV_COMPAT3
        EVFLAG_NOSIGFD = 0, /* 兼容3.9之前版本 */
#endif
        EVFLAG_SIGNALFD = 0x00200000U, /* 尝试使用signalfd */
        EVFLAG_NOSIGMASK = 0x00400000U /* 避免修改信号掩码 */
    };

    /* 需要按位或组合的方法标志位 */
    enum {
        EVBACKEND_SELECT = 0x00000001U, /* about anywhere */
        EVBACKEND_POLL = 0x00000002U,   /* !win */
        EVBACKEND_EPOLL = 0x00000004U,  /* linux */
        EVBACKEND_KQUEUE = 0x00000008U, /* bsd */
        EVBACKEND_DEVPOLL = 0x00000010U,
        /* solaris 8 */               /* NYI */
        EVBACKEND_PORT = 0x00000020U, /* solaris 10 */
        EVBACKEND_ALL = 0x0000003FU,  /* all known backends */
        EVBACKEND_MASK = 0x0000FFFFU  /* all future backends */
    };

#if EV_PROTOTYPES
    /* 
     * 获取libev的主版本号
     * 返回值：主版本号
     */
    EV_API_DECL int ev_version_major(void) noexcept;

    /* 
     * 获取libev的次版本号
     * 返回值：次版本号
     */
    EV_API_DECL int ev_version_minor(void) noexcept;

    /*
     * 获取当前系统支持的所有后端类型
     * 返回值：支持的后端类型的位掩码组合，每个位代表一种后端类型（如select、poll、epoll等）
     */
    EV_API_DECL unsigned int ev_supported_backends(void) noexcept;

    /*
     * 获取当前系统推荐使用的后端类型
     * 根据系统特性和性能考虑，返回最适合的后端类型
     * 返回值：推荐的后端类型的位掩码组合
     */
    EV_API_DECL unsigned int ev_recommended_backends(void) noexcept;

    /*
     * 获取支持嵌入的后端类型
     * 某些后端可能不支持在其他事件循环中嵌入使用
     * 返回值：支持嵌入的后端类型的位掩码组合
     */
    EV_API_DECL unsigned int ev_embeddable_backends(void) noexcept;

    /*
     * 获取当前的绝对时间戳
     * 此函数返回实际的系统时间，精度为微秒级
     * 注意: ev_now()函数通常更快，且返回事件循环使用的时间戳
     * 返回值：当前时间戳（双精度浮点数表示的秒数）
     */
    EV_API_DECL ev_tstamp ev_time(void) noexcept;

    /*
     * 使当前线程休眠指定的时间
     * 参数：
     *   delay: 休眠时间（秒），支持亚秒级精度
     */
    EV_API_DECL void ev_sleep(ev_tstamp delay) noexcept;

    /* 设置要使用的内存分配函数，功能类似于realloc。
     * 该函数用于分配和释放内存。
     * 当需要分配内存时若返回零值，库可能会中止运行
     * 或采取某些可能造成破坏的操作。
     * 默认使用系统自带的realloc函数。
     */
    EV_API_DECL void ev_set_allocator(void *(*cb)(void *ptr, long size) noexcept) noexcept;

    /* 设置当发生可重试系统调用错误时
     * 需要调用的回调函数
     * (例如失败的select、poll或epoll_wait操作)
     */
    EV_API_DECL void ev_set_syserr_cb(void (*cb)(const char *msg) noexcept) noexcept;

#if EV_MULTIPLICITY

    /*
     * 获取或创建默认事件循环
     * 默认循环是唯一可以处理信号和子进程监视器的循环
     * 可以多次调用此函数，总是返回同一个循环实例
     * 参数：
     *   flags: 循环标志位，可以是EVFLAG_*的组合
     * 返回值：
     *   成功返回循环指针，失败返回NULL
     * 注意：
     *   - 这是获取全局默认事件循环的推荐方式
     *   - 默认循环在首次调用时创建，之后的调用返回相同实例
     *   - 主要用于简单的单线程应用
     */
    EV_API_DECL struct ev_loop *ev_default_loop(unsigned int flags EV_CPP(= 0)) noexcept;

#ifdef EV_API_STATIC
    EV_API_DECL struct ev_loop *ev_default_loop_ptr;
#endif

    EV_INLINE struct ev_loop *ev_default_loop_uc_(void) noexcept {
        extern struct ev_loop *ev_default_loop_ptr;

        return ev_default_loop_ptr;
    }

    EV_INLINE int
    ev_is_default_loop(struct ev_loop * loop) noexcept {
        return EV_A == EV_DEFAULT_UC;
    }

    /*
     * 创建新的事件循环实例
     * 创建一个独立的事件循环，不处理信号和子进程
     * 参数：
     *   flags: 循环标志位，可以是EVFLAG_*的组合
     * 返回值：
     *   成功返回新创建的循环指针，失败返回NULL
     * 注意：
     *   - 新创建的循环与默认循环相互独立
     *   - 不支持信号处理，主要用于特定任务的事件处理
     *   - 适合在多线程环境中使用
     */
    EV_API_DECL struct ev_loop *ev_loop_new(unsigned int flags EV_CPP(= 0)) noexcept;

    /*
     * 获取事件循环的当前时间
     * 返回事件循环内部使用的时间戳，在每次轮询后更新
     * 参数：
     *   loop: 事件循环指针
     * 返回值：
     *   事件循环的当前时间戳（双精度浮点数表示的秒数）
     * 注意：
     *   - 此时间用于定时器和周期性事件的调度
     *   - 不一定与系统时间完全同步
     */
    EV_API_DECL ev_tstamp ev_now(struct ev_loop * loop) noexcept;

#else

    EV_API_DECL int ev_default_loop(unsigned int flags EV_CPP(= 0)) noexcept; /* returns true when successful */

    EV_API_DECL ev_tstamp ev_rt_now;

    EV_INLINE ev_tstamp ev_now(void) noexcept { return ev_rt_now;}

    /* looks weird, but ev_is_default_loop (loop) still works if this exists */
    EV_INLINE int ev_is_default_loop(void) noexcept { return 1;}

#endif /* multiplicity */

    /* destroy event loops, also works for the default loop */
    EV_API_DECL void ev_loop_destroy(struct ev_loop * loop);

    /* 该函数需要在fork之后调用，用于复制循环 */
    /* 当你想在子进程中复用这个循环时使用 */
    /* 可以在父进程或子进程中调用它 */
    /* 实际上你可以在任何时间、任何位置调用它 :) */
    EV_API_DECL void ev_loop_fork(struct ev_loop * loop) noexcept;

    EV_API_DECL unsigned int ev_backend(struct ev_loop * loop) noexcept; /* backend in use by loop */

    EV_API_DECL void ev_now_update(struct ev_loop * loop) noexcept; /* update event loop time */

#if EV_WALK_ENABLE
    /* 遍历指定类型的所有（近乎全部）监视器，对每个监视器调用回调函数 */
    /* 回调函数可以停止监视器，但不对事件循环进行其他操作 */
    EV_API_DECL void ev_walk(struct ev_loop * loop, int types, void (*cb)(struct ev_loop *loop, int type, void *w)) noexcept;
#endif

#endif /* prototypes */

    /* ev_run flags values */
    enum {
        EVRUN_NOWAIT = 1, /* do not block/wait */
        EVRUN_ONCE = 2    /* block *once* only */
    };

    /* ev_break how values */
    enum {
        EVBREAK_CANCEL = 0, /* undo unloop */
        EVBREAK_ONE = 1,    /* unloop once */
        EVBREAK_ALL = 2     /* unloop all loops */
    };

#if EV_PROTOTYPES
    /*
     * 运行事件循环，处理事件
     * 参数：
     *   loop: 事件循环指针
     *   flags: 运行标志，可以是以下值：
     *     0: 正常模式，处理完所有活动事件后阻塞等待新事件
     *     EVRUN_NOWAIT: 非阻塞模式，处理当前所有活动事件后立即返回
     *     EVRUN_ONCE: 处理一轮事件后返回，可能会阻塞等待事件发生
     * 返回值：
     *   非零表示成功，零表示循环被中断（通过ev_break）
     */
    EV_API_DECL int ev_run(struct ev_loop * loop, int flags EV_CPP(= 0));

    /*
     * 中断事件循环
     * 参数：
     *   loop: 事件循环指针
     *   how: 中断方式，可以是以下值：
     *     EVBREAK_ONE: 仅中断当前事件循环
     *     EVBREAK_ALL: 中断所有嵌套的事件循环
     *     EVBREAK_CANCEL: 取消之前的中断请求
     */
    EV_API_DECL void ev_break(struct ev_loop * loop, int how EV_CPP(= EVBREAK_ONE)) noexcept;

    /*
     * ref/unref 可用于在主循环上增加或减少引用计数。每个监视器都会持有一个引用。
     * 若您有一个长期运行的监视器从未注销，但又不希望它阻止ev_loop运行，
     * 可在启动后调用unref()，在停止前调用ref()。
     */
    EV_API_DECL void ev_ref(struct ev_loop * loop) noexcept;
    EV_API_DECL void ev_unref(struct ev_loop * loop) noexcept;

    /*
     * 便捷函数：等待单个事件触发，无需注册事件监视器
     * 若timeout参数小于0，则无限期等待
     */
    EV_API_DECL void ev_once(struct ev_loop * loop, int fd, int events, ev_tstamp timeout, void (*cb)(int revents, void *arg), void *arg) noexcept;
#if EV_FEATURE_API
    EV_API_DECL unsigned int ev_iteration(struct ev_loop * loop) noexcept; /* number of loop iterations */
    EV_API_DECL unsigned int ev_depth(struct ev_loop * loop) noexcept;     /* #ev_loop enters - #ev_loop leaves */
    EV_API_DECL void ev_verify(struct ev_loop * loop) noexcept;            /* abort if loop data corrupted */

    EV_API_DECL void ev_set_io_collect_interval(struct ev_loop * loop, ev_tstamp interval) noexcept;      /* sleep at least this time, default 0 */
    EV_API_DECL void ev_set_timeout_collect_interval(struct ev_loop * loop, ev_tstamp interval) noexcept; /* sleep at least this time, default 0 */

    /* advanced stuff for threading etc. support, see docs */
    EV_API_DECL void ev_set_userdata(struct ev_loop * loop, void *data) noexcept;
    EV_API_DECL void *ev_userdata(struct ev_loop * loop) noexcept;
    typedef void (*ev_loop_callback)(struct ev_loop *loop);
    EV_API_DECL void ev_set_invoke_pending_cb(struct ev_loop * loop, ev_loop_callback invoke_pending_cb) noexcept;
    /* C++ doesn't allow the use of the ev_loop_callback typedef here, so we need to spell it out */
    EV_API_DECL void ev_set_loop_release_cb(struct ev_loop * loop, void (*release)(struct ev_loop *loop) noexcept, void (*acquire)(struct ev_loop *loop) noexcept) noexcept;

    EV_API_DECL unsigned int ev_pending_count(struct ev_loop * loop) noexcept; /* number of pending events, if any */
    EV_API_DECL void ev_invoke_pending(struct ev_loop * loop);                 /* invoke all pending watchers */

    /*
     * stop/start the timer handling.
     */
    EV_API_DECL void ev_suspend(struct ev_loop * loop) noexcept;
    EV_API_DECL void ev_resume(struct ev_loop * loop) noexcept;
#endif

#endif

/* these may evaluate ev multiple times, and the other arguments at most once */
/* either use ev_init + ev_TYPE_set, or the ev_TYPE_init macro, below, to first initialise a watcher */
#define ev_init(ev, cb_)                               \
    do                                                 \
    {                                                  \
        ((ev_watcher *)(void *)(ev))->active =         \
            ((ev_watcher *)(void *)(ev))->pending = 0; \
        ev_set_priority((ev), 0);                      \
        ev_set_cb((ev), cb_);                          \
    } while (0)

#define ev_io_set(ev, fd_, events_)             \
    do                                          \
    {                                           \
        (ev)->fd = (fd_);                       \
        (ev)->events = (events_) | EV__IOFDSET; \
    } while (0)

#define ev_timer_set(ev, after_, repeat_)         \
    do                                            \
    {                                             \
        ((ev_watcher_time *)(ev))->at = (after_); \
        (ev)->repeat = (repeat_);                 \
    } while (0)

#define ev_periodic_set(ev, ofs_, ival_, rcb_) \
    do                                         \
    {                                          \
        (ev)->offset = (ofs_);                 \
        (ev)->interval = (ival_);              \
        (ev)->reschedule_cb = (rcb_);          \
    } while (0)

#define ev_signal_set(ev, signum_) \
    do                             \
    {                              \
        (ev)->signum = (signum_);  \
    } while (0)

#define ev_child_set(ev, pid_, trace_) \
    do                                 \
    {                                  \
        (ev)->pid = (pid_);            \
        (ev)->flags = !!(trace_);      \
    } while (0)

#define ev_stat_set(ev, path_, interval_) \
    do                                    \
    {                                     \
        (ev)->path = (path_);             \
        (ev)->interval = (interval_);     \
        (ev)->wd = -2;                    \
    } while (0)

#define ev_idle_set(ev)    /* nop, yes, this is a serious in-joke */
#define ev_prepare_set(ev) /* nop, yes, this is a serious in-joke */
#define ev_check_set(ev)   /* nop, yes, this is a serious in-joke */
#define ev_embed_set(ev, other_) \
    do                           \
    {                            \
        (ev)->other = (other_);  \
    } while (0)

#define ev_fork_set(ev)    /* nop, yes, this is a serious in-joke */
#define ev_cleanup_set(ev) /* nop, yes, this is a serious in-joke */
#define ev_async_set(ev)   /* nop, yes, this is a serious in-joke */

#define ev_io_init(ev, cb, fd, events)   \
    do                                   \
    {                                    \
        ev_init((ev), (cb));             \
        ev_io_set((ev), (fd), (events)); \
    } while (0)

#define ev_timer_init(ev, cb, after, repeat)   \
    do                                         \
    {                                          \
        ev_init((ev), (cb));                   \
        ev_timer_set((ev), (after), (repeat)); \
    } while (0)
#define ev_periodic_init(ev, cb, ofs, ival, rcb)     \
    do                                               \
    {                                                \
        ev_init((ev), (cb));                         \
        ev_periodic_set((ev), (ofs), (ival), (rcb)); \
    } while (0)
#define ev_signal_init(ev, cb, signum) \
    do                                 \
    {                                  \
        ev_init((ev), (cb));           \
        ev_signal_set((ev), (signum)); \
    } while (0)
#define ev_child_init(ev, cb, pid, trace)   \
    do                                      \
    {                                       \
        ev_init((ev), (cb));                \
        ev_child_set((ev), (pid), (trace)); \
    } while (0)
#define ev_stat_init(ev, cb, path, interval)   \
    do                                         \
    {                                          \
        ev_init((ev), (cb));                   \
        ev_stat_set((ev), (path), (interval)); \
    } while (0)

#define ev_idle_init(ev, cb) \
    do                       \
    {                        \
        ev_init((ev), (cb)); \
        ev_idle_set((ev));   \
    } while (0)

#define ev_prepare_init(ev, cb) \
    do                          \
    {                           \
        ev_init((ev), (cb));    \
        ev_prepare_set((ev));   \
    } while (0)

#define ev_check_init(ev, cb) \
    do                        \
    {                         \
        ev_init((ev), (cb));  \
        ev_check_set((ev));   \
    } while (0)

#define ev_embed_init(ev, cb, other) \
    do                               \
    {                                \
        ev_init((ev), (cb));         \
        ev_embed_set((ev), (other)); \
    } while (0)

#define ev_fork_init(ev, cb) \
    do                       \
    {                        \
        ev_init((ev), (cb)); \
        ev_fork_set((ev));   \
    } while (0)

#define ev_cleanup_init(ev, cb) \
    do                          \
    {                           \
        ev_init((ev), (cb));    \
        ev_cleanup_set((ev));   \
    } while (0)

#define ev_async_init(ev, cb) \
    do                        \
    {                         \
        ev_init((ev), (cb));  \
        ev_async_set((ev));   \
    } while (0)

#define ev_is_pending(ev) (0 + ((ev_watcher *)(void *)(ev))->pending) /* ro, true when watcher is waiting for callback invocation */
#define ev_is_active(ev) (0 + ((ev_watcher *)(void *)(ev))->active)   /* ro, true when the watcher has been started */

#define ev_cb_(ev) (ev)->cb /* rw */
#define ev_cb(ev) (memmove(&ev_cb_(ev), &((ev_watcher *)(ev))->cb, sizeof(ev_cb_(ev))), (ev)->cb)

#if EV_MINPRI == EV_MAXPRI
#define ev_priority(ev) ((ev), EV_MINPRI)
#define ev_set_priority(ev, pri) ((ev), (pri))
#else
#define ev_priority(ev) (+(((ev_watcher *)(void *)(ev))->priority))
#define ev_set_priority(ev, pri) ((ev_watcher *)(void *)(ev))->priority = (pri)
#endif

#define ev_periodic_at(ev) (+((ev_watcher_time *)(ev))->at)

#ifndef ev_set_cb // TODO
#define ev_set_cb(ev, cb_) (ev_cb_(ev) = (cb_), memmove(&((ev_watcher *)(ev))->cb, &ev_cb_(ev), sizeof(ev_cb_(ev))))
#endif

/* 停止（启用、添加）监视器时，若其已在运行则无操作 */
/* 停止（禁用、删除）监视器时，除非其已在运行否则无操作 */
#if EV_PROTOTYPES
    /* 向监视器注入事件，模拟真实事件发生 */
    /* 接受任何ev_watcher类型 */
    /*
     * 手动触发一个监视器的事件
     * 参数：
     *   loop: 事件循环指针
     *   w: 监视器指针
     *   revents: 要触发的事件类型
     * 说明：
     *   可用于测试或特殊情况下模拟事件发生
     */
    EV_API_DECL void ev_feed_event(struct ev_loop * loop, void *w, int revents) noexcept;

    /*
     * 手动触发一个文件描述符的事件
     * 参数：
     *   loop: 事件循环指针
     *   fd: 文件描述符
     *   revents: 要触发的事件类型
     * 说明：
     *   直接触发与文件描述符相关的事件
     */
    EV_API_DECL void ev_feed_fd_event(struct ev_loop * loop, int fd, int revents) noexcept;

#if EV_SIGNAL_ENABLE
    /*
     * 手动触发一个信号
     * 参数：
     *   signum: 信号编号
     * 说明：
     *   向进程发送指定的信号
     */
    EV_API_DECL void ev_feed_signal(int signum) noexcept;

    /*
     * 手动触发事件循环中的信号处理
     * 参数：
     *   loop: 事件循环指针
     *   signum: 信号编号
     * 说明：
     *   在事件循环中模拟接收到信号
     */
    EV_API_DECL void ev_feed_signal_event(struct ev_loop * loop, int signum) noexcept;
#endif

    /*
     * 立即调用监视器的回调函数
     * 参数：
     *   loop: 事件循环指针
     *   w: 监视器指针
     *   revents: 要触发的事件类型
     * 说明：
     *   直接调用监视器的回调函数，而不经过事件循环的调度
     */
    EV_API_DECL void ev_invoke(struct ev_loop * loop, void *w, int revents);

    /*
     * 清除监视器的待处理状态
     * 参数：
     *   loop: 事件循环指针
     *   w: 监视器指针
     * 返回值：
     *   返回之前的待处理状态
     */
    EV_API_DECL int ev_clear_pending(struct ev_loop * loop, void *w) noexcept;

    /*
     * I/O监视器操作函数
     */
    /* 启动I/O监视器 */
    EV_API_DECL void ev_io_start(struct ev_loop * loop, ev_io * w) noexcept;
    /* 停止I/O监视器 */
    EV_API_DECL void ev_io_stop(struct ev_loop * loop, ev_io * w) noexcept;

    /*
     * 定时器监视器操作函数
     */
    /* 启动定时器 */
    EV_API_DECL void ev_timer_start(struct ev_loop * loop, ev_timer * w) noexcept;
    /* 停止定时器 */
    EV_API_DECL void ev_timer_stop(struct ev_loop * loop, ev_timer * w) noexcept;
    /* 
     * 重新启动定时器
     * 若定时器已激活且无重复间隔，则停止定时器
     * 若定时器已激活且有重复间隔，则重新启动定时器
     * 若定时器未激活且有重复间隔，则启动定时器
     */
    EV_API_DECL void ev_timer_again(struct ev_loop * loop, ev_timer * w) noexcept;

    /*
     * 获取定时器剩余时间
     * 参数：
     *   loop: 事件循环指针
     *   w: 定时器监视器指针
     * 返回值：
     *   定时器触发前的剩余时间（单位：秒）
     */
    EV_API_DECL ev_tstamp ev_timer_remaining(struct ev_loop * loop, ev_timer * w) noexcept;

#if EV_PERIODIC_ENABLE
    /*
     * 周期性定时器监视器操作函数
     */
    /* 启动周期性定时器 */
    EV_API_DECL void ev_periodic_start(struct ev_loop * loop, ev_periodic * w) noexcept;
    /* 停止周期性定时器 */
    EV_API_DECL void ev_periodic_stop(struct ev_loop * loop, ev_periodic * w) noexcept;
    /* 重新计算下次触发时间并重启定时器 */
    EV_API_DECL void ev_periodic_again(struct ev_loop * loop, ev_periodic * w) noexcept;
#endif

/* only supported in the default loop */
#if EV_SIGNAL_ENABLE
    /*
     * 信号监视器操作函数
     * 注意：仅在默认事件循环中支持
     */
    /* 启动信号监视器 */
    EV_API_DECL void ev_signal_start(struct ev_loop * loop, ev_signal * w) noexcept;
    /* 停止信号监视器 */
    EV_API_DECL void ev_signal_stop(struct ev_loop * loop, ev_signal * w) noexcept;
#endif

/* only supported in the default loop */
#if EV_CHILD_ENABLE
    /*
     * 子进程监视器操作函数
     * 注意：仅在默认事件循环中支持
     */
    /* 启动子进程监视器 */
    EV_API_DECL void ev_child_start(struct ev_loop * loop, ev_child * w) noexcept;
    /* 停止子进程监视器 */
    EV_API_DECL void ev_child_stop(struct ev_loop * loop, ev_child * w) noexcept;
#endif

#if EV_STAT_ENABLE
    /*
     * 文件状态监视器操作函数
     */
    /* 启动文件状态监视器 */
    EV_API_DECL void ev_stat_start(struct ev_loop * loop, ev_stat * w) noexcept;
    /* 停止文件状态监视器 */
    EV_API_DECL void ev_stat_stop(struct ev_loop * loop, ev_stat * w) noexcept;
    /* 立即更新文件状态信息 */
    EV_API_DECL void ev_stat_stat(struct ev_loop * loop, ev_stat * w) noexcept;
#endif

#if EV_IDLE_ENABLE
    /*
     * 空闲监视器操作函数
     * 当事件循环空闲时触发回调
     */
    /* 启动空闲监视器 */
    EV_API_DECL void ev_idle_start(struct ev_loop * loop, ev_idle * w) noexcept;
    /* 停止空闲监视器 */
    EV_API_DECL void ev_idle_stop(struct ev_loop * loop, ev_idle * w) noexcept;
#endif

#if EV_PREPARE_ENABLE
    /*
     * 准备阶段监视器操作函数
     * 在事件循环阻塞之前触发
     */
    /* 启动准备阶段监视器 */
    EV_API_DECL void ev_prepare_start(struct ev_loop * loop, ev_prepare * w) noexcept;
    /* 停止准备阶段监视器 */
    EV_API_DECL void ev_prepare_stop(struct ev_loop * loop, ev_prepare * w) noexcept;
#endif

#if EV_CHECK_ENABLE
    /*
     * 检查阶段监视器操作函数
     * 在事件循环阻塞之后触发
     */
    /* 启动检查阶段监视器 */
    EV_API_DECL void ev_check_start(struct ev_loop * loop, ev_check * w) noexcept;
    /* 停止检查阶段监视器 */
    EV_API_DECL void ev_check_stop(struct ev_loop * loop, ev_check * w) noexcept;
#endif

#if EV_FORK_ENABLE
    /*
     * fork监视器操作函数
     * 在fork后的子进程中触发
     */
    /* 启动fork监视器 */
    EV_API_DECL void ev_fork_start(struct ev_loop * loop, ev_fork * w) noexcept;
    /* 停止fork监视器 */
    EV_API_DECL void ev_fork_stop(struct ev_loop * loop, ev_fork * w) noexcept;
#endif

#if EV_CLEANUP_ENABLE
    /*
     * 清理监视器操作函数
     * 在事件循环销毁前触发
     */
    /* 启动清理监视器 */
    EV_API_DECL void ev_cleanup_start(struct ev_loop * loop, ev_cleanup * w) noexcept;
    /* 停止清理监视器 */
    EV_API_DECL void ev_cleanup_stop(struct ev_loop * loop, ev_cleanup * w) noexcept;
#endif

#if EV_EMBED_ENABLE
    /*
     * 嵌入式事件循环操作函数
     * 用于在一个事件循环中嵌入另一个循环
     * 注意：仅支持可嵌入的事件循环
     */
    /* 启动嵌入式事件循环 */
    EV_API_DECL void ev_embed_start(struct ev_loop * loop, ev_embed * w) noexcept;
    /* 停止嵌入式事件循环 */
    EV_API_DECL void ev_embed_stop(struct ev_loop * loop, ev_embed * w) noexcept;
    /* 处理嵌入式事件循环中的待处理事件 */
    EV_API_DECL void ev_embed_sweep(struct ev_loop * loop, ev_embed * w) noexcept;
#endif

#if EV_ASYNC_ENABLE
    /*
     * 异步事件监视器操作函数
     * 用于跨线程触发事件
     */
    /* 启动异步事件监视器 */
    EV_API_DECL void ev_async_start(struct ev_loop * loop, ev_async * w) noexcept;
    /* 停止异步事件监视器 */
    EV_API_DECL void ev_async_stop(struct ev_loop * loop, ev_async * w) noexcept;
    /* 发送异步事件通知 */
    EV_API_DECL void ev_async_send(struct ev_loop * loop, ev_async * w) noexcept;
#endif

/*---------------------------------------------------------------------*/
// 以下定义了与evlib 3兼容的包装层.
#if EV_COMPAT3
/* 兼容 libev 3.x 版本的宏定义 */
#define EVLOOP_NONBLOCK EVRUN_NOWAIT   // 非阻塞运行事件循环
#define EVLOOP_ONESHOT EVRUN_ONCE      // 只运行一次事件循环
#define EVUNLOOP_CANCEL EVBREAK_CANCEL // 取消所有 break 请求
#define EVUNLOOP_ONE EVBREAK_ONE       // 退出当前事件循环迭代
#define EVUNLOOP_ALL EVBREAK_ALL       // 完全退出事件循环

#if EV_PROTOTYPES
    /**
     * 运行事件循环 (兼容3.x API)
     * @param loop 事件循环实例
     * @param flags 运行标志 (EVLOOP_NONBLOCK/EVLOOP_ONESHOT)
     */
    EV_INLINE void ev_loop(struct ev_loop * loop, int flags) { ev_run(loop, flags); }
    
    /**
     * 中断事件循环 (兼容3.x API)
     * @param loop 事件循环实例
     * @param how 中断方式 (EVUNLOOP_ONE/EVUNLOOP_ALL/EVUNLOOP_CANCEL)
     */
    EV_INLINE void ev_unloop(struct ev_loop * loop, int how) { ev_break(loop, how); }
    
    /**
     * 销毁默认事件循环 (兼容3.x API)
     */
    EV_INLINE void ev_default_destroy(void) { ev_loop_destroy(EV_DEFAULT); }
    
    /**
     * 在fork后重新初始化默认事件循环 (兼容3.x API)
     */
    EV_INLINE void ev_default_fork(void) { ev_loop_fork(EV_DEFAULT); }

#if EV_FEATURE_API
    /**
     * 获取事件循环迭代次数 (兼容3.x API)
     * @param loop 事件循环实例
     * @return 当前迭代次数
     */
    EV_INLINE unsigned int ev_loop_count(struct ev_loop * loop) { return ev_iteration(loop); }
    
    /**
     * 获取事件循环嵌套深度 (兼容3.x API)
     * @param loop 事件循环实例
     * @return 当前嵌套深度
     */
    EV_INLINE unsigned int ev_loop_depth(struct ev_loop * loop) { return ev_depth(loop); }
    
    /**
     * 验证事件循环内部状态 (兼容3.x API)
     * @param loop 事件循环实例
     */
    EV_INLINE void ev_loop_verify(struct ev_loop * loop) { ev_verify(loop); }
#endif
#endif
#else
/* 非兼容模式下只声明 ev_loop 结构体 */
typedef struct ev_loop ev_loop;
#endif
    /*---------------------------------------------------------------------*/

#endif

EV_CPP( })

#endif
