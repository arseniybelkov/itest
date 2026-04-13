#include <assert.h>

int add(int x, int y) {
	return x + y;
}

// ------------------
// TESTS
// ------------------
#include "../itest.c"

ITEST_SUITE_BEGIN(SIMPLE_TESTS) {
	ITEST(two_plus_two) {
		ASSERT_EQ(4, add(2, 2));
	}

	ITEST(two_plus_three) {
		ASSERT_EQ(5, add(2, 3));
	}

	ITEST(ten_plus_one_wrong) {
		int x = 10;
		int y = 1;
		ASSERT_EQ(21, add(x, y));
	}

	ITEST_SUITE_END()
}
