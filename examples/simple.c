#include "../itest.h"

int add(int x, int y) {
	return x + y;
}

#ifdef TEST_ONLY_MAIN
int main() {
	int x = add(1 ,2);
	ASSERT_EQ(x, 3);
	ASSERT_EQ(x, 2);
	return 0;
}
#endif

// ------------------
// TESTS
// ------------------

ITEST_SUITE_BEGIN(SIMPLE_TESTS) {
	ITEST_SINGLE_WORKER(two_plus_two) {
		ASSERT_EQ(4, add(2, 2));
	}

	ITEST_SINGLE_WORKER(ten_plus_one_wrong) {
		int x = 10;
		int y = 1;
		ASSERT_EQ(21, add(x, y));
	}

	ITEST_SINGLE_WORKER(two_plus_three) {
		ASSERT_EQ(5, add(2, 3));
	}

	ITEST_SUITE_END()
}
