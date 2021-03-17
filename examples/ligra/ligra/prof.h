//
// Created by fred1 on 2020/11/25.
//

#ifndef PROF_HEADER_PROF_H
#define PROF_HEADER_PROF_H


#include <error.h>
#include <cerrno>
#include <linux/perf_event.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <cstdlib>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <unistd.h>

#define PROF_START()                                        \
    do {                                                    \
        PROF_IOCTL_(ENABLE);                                \
        PROF_IOCTL_(RESET);                                 \
    } while(0)

#define PROF_EVENT(type, config)  (uint32_t)(type), (uint64_t)(config),

#define PROF_EVENT_HW(config)  PROF_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_ ## config)

#define PROF_EVENT_SW(config)  PROF_EVENT(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_ ## config)

#define PROF_EVENT_CACHE(cache, op, result)                 \
    PROF_EVENT(PERF_TYPE_HW_CACHE,                          \
               (PERF_COUNT_HW_CACHE_ ## cache) |            \
               (PERF_COUNT_HW_CACHE_OP_ ## op << 8) |       \
               (PERF_COUNT_HW_CACHE_RESULT_ ## result << 16))

#define PROF_EVENT_NUMA(memory, op, result)

#define PROF_STOP()                                         \
    do {                                                    \
        PROF_IOCTL_(DISABLE);                               \
        PROF_READ_COUNTERS_(prof_event_buf_);               \
    } while(0)

#define PROF_COUNTERS  (prof_event_buf_ + 1)

#define PROF_DO(block)                                      \
    do {                                                    \
        uint64_t i_;                                        \
        PROF_STOP();                                        \
        for (i_ = 0; i_ < prof_event_cnt_; i_++) {          \
            uint64_t index = i_;                            \
            uint64_t counter = prof_event_buf_[i_ + 1];     \
            (void)index;                                    \
            (void)counter;                                  \
            block;                                          \
        }                                                   \
    } while (0)

#define PROF_CALL(callback)  PROF_DO(callback(index, counter))

#define PROF_FILE(file)                                     \
    PROF_DO(if (prof_event_cnt_ > 1) {                      \
            fprintf((file), "%lu\t%lu\n", index, counter);  \
        } else {                                            \
            fprintf((file), "%lu\n", counter);              \
        }                                                   \
    )

#define PROF_STDOUT()  PROF_FILE(stdout)

#define PROF_STDERR()  PROF_FILE(stderr)

/* DEFAULTS ---------------------------------------------- */
#ifndef PROF_EVENT_LIST
#ifdef PERF_COUNT_HW_REF_CPU_CYCLES /* since Linux 3.3 */
#define PROF_EVENT_LIST PROF_EVENT_HW(REF_CPU_CYCLES)
#else
#define PROF_EVENT_LIST PROF_EVENT_HW(CPU_CYCLES)
#endif
#endif

/* UTILITY ----------------------------------------------- */
#define PROF_ASSERT_(x)                                                        \
    do {                                                                       \
        if (!(x)) {                                                            \
            fprintf(stderr, "# %s:%d: PROF error", __FILE__, __LINE__);        \
            if (errno) {                                                       \
                fprintf(stderr, " (%s)", strerror(errno));                     \
            }                                                                  \
            printf("\n");                                                      \
            abort();                                                           \
        }                                                                      \
    } while (0)

#define PROF_IOCTL_(mode)                                                      \
    do {                                                                       \
        PROF_ASSERT_(ioctl(prof_fd_,                                           \
                           PERF_EVENT_IOC_ ## mode,                            \
                           PERF_IOC_FLAG_GROUP) != -1);                        \
    } while (0)

#define PROF_READ_COUNTERS_(buffer)                                            \
    do {                                                                       \
        const ssize_t to_read = sizeof(uint64_t) * (prof_event_cnt_ + 1);      \
        PROF_ASSERT_(read(prof_fd_, buffer, to_read) == to_read);              \
    } while (0)

/* SETUP  ------------------------------------------------- */
static int prof_fd_;
static uint64_t prof_event_cnt_;
static uint64_t *prof_event_buf_;

static void prof_init_(uint64_t dummy, ...) {
    uint32_t type;
    va_list ap;

    prof_fd_ = -1;
    prof_event_cnt_ = 0;
    va_start(ap, dummy);
    while (type = va_arg(ap, uint32_t), type != (uint32_t)-1) {
        struct perf_event_attr pe;
        uint64_t config;
        int fd;

        config = va_arg(ap, uint64_t);

        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.size = sizeof(struct perf_event_attr);
        pe.read_format = PERF_FORMAT_GROUP;
        pe.type = type;
        pe.config = config;
#ifdef PROF_USER_EVENTS_ONLY
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;
#endif

        fd = syscall(__NR_perf_event_open, &pe, 0, -1, prof_fd_, 0);
        PROF_ASSERT_(fd != -1);
        if (prof_fd_ == -1) {
            prof_fd_ = fd;
        }

        prof_event_cnt_++;
    }
    va_end(ap);

    prof_event_buf_ = (uint64_t *)malloc((prof_event_cnt_ + 1) *
                                         sizeof(uint64_t));
}

void __attribute__((constructor)) prof_init()
{
    prof_init_(0, PROF_EVENT_LIST /*,*/ (uint32_t)-1);
}

void __attribute__((destructor)) prof_fini()
{
    PROF_ASSERT_(close(prof_fd_) != -1);
    free(prof_event_buf_);
}

#endif //PROF_HEADER_PROF_H
