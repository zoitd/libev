
#define VAR(name, decl) decl
#define VARx(type, name) VAR((name), (type name))

struct ev_loop
{
    using ev_tstamp = double;
    ev_tstamp ev_rt_now;

    ev_tstamp now_floor;        /* 上次刷新实时时间的时间点 */
    ev_tstamp mn_now;           /* 单调时钟当前时间 */
    VARx(ev_tstamp, rtmn_diff); /* 实时时间与单调时间的差值 */

    /* 用于事件的反向馈送 */
    VARx(W *, rfeeds);   /* 反向馈送事件链表 */
    VARx(int, rfeedmax); /* 反向馈送事件最大数量 */
    VARx(int, rfeedcnt); /* 当前反向馈送事件计数 */

    VAR(pendings, ANPENDING *pendings[NUMPRI]); /* 按优先级分类的待处理事件数组 */
    VAR(pendingmax, int pendingmax[NUMPRI]);    /* 每个优先级的待处理事件最大数量 */
    VAR(pendingcnt, int pendingcnt[NUMPRI]);    /* 每个优先级的当前待处理事件计数 */
    VARx(int, pendingpri);                      /* 当前待处理的最高优先级 */
    VARx(ev_prepare, pending_w);                /* 待处理观察者占位符，用于触发待处理事件 */

    VARx(ev_tstamp, io_blocktime);      /* I/O操作最大阻塞时间 */
    VARx(ev_tstamp, timeout_blocktime); /* 超时事件最大阻塞时间 */

    VARx(int, backend);   /* 当前使用的后端I/O多路复用机制 */
    VARx(int, activecnt); /* 活跃事件总数("refcount") */

    VARx(EV_ATOMIC_T, loop_done); /* 循环结束标志，由ev_break设置 */

    VARx(int, backend_fd);            /* 后端I/O多路复用机制使用的文件描述符 */
    VARx(ev_tstamp, backend_mintime); /* 后端定时器的最小分辨率(假设值) */

    VAR(backend_modify, void (*backend_modify)(struct ev_loop *loop, int fd, int oev, int nev)); /* 修改后端文件描述符状态的函数指针 */
    VAR(backend_poll, void (*backend_poll)(struct ev_loop *loop, ev_tstamp timeout));            /* 轮询后端事件的函数指针 */
    VARx(ANFD *, anfds);
    VARx(int, anfdmax);                    /* 活动文件描述符数组的最大容量 */
    VAR(evpipe, int evpipe[2]);            /* 用于线程间通信的管道 */
    VARx(ev_io, pipe_w);                   /* 管道读端的I/O观察者 */
    VARx(EV_ATOMIC_T, pipe_write_wanted);  /* 需要写入管道的标志(原子操作) */
    VARx(EV_ATOMIC_T, pipe_write_skipped); /* 跳过管道写入的次数(原子操作) */

#if !defined(_WIN32) || EV_GENWRAP
    VARx(pid_t, curpid); /* 当前进程ID */
#endif

    VARx(char, postfork); /* fork后需要重建内核状态的标志 */

#if EV_USE_SELECT || EV_GENWRAP
    VARx(void *, vec_ri); /* select后端的读输入文件描述符集 */
    VARx(void *, vec_ro); /* select后端的读输出文件描述符集 */
    VARx(void *, vec_wi); /* select后端的写输入文件描述符集 */
    VARx(void *, vec_wo); /* select后端的写输出文件描述符集 */
#if defined(_WIN32) || EV_GENWRAP
    VARx(void *, vec_eo); /* select后端的异常文件描述符集(Windows特有) */
#endif
    VARx(int, vec_max); /* select后端文件描述符集的最大大小 */
#endif

#if EV_USE_POLL || EV_GENWRAP
    VARx(struct pollfd *, polls); /* poll后端的文件描述符数组 */
    ;
    VARx(int, pollmax);    /* poll数组的最大容量 */
    VARx(int, pollcnt);    /* poll数组的当前使用数量 */
    VARx(int *, pollidxs); /* 文件描述符到poll数组索引的映射 */
    VARx(int, pollidxmax); /* poll索引映射的最大容量 */
#endif

#if EV_USE_EPOLL || EV_GENWRAP
    VARx(struct epoll_event *, epoll_events); /* epoll后端的事件数组 */
    VARx(int, epoll_eventmax);                /* epoll事件数组的最大容量 */
    VARx(int *, epoll_eperms);                /* epoll权限错误文件描述符数组 */
    VARx(int, epoll_epermcnt);                /* epoll权限错误计数 */
    VARx(int, epoll_epermmax);                /* epoll权限错误数组最大容量 */
#endif

#if EV_USE_KQUEUE || EV_GENWRAP
    VARx(pid_t, kqueue_fd_pid);            /* kqueue文件描述符所属进程ID */
    VARx(struct kevent *, kqueue_changes); /* kqueue变更事件数组 */
    VARx(int, kqueue_changemax);           /* kqueue变更数组最大容量 */
    VARx(int, kqueue_changecnt);           /* kqueue当前变更计数 */
    VARx(struct kevent *, kqueue_events);  /* kqueue返回事件数组 */
    VARx(int, kqueue_eventmax);            /* kqueue事件数组最大容量 */
#endif

#if EV_USE_PORT || EV_GENWRAP
    VARx(struct port_event *, port_events); /* Solaris端口后端的事件数组 */
    VARx(int, port_eventmax);               /* 端口事件数组最大容量 */
#endif

#if EV_USE_IOCP || EV_GENWRAP
    VARx(HANDLE, iocp); /* Windows IOCP后端的完成端口句柄 */
#endif

