#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS
 
#include <sys/epoll.h>
#include <stdio.h>            /* for perror() */
#include <unistd.h>           /* for syscall() */
#include <sys/syscall.h>      /* for __NR_* definitions */
#include <linux/aio_abi.h>    /* for AIO types and constants */
#include <fcntl.h>            /* O_RDWR */
#include <string.h>           /* memset() */
#include <inttypes.h>         /* uint64_t */
#include <stdlib.h>
 #include <sys/eventfd.h>

#define TEST_FILE   "aio_test_file"
#define TEST_FILE_SIZE  (128 * 1024)
#define NUM_EVENTS  128
#define ALIGN_SIZE  512
#define RD_WR_SIZE  1024
 
inline int io_setup(unsigned nr, aio_context_t *ctxp)
{
    return syscall(__NR_io_setup, nr, ctxp);
}
 
inline int io_submit(aio_context_t ctx, long nr,  struct iocb **iocbpp)
{
    return syscall(__NR_io_submit, ctx, nr, iocbpp);
}
 
inline int io_getevents(aio_context_t ctx, long min_nr, long max_nr,
        struct io_event *events, struct timespec *timeout)
{
    return syscall(__NR_io_getevents, ctx, min_nr, max_nr, events, timeout);
}
 
inline int io_destroy(aio_context_t ctx)
{
    return syscall(__NR_io_destroy, ctx);
}
 
inline int eventfd2(unsigned int initval, int flags)
{
    return syscall(__NR_eventfd2, initval, flags);
}
 
struct custom_iocb
{
    struct iocb iocb;
    int nth_request;
};
 
typedef void io_callback_t(aio_context_t ctx, struct iocb *iocb, long res, long res2);
 
void aio_callback(aio_context_t ctx, struct iocb *iocb, long res, long res2)
{
    struct custom_iocb *iocbp = (struct custom_iocb *)iocb;
    printf("nth_request: %d, request_type: %s, offset: %lld, length: %llu, res: %ld, res2: %ld\n",
            iocbp->nth_request, (iocb->aio_lio_opcode == IOCB_CMD_PREAD) ? "READ" : "WRITE",
            iocb->aio_offset, iocb->aio_nbytes, res, res2);
}
 
int main(int argc, char *argv[])
{
    int efd, fd, epfd;
    aio_context_t ctx;
    struct timespec tms;
    struct io_event events[NUM_EVENTS];
    struct custom_iocb iocbs[NUM_EVENTS];
    struct iocb *iocbps[NUM_EVENTS];
    struct custom_iocb *iocbp;
    int i, j, r;
    void *buf;
    void *aio_buf;
    struct epoll_event epevent;
 
    efd = eventfd(0, O_NONBLOCK | O_CLOEXEC );
    if (efd == -1) {
        perror("eventfd2");
        return 2;
    }
 
    fd = open(TEST_FILE, O_RDWR | O_CREAT /*| O_DIRECT*/, 0644);
    if (fd == -1) {
        perror("open");
        return 3;
    }
    ftruncate(fd, TEST_FILE_SIZE);
 
    ctx = 0;
    if (io_setup(NUM_EVENTS, &ctx)) {
        perror("io_setup");
        return 4;
    }
 
    if (posix_memalign(&buf, ALIGN_SIZE, RD_WR_SIZE * NUM_EVENTS)) {
        perror("posix_memalign");
        return 5;
    }
    printf("buf: %p\n", buf);
 
    for (i = 0, iocbp = iocbs; i < NUM_EVENTS; ++i, ++iocbp) {
        aio_buf = (void *)((char *)buf + (i*RD_WR_SIZE));
        memset(aio_buf, i + 65, RD_WR_SIZE);
 
        iocbp->iocb.aio_fildes = fd;
        iocbp->iocb.aio_lio_opcode = IOCB_CMD_PWRITE;//IOCB_CMD_PREAD
        iocbp->iocb.aio_buf = (uint64_t)aio_buf;
        iocbp->iocb.aio_offset = i * RD_WR_SIZE;
        iocbp->iocb.aio_nbytes = RD_WR_SIZE;
 
        iocbp->iocb.aio_flags = IOCB_FLAG_RESFD;
        iocbp->iocb.aio_resfd = efd;
 
        iocbp->iocb.aio_data = (__u64)aio_callback;
 
        iocbp->nth_request = i + 1;
        iocbps[i] = &iocbp->iocb;
    }
 
    if (io_submit(ctx, NUM_EVENTS, iocbps) != NUM_EVENTS) {
        perror("io_submit");
        return 6;
    }
 
    epfd = epoll_create(1);
    if (epfd == -1) {
        perror("epoll_create");
        return 7;
    }
 
    epevent.events = EPOLLIN | EPOLLET;
    epevent.data.ptr = NULL;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &epevent)) {
        perror("epoll_ctl");
        return 8;
    }
 
    i = 0;
    while (i < NUM_EVENTS) {
        uint64_t finished_aio;
 
        if (epoll_wait(epfd, &epevent, 1, -1) != 1) {
            perror("epoll_wait");
            return 9;
        }
 
        if (read(efd, &finished_aio, sizeof(finished_aio)) != sizeof(finished_aio)) {
            perror("read");
            return 10;
        }
 
        printf("finished io number: %"PRIu64"\n", finished_aio);
 
        while (finished_aio > 0) {
            tms.tv_sec = 0;
            tms.tv_nsec = 0;
            r = io_getevents(ctx, 1, finished_aio, events, &tms);
            if (r > 0) {
                for (j = 0; j < r; ++j) {
                    ((io_callback_t *)(events[j].data))(ctx, (struct iocb *)events[j].obj, events[j].res, events[j].res2);
                }
                i += r;
                finished_aio -= r;
            }
        }
    }
 
    close(epfd);
    free(buf);
    io_destroy(ctx);
    close(fd);
    close(efd);
   // remove(TEST_FILE);
 
    return 0;
}

