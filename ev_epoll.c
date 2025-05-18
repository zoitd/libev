/*
 * 关于epoll的总体说明：
 *
 * a) epoll会静默地从fd集合中移除文件描述符。由于没有其他方式告知我们某个fd已被移除，
 *    我们必须持续"重新武装"那些可能已发生变化的fd（kqueue也有同样问题，但代价小得多）。
 * b) 由于ADD与MOD操作不同，加上a)问题，会导致大量额外的系统调用，且似乎没有任何优势。
 * c) 无法处理fork或文件描述符复制（如dup）的情况，限制了其相对于poll的适用性，
 *    因此它并非通用的poll替代方案。
 * d) epoll在处理大量文件描述符（例如普通文件）时的行为与select不同。虽然不关键，
 *    但其他高级接口似乎都没有这种（相当不符合Unix风格的）限制。
 * e) epoll声称支持嵌入式使用，但实际上你永远不会从epoll fd上获取就绪事件
 *    （2.6.26及以下版本存在此缺陷，2.6.32及以上版本已修复）。
 * f) 当epoll_ctl返回EPERM错误时，表示该fd始终处于就绪状态。
 *
 * 本文件中大量"怪异代码"和复杂处理逻辑，都是为了应对epoll的这些设计缺陷。
 * 我们极力避免在常见使用模式中调用epoll_ctl系统调用，并妥善处理因接收已关闭
 * 或早已失效的文件描述符事件而引发的问题。
 */

#include <sys/epoll.h>

#define EV_EMASK_EPERM 0x80

static void epoll_modify(struct ev_loop *loop, int fd, int oev, int nev) {
    struct epoll_event ev;
    unsigned char oldmask;

    /*
    * 我们通过在此处忽略EPOLL_CTL_DEL来处理它
    * 基于假设fd无论如何已经消失
    * 如果这个假设错误，我们必须在epoll_poll中处理这个虚假事件
    * 如果fd被再次添加，我们尝试ADD操作，如果失败
    * 我们假设它仍然具有相同的事件掩码
    */
    if (!nev) return;

    oldmask = anfds[fd].emask;
    anfds[fd].emask = nev;

    /* store the generation counter in the upper 32 bits, the fd in the lower 32 bits */
    ev.data.u64 = (uint64_t)(uint32_t)fd | ((uint64_t)(uint32_t)++anfds[fd].egen << 32);
    ev.events = (nev & EV_READ ? EPOLLIN : 0) | (nev & EV_WRITE ? EPOLLOUT : 0);

    if (!epoll_ctl(backend_fd, oev && oldmask != nev ? EPOLL_CTL_MOD : EPOLL_CTL_ADD, fd, &ev))[[likely]]
        return;

    if (expect_true(errno == ENOENT)) {
        /* 如果ENOENT表示fd已消失，尝试采取正确的操作 */
        if (!nev) goto dec_egen;

        if (!epoll_ctl(backend_fd, EPOLL_CTL_ADD, fd, &ev)) return;

    } else if (expect_true(errno == EEXIST)) {
         /* EEXIST表示我们忽略了之前的DEL操作，但fd仍然活跃 */
            /* 如果内核掩码与新掩码相同，我们假设它没有改变 */
        if (oldmask == nev) goto dec_egen;

        if (!epoll_ctl(backend_fd, EPOLL_CTL_MOD, fd, &ev)) return;
    } else if (errno == EPERM)[[likely]] {
         /* EPERM表示fd总是就绪，但epoll过于挑剔,无法处理它，不像select或poll */
        anfds[fd].emask = EV_EMASK_EPERM;

        /* add fd to epoll_eperms, if not already inside */
        if (!(oldmask & EV_EMASK_EPERM)) {
            array_needsize(int, epoll_eperms, epoll_epermmax, epoll_epermcnt + 1, EMPTY2);
            epoll_eperms[epoll_epermcnt++] = fd;
        }

        return;
    }

    fd_kill(loop, fd);

dec_egen:
    /* 我们没有成功调用epoll_ctl，所以再次递减生成计数器 */
    --anfds[fd].egen;
}

