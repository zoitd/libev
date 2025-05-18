#include <errno.h>
#include <string.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include "ev.h"
#include "ev_wrap.h"

typedef double ev_tstamp;

[[gnu::noinline]] void static kqueue_change(struct ev_loop *loop, int fd, int filter, int flags, int fflags) {
    ++kqueue_changecnt;
    array_needsize(struct kevent, kqueue_changes, kqueue_changemax, kqueue_changecnt, EMPTY2);

    EV_SET(&kqueue_changes[kqueue_changecnt - 1], fd, filter, flags, fflags, 0, 0);
}

/* OS X at least needs this */
#ifndef EV_ENABLE
#define EV_ENABLE 0
#endif
#ifndef NOTE_EOF
#define NOTE_EOF 0
#endif

static void kqueue_modify(struct ev_loop *loop, int fd, int oev, int nev) {
    if (oev != nev) {
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

static void kqueue_poll(struct ev_loop *loop, ev_tstamp timeout) {
    int res, i;
    struct timespec ts;

    /* need to resize so there is enough space for errors */
    if (kqueue_changecnt > kqueue_eventmax) {
        ev_free(kqueue_events);
        kqueue_eventmax = array_nextsize(sizeof(struct kevent), kqueue_eventmax, kqueue_changecnt);
        kqueue_events = (struct kevent *)ev_malloc(sizeof(struct kevent) * kqueue_eventmax);
    }

    EV_RELEASE_CB;
    EV_TS_SET(ts, timeout);
    res = kevent(backend_fd, kqueue_changes, kqueue_changecnt, kqueue_events, kqueue_eventmax, &ts);
    EV_ACQUIRE_CB;
    kqueue_changecnt = 0;

    if (expect_false(res < 0)) {
        if (errno != EINTR)
            ev_syserr("(libev) kevent");

        return;
    }

    for (i = 0; i < res; ++i) {
        int fd = kqueue_events[i].ident;

        if (expect_false(kqueue_events[i].flags & EV_ERROR)) {
            int err = kqueue_events[i].data;

            /* we are only interested in errors for fds that we are interested in :) */
            if (anfds[fd].events) {
                if (err == ENOENT) /* resubmit changes on ENOENT */
                    kqueue_modify(loop, fd, 0, anfds[fd].events);
                else if (err == EBADF) /* on EBADF, we re-check the fd */
                {
                    if (fd_valid(fd))
                        kqueue_modify(loop, fd, 0, anfds[fd].events);
                    else
                        fd_kill(loop, fd);
                } else /* on all other errors, we error out on the fd */
                    fd_kill(loop, fd);
            }
        } else
            fd_event(
                loop,
                fd,
                kqueue_events[i].filter == EVFILT_READ    ? EV_READ
                : kqueue_events[i].filter == EVFILT_WRITE ? EV_WRITE
                                                          : 0);
    }

    if (expect_false(res == kqueue_eventmax)) {
        ev_free(kqueue_events);
        kqueue_eventmax = array_nextsize(sizeof(struct kevent), kqueue_eventmax, kqueue_eventmax + 1);
        kqueue_events = (struct kevent *)ev_malloc(sizeof(struct kevent) * kqueue_eventmax);
    }
}

int static inline kqueue_init(struct ev_loop *loop, int flags) {
    /* initialize the kernel queue */
    kqueue_fd_pid = getpid();
    if ((backend_fd = kqueue()) < 0) return 0;

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

void static inline kqueue_destroy(struct ev_loop *loop) {
    ev_free(kqueue_events);
    ev_free(kqueue_changes);
}

void static inline kqueue_fork(struct ev_loop *loop) {
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
