#include <stdio.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <sys/mman.h>

#if defined(_WIN32) || defined(_WIN64)
#define __WINDOWS__
#endif

/*
    Synchronization
    ___ITEST_Mutex is used for guarding stdout / stderr,
    when tests results are printed.

    It uses futex, that is shared among all the processes.
    Futex is allocated via `mmap(..., MAP_SHARED, ...)` syscall.
*/

struct ___ITEST_mutex {
    _Atomic uint32_t state;
};

enum ___ITEST_mutex_state : uint32_t {
    ___ITEST_mutex_state_free = 0,
    ___ITEST_mutex_state_contention = 1,
    ___ITEST_mutex_state_locked = 2,
};

#ifdef __APPLE__
#define UL_COMPARE_AND_WAIT 1

int __ulock_wait(uint32_t operation, void* addr, uint64_t value,
                uint32_t timeout); /* timeout is specified in microseconds */
int __ulock_wake(uint32_t operation, void* addr, uint64_t wake_value);

#elif defined(__WINDOWS__)
#include <synchapi.h>

#else // LINUX / BSD

#include <sys/syscall.h>
#include <linux/futex.h>

static int ___ITEST_futex(uint32_t* uaddr, int op, int val,
                 const struct timespec* timeout, int* uaddr2, int val3) {
  return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}
#endif

/*
    FUTEX_WAKE and FUTEX_WAIT are used without *_PRIVATE flag,
    because in the `itest` case futex is not bounded to one process, but rather
    establishes communication between a number of them.
*/

static void ___ITEST_futex_wake_one(uint32_t* waiters) {
    #ifdef __APPLE__
    __ulock_wake(UL_COMPARE_AND_WAIT, waiters, 0);
    #elif defined(__WINDOWS__)
    WakeByAddressSingle(waiters);
    #else
    ___ITEST_futex(waiters, FUTEX_WAKE, 1, NULL, NULL, 0);
    #endif
}

static void ___ITEST_futex_wait(uint32_t* loc, uint32_t old) {
    #ifdef __APPLE__
    __ulock_wait(UL_COMPARE_AND_WAIT, loc, old, 0);
    #elif defined(__WINDOWS__)
    WaitOnAddress(loc, &old, sizeof(uint32_t*), INFINITE);
    #else
    ___ITEST_futex(loc, FUTEX_WAIT, old, NULL, NULL, 0);
    #endif
}

static void ___ITEST_mutex_lock(struct ___ITEST_mutex* mutex) {
    uint32_t mutex_is_free = ___ITEST_mutex_state_free;
    if (!atomic_compare_exchange_strong(&mutex->state, &mutex_is_free, ___ITEST_mutex_state_contention)) {
        while (atomic_exchange(&mutex->state, ___ITEST_mutex_state_locked) != ___ITEST_mutex_state_free) {
            ___ITEST_futex_wait((uint32_t*) &mutex->state, ___ITEST_mutex_state_locked);
        }
    }
}

static void ___ITEST_mutex_unlock(struct ___ITEST_mutex* mutex) {
    uint32_t* waiters = (uint32_t*) &mutex->state;
    if (atomic_exchange(&mutex->state, ___ITEST_mutex_state_free) != ___ITEST_mutex_state_free) {
        ___ITEST_futex_wake_one(waiters);
    }
}

static struct ___ITEST_mutex* ___ITEST_stdout_guard = NULL;

static void ___ITEST_init_stdout_guard() {
    const int no_file_descriptor = -1;
    const int no_offset = 0;

    // Allocating futex that is shared between processes.
    // See the example at https://man7.org/linux/man-pages/man2/futex.2.html
    ___ITEST_stdout_guard = (struct ___ITEST_mutex*) mmap(
        NULL, sizeof(struct ___ITEST_mutex),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS,
        no_file_descriptor, no_offset
    );

    if (___ITEST_stdout_guard == MAP_FAILED) {
        fprintf(stderr, "MMAP failed\n");
        exit(1);
    }

    ___ITEST_mutex_unlock(___ITEST_stdout_guard);
}

/*
    Test Results & Statuses
*/

enum ___ITEST_test_status : uint8_t {
    ___ITEST_test_status_running = 0,
    ___ITEST_test_status_success = 1,
    ___ITEST_test_status_failure = 2,
};

struct ___ITEST_location {
    const char* file;
    int line;
};

struct ___ITEST_test_result {
    enum ___ITEST_test_status status;
    union {
        struct {} running;
        struct {
            const char* expression;
            struct ___ITEST_location location;
        } failure;
        struct {} success;
    } as;
};

inline static struct ___ITEST_test_result ___ITEST_test_succeed() {
    struct ___ITEST_test_result success = { .status = ___ITEST_test_status_success, .as = {} };
    return success;
}

inline static struct ___ITEST_test_result ___ITEST_test_failed(const char* expression, const struct ___ITEST_location location) {
    struct ___ITEST_test_result failure = { .status = ___ITEST_test_status_failure, .as = {} };
    failure.as.failure.expression = expression;
    failure.as.failure.location = location;
    return failure;
}