static void epoll_poll(struct ev_loop *loop, ev_t   stamp timeout) {
    int i;
    int eventcnt;

    if (expect_false(epoll_epermcnt))
        timeout = 0.;

    /* epoll wait times cannot be larger than (LONG_MAX - 999UL) / HZ msecs, which is below */
    /* the default libev max wait time, however. */
    EV_RELEASE_CB;
    eventcnt = epoll_wait(backend_fd, epoll_events, epoll_eventmax, timeout * 1e3);
    EV_ACQUIRE_CB;

    if (expect_false(eventcnt < 0)) {
        if (errno != EINTR)
            ev_syserr("(libev) epoll_wait");

        return;
    }

    for (i = 0; i < eventcnt; ++i) {
        struct epoll_event *ev = epoll_events + i;

        int fd = (uint32_t)ev->data.u64; /* mask out the lower 32 bits */
        int want = anfds[fd].events;
        int got = (ev->events & (EPOLLOUT | EPOLLERR | EPOLLHUP) ? EV_WRITE : 0) | (ev->events & (EPOLLIN | EPOLLERR | EPOLLHUP) ? EV_READ : 0);

        /*
         * check for spurious notification.
         * this only finds spurious notifications on egen updates
         * other spurious notifications will be found by epoll_ctl, below
         * we assume that fd is always in range, as we never shrink the anfds array
         */
        if (expect_false((uint32_t)anfds[fd].egen != (uint32_t)(ev->data.u64 >> 32))) {
            /* recreate kernel state */
            postfork |= 2;
            continue;
        }

        if (expect_false(got & ~want)) {
            anfds[fd].emask = want;

            /*
             * we received an event but are not interested in it, try mod or del
             * this often happens because we optimistically do not unregister fds
             * when we are no longer interested in them, but also when we get spurious
             * notifications for fds from another process. this is partially handled
             * above with the gencounter check (== our fd is not the event fd), and
             * partially here, when epoll_ctl returns an error (== a child has the fd
             * but we closed it).
             */
            ev->events = (want & EV_READ ? EPOLLIN : 0) | (want & EV_WRITE ? EPOLLOUT : 0);

            /* pre-2.6.9 kernels require a non-null pointer with EPOLL_CTL_DEL, */
            /* which is fortunately easy to do for us. */
            if (epoll_ctl(backend_fd, want ? EPOLL_CTL_MOD : EPOLL_CTL_DEL, fd, ev)) {
                postfork |= 2; /* an error occurred, recreate kernel state */
                continue;
            }
        }

        fd_event(loop, fd, got);
    }

    /* if the receive array was full, increase its size */
    if (expect_false(eventcnt == epoll_eventmax)) {
        ev_free(epoll_events);
        epoll_eventmax = array_nextsize(sizeof(struct epoll_event), epoll_eventmax, epoll_eventmax + 1);
        epoll_events = (struct epoll_event *)ev_malloc(sizeof(struct epoll_event) * epoll_eventmax);
    }

    /* now synthesize events for all fds where epoll fails, while select works... */
    for (i = epoll_epermcnt; i--;) {
        int fd = epoll_eperms[i];
        unsigned char events = anfds[fd].events & (EV_READ | EV_WRITE);

        if (anfds[fd].emask & EV_EMASK_EPERM && events)
            fd_event(loop, fd, events);
        else {
            epoll_eperms[i] = epoll_eperms[--epoll_epermcnt];
            anfds[fd].emask = 0;
        }
    }
}

int static inline epoll_init(struct ev_loop *loop, int flags) {
#ifdef EPOLL_CLOEXEC
    backend_fd = epoll_create1(EPOLL_CLOEXEC);
    if (backend_fd < 0 && (errno == EINVAL || errno == ENOSYS))
#endif
        backend_fd = epoll_create(256);

    if (backend_fd < 0)
        return 0;

    fcntl(backend_fd, F_SETFD, FD_CLOEXEC);

    backend_mintime = 1e-3; /* epoll does sometimes return early, this is just to avoid the worst */
    backend_modify = epoll_modify;
    backend_poll = epoll_poll;

    epoll_eventmax = 64; /* initial number of events receivable per poll */
    epoll_events = (struct epoll_event *)ev_malloc(sizeof(struct epoll_event) * epoll_eventmax);

    return EVBACKEND_EPOLL;
}

void static inline epoll_destroy(struct ev_loop *loop) {
    ev_free(epoll_events);
    array_free(epoll_eperm, EMPTY);
}

void static inline epoll_fork(struct ev_loop *loop) {
    close(backend_fd);

    while ((backend_fd = epoll_create(256)) < 0)
        ev_syserr("(libev) epoll_create");

    fcntl(backend_fd, F_SETFD, FD_CLOEXEC);

    fd_rearm_all(loop);
}
