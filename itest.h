#include <stdio.h>
#include <stdlib.h>

/* Typedefs */

typedef unsigned char ___ITEST_u8;
typedef unsigned int ___ITEST_u32;
typedef void (*___ITEST_test_body)(void);

/* Test Registry implementation */

#define ___ITEST_CAPACITY 128

static struct {
    ___ITEST_test_body list[___ITEST_CAPACITY] = { NULL };
    ___ITEST_u32 size = 0;
} ___ITEST_registry;

static ___ITEST_u8 ___ITEST_add_test(___ITEST_test_body fn) {
    if (___ITEST_registry.size == ___ITEST_CAPACITY) {
        return 1;
    }
    ___ITEST_registry.list[___ITEST_registry.size++] = fn;
    return 0;
}

static void ___ITEST_execute_all_tests() {
    for (___ITEST_u32 i = 0; i != ___ITEST_registry.size; ++i) {
        ___ITEST_registry.list[i]();
    }
}

/* Test Results & Statuses */

enum ___ITEST_test_status : ___ITEST_u8 {
    ___ITEST_test_status_success = 0,
    ___ITEST_test_status_failure = 1,
};

struct ___ITEST_location {
    const char* file;
    int line;
};

struct ___ITEST_test_result {
    enum ___ITEST_test_status status;
    union {
        struct {
            const char* expression;
            struct ___ITEST_location location;
        } failure;
        struct {} success;
    } as;
};

static struct ___ITEST_test_result ___ITEST_test_succeed() {
    struct ___ITEST_test_result success = { .status = ___ITEST_test_status_success, .as = {} };
    return success;
}

static struct ___ITEST_test_result ___ITEST_test_failed(const char* expression, const struct ___ITEST_location location) {
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

static const char* ___ITEST_NO_COLOR = "\x1B[0m";
static const char* ___ITEST_RED_COLOR = "\x1B[31m";
static const char* ___ITEST_GREEN_COLOR = "\x1B[32m";

static void ___ITEST_print_test_result(
    const char* suite, const char* test,
    const struct ___ITEST_test_result result
) {
    ___ITEST_u8 is_success = result.status == ___ITEST_test_status_success;
    const char* test_status_str = is_success ? "OK" : "FAILED";
    FILE* stream = is_success ? stdout : stderr;

    #ifdef ITEST_NO_COLOR_IO
    fprintf(stream, "Test %s::%s ... %s\n", suite, test, test_status_str);
    #else
    const char* color = is_success ? ___ITEST_GREEN_COLOR : ___ITEST_RED_COLOR;
    fprintf(stream, "%sTest %s::%s ... %s%s\n", ___ITEST_NO_COLOR, suite, test, color, test_status_str);
    #endif
    if (!is_success) {
        const char* expression = result.as.failure.expression;
        struct ___ITEST_location location = result.as.failure.location;
        fprintf(stream, "Reason:\n%s at %s:%d\n", expression, location.file, location.line);
    }
}

static struct ___ITEST_test_suite ___ITEST_test_suite = {
    .count = 0, .name = NULL, .current_test = NULL, .current_test_status = ___ITEST_test_status_failure
};

#define ___ITEST_ENTRYPOINT __attribute__((constructor))

#define ITEST_SUITE_BEGIN(itest_test_suite)\
___ITEST_ENTRYPOINT void ___itest_main_##itest_test_suite() {\
    ___ITEST_test_suite.name = #itest_test_suite;\
    
#define ITEST_SUITE_END()\
    /* This way `int main() { ... }` never gets executed */\
    exit(0);\
}

#define ITEST(itest_test_name)\
    if (___ITEST_test_suite.count != 0) {\
        ___ITEST_print_test_result(\
            ___ITEST_test_suite.name,\
            ___ITEST_test_suite.current_test,\
            ___ITEST_test_succeed()\
        );\
    }\
    \
    struct ___ITEST_test_result ___ITEST_status_##itest_test_name = { };\
    ___ITEST_test_suite.current_test = #itest_test_name;\
    ___ITEST_test_suite.count++;\
    /* Test code goes here */\

/* Asserts implementation */

#define ASSERT_EQ(x, y) ASSERT((x) == (y))

#define ASSERT_NE(x, y) ASSERT((x) != (y))

#define ASSERT_GT(x, y) ASSERT((x) > (y))

#define ASSERT_LT(x, y) ASSERT((x) < (y))

#define ASSERT_GE(x, y) ASSERT((x) >= (y))

#define ASSERT_LE(x, y) ASSERT((x) <= (y))

#define ASSERT(expr) do {\
    const char* expression = #expr;\
    const struct ___ITEST_location location = ___ITEST_HERE();\
    ___ITEST_assert_impl((expr), expression, location, &___ITEST_test_suite);\
} while (0)

static void ___ITEST_assert_impl(
    const ___ITEST_u8 expr_success, const char* expression,
    const struct ___ITEST_location loc, const struct ___ITEST_test_suite* suite
) {
    const ___ITEST_u8 is_inside_a_test = suite->name != NULL;
    if (expr_success) {
        return;
    }
    const struct ___ITEST_test_result res = ___ITEST_test_failed(expression, loc);
    if (is_inside_a_test) {
        ___ITEST_print_test_result(suite->name, suite->current_test, res);
    } else {
        #ifdef ITEST_NO_COLOR_IO
        fprintf(stderr, "Assert failed: %s at %s:%d\n", expression, loc.file, loc.line);
        #else
        fprintf(stderr, "%sAssert failed: %s at %s:%d\n", ___ITEST_RED_COLOR, expression, loc.file, loc.line);
        #endif
    }
    exit(1);
}

/* Misscelaneous */

#define ___ITEST_HERE() { .file = __FILE__, .line = __LINE__ }