struct ___ITEST_test_suite {
    size_t count;
    const char* name;
    const char* current_test;
    enum ___ITEST_test_status current_test_status;
};

#define ___ITEST_KNRM "\x1B[0m" // resets color
#define ___ITEST_KRED "\x1B[31m"
#define ___ITEST_KGRN "\x1B[32m"

static void ___ITEST_print_test_result(const char* suite, const char* test, struct ___ITEST_test_result result) {
    uint8_t is_success = result.status == ___ITEST_test_status_success;
    const char* test_status_str = is_success ? "OK" : "FAILED";
    const char* color = is_success ? ___ITEST_KGRN : ___ITEST_KRED;
    FILE* stream = is_success ? stdout : stderr;

    // printf("BEFORE LOCK\n");
    // ___ITEST_mutex_lock(___ITEST_stdout_guard);
    // printf("AFTER LOCK\n");

    fprintf(stream, "%sTest %s::%s ... %s%s\n", ___ITEST_KNRM, suite, test, color, test_status_str);
    if (!is_success) {
        const char* expression = result.as.failure.expression;
        struct ___ITEST_location location = result.as.failure.location;
        fprintf(stream, "Reason:\n%s at %s:%d\n", expression, location.file, location.line);
    }

    // ___ITEST_mutex_unlock(___ITEST_stdout_guard);
}

static struct ___ITEST_test_suite ___ITEST_test_suite = {
    .count = 0, .name = NULL, .current_test = NULL, .current_test_status = ___ITEST_test_status_running
};

#define ITEST_SUITE_BEGIN(itest_test_suite) int main() {\
    pid_t ___ITEST_is_child_process = 0;\
    ___ITEST_init_stdout_guard();\
    ___ITEST_test_suite.name = #itest_test_suite;\
    
#define ITEST_SUITE_END()\
    ___ITEST_SUITE_SUCCEED();\
}

#define ITEST_SINGLE_WORKER(itest_test_name)\
    if (___ITEST_test_suite.count != 0) {\
        ___ITEST_print_test_result(\
            ___ITEST_test_suite.name,\
            ___ITEST_test_suite.current_test,\
            ___ITEST_test_succeed()\
        );\
        /* ___ITEST_SUITE_SUCCEED(); */ \
    }\
    \
    struct ___ITEST_test_result ___ITEST_status_##itest_test_name = { };\
    ___ITEST_test_suite.current_test = #itest_test_name;\
    ___ITEST_test_suite.count++;\
    /* Test code goes here */\


#define ITEST(itest_test_name)\
    /* Previously launched child process finishes its execution here */\
    if (___ITEST_test_suite.count != 0 && ___ITEST_is_child_process) {\
        ___ITEST_print_test_result(\
            ___ITEST_test_suite.name,\
            ___ITEST_test_suite.current_test,\
            ___ITEST_test_succeed()\
        );\
        ___ITEST_SUITE_SUCCEED();\
    }\
    \
    struct ___ITEST_test_result ___ITEST_status_##itest_test_name = {  };\
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {\
        perror("signal");\
        ___ITEST_SUITE_FAIL();\
    }\
    \
    ___ITEST_test_suite.current_test = #itest_test_name;\
    \
    pid_t ___ITEST_pid_for_##itest_test_name = fork();\
    ___ITEST_test_suite.count += 1;\
    ___ITEST_is_child_process = ___ITEST_pid_for_##itest_test_name == 0;\
    \
    if (___ITEST_pid_for_##itest_test_name == -1) {\
        ___ITEST_SUITE_FAIL();\
    } else if (___ITEST_pid_for_##itest_test_name != 0) {\
        /* do nothing */ \
    }\
    else /* here goes the code for the test */

#define ASSERT_EQ(x, y) ASSERT((x) == (y))

#define ASSERT_NE(x, y) ASSERT((x) != (y))

#define ASSERT_GT(x, y) ASSERT((x) > (y))

#define ASSERT_LT(x, y) ASSERT((x) < (y))

#define ASSERT_GE(x, y) ASSERT((x) >= (y))

#define ASSERT_LE(x, y) ASSERT((x) <= (y))

#define ASSERT(expr) do {\
    const char* expression = #expr;\
    struct ___ITEST_location location = ___ITEST_HERE();\
    ___ITEST_assert_impl((expr), expression, &___ITEST_test_suite, location);\
} while (0)

inline static struct ___ITEST_test_result ___ITEST_assert_impl(
    uint8_t expr_success, const char* expression,
    const struct ___ITEST_test_suite* suite, struct ___ITEST_location loc
) {
    if (!expr_success) {
        struct ___ITEST_test_result res = ___ITEST_test_failed(expression, loc);
        ___ITEST_print_test_result(suite->name, suite->current_test, res);
        return res;
    }
    return ___ITEST_test_succeed();
}

#define ___ITEST_HERE() { .file = __FILE__, .line = __LINE__ }

#define ___ITEST_SUITE_SUCCEED() return 0

#define ___ITEST_SUITE_FAIL() return 1