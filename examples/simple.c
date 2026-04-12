#include <assert.h>

int add(int x, int y) {
	return x + y;
}

// ------------------
// TESTS
// ------------------
#include "../itest.c"

ITEST_SUITE_BEGIN(SIMPLE_TESTS) {

	ITEST(two_plus_two, SIMPLE_TESTS) {
		ASSERT_EQ(4, add(2, 2));
	}

	ITEST(two_plus_three, SIMPLE_TESTS) {
		ASSERT_EQ(5, add(2, 3));
	}

	ITEST_SUITE_END(SIMPLE_TESTS)
}