    VARx(int *, fdchanges);
    VARx(int, fdchangemax); /* 文件描述符变更数组最大容量 */
    VARx(int, fdchangecnt); /* 当前文件描述符变更计数 */

    VARx(ANHE *, timers); /* 定时器堆数组 */
    VARx(int, timermax);  /* 定时器堆最大容量 */
    VARx(int, timercnt);  /* 当前定时器计数 */

#if EV_PERIODIC_ENABLE || EV_GENWRAP
    VARx(ANHE *, periodics); /* 周期性定时器堆数组 */
    VARx(int, periodicmax);  /* 周期性定时器堆最大容量 */
    VARx(int, periodiccnt);  /* 当前周期性定时器计数 */
#endif

#if EV_IDLE_ENABLE || EV_GENWRAP
    VAR(idles, ev_idle **idles[NUMPRI]); /* 按优先级分类的空闲观察者数组 */
    VAR(idlemax, int idlemax[NUMPRI]);   /* 每个优先级的空闲观察者最大数量 */
    VAR(idlecnt, int idlecnt[NUMPRI])    /* 每个优先级的当前空闲观察者计数 */
#endif
    ;
    VARx(int, idleall); /* 空闲观察者总数 */

    VARx(struct ev_prepare **, prepares); /* 准备观察者数组 */
    VARx(int, preparemax);                /* 准备观察者最大数量 */
    VARx(int, preparecnt)                 /* 当前准备观察者计数 */

        ;
    VARx(struct ev_check **, checks); /* 检查观察者数组 */
    VARx(int, checkmax);              /* 检查观察者最大数量 */
    VARx(int, checkcnt);              /* 当前检查观察者计数 */

#if EV_FORK_ENABLE || EV_GENWRAP
    VARx(struct ev_fork **, forks); /* fork观察者数组 */
    VARx(int, forkmax);             /* fork观察者最大数量 */
    VARx(int, forkcnt);             /* 当前fork观察者计数 */
#endif

#if EV_CLEANUP_ENABLE || EV_GENWRAP
    VARx(struct ev_cleanup **, cleanups); /* 清理观察者数组 */
    VARx(int, cleanupmax);                /* 清理观察者最大数量 */
    VARx(int, cleanupcnt);                /* 当前清理观察者计数 */
#endif

#if EV_ASYNC_ENABLE || EV_GENWRAP
    VARx(EV_ATOMIC_T, async_pending); /* 待处理异步事件标志(原子操作) */
    VARx(struct ev_async **, asyncs); /* 异步观察者数组 */
    VARx(int, asyncmax);              /* 异步观察者最大数量 */
    VARx(int, asynccnt);              /* 当前异步观察者计数 */
#endif

#if EV_USE_INOTIFY || EV_GENWRAP
    VARx(int, fs_fd);                                /* inotify文件描述符 */
    VARx(ev_io, fs_w);                               /* inotify I/O观察者 */
    VARx(char, fs_2625);                             /* 是否运行在Linux 2.6.25或更新版本 */
    VAR(fs_hash, ANFS fs_hash[EV_INOTIFY_HASHSIZE]); /* inotify文件系统监视哈希表 */
#endif

    VARx(EV_ATOMIC_T, sig_pending); /* 待处理信号标志(原子操作) */
#if EV_USE_SIGNALFD || EV_GENWRAP
    VARx(int, sigfd);          /* signalfd文件描述符 */
    VARx(ev_io, sigfd_w);      /* signalfd I/O观察者 */
    VARx(sigset_t, sigfd_set); /* signalfd信号集 */
#endif

    VARx(unsigned int, origflags); /* 事件循环的原始标志位 */

#if EV_FEATURE_API || EV_GENWRAP
    VARx(unsigned int, loop_count); /* 事件循环迭代/阻塞总次数 */
    VARx(unsigned int, loop_depth); /* ev_run进入次数 - ev_run离开次数 */

    VARx(void *, userdata); /* 用户自定义数据指针 */

    /* C++ doesn't support the ev_loop_callback typedef here. stinks. */

    VAR(release_cb, void (*release_cb)(struct ev_loop *loop) noexcept); /* 释放事件循环锁时的回调 */
    VAR(acquire_cb, void (*acquire_cb)(struct ev_loop *loop) noexcept); /* 获取事件循环锁时的回调 */
    VAR(invoke_cb, ev_loop_callback invoke_cb);                         /* 调用事件循环时的回调 */
#endif

#undef VARx
#undef VAR
};