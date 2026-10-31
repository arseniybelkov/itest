#include "../itest.h"

int add(int x, int y) {
	return x + y;
}

int main() {
	int x = add(1 ,2);
	ASSERT_EQ(x, 3);
	ASSERT_EQ(x, 2);
	return 0;
}

// ------------------
// TESTS
// ------------------

ITEST_SUITE_BEGIN(SIMPLE_TESTS) {
	ITEST(two_plus_two) {
		ASSERT_EQ(4, add(2, 2));
	}

	ITEST(ten_plus_one_wrong) {
		int x = 10;
		int y = 1;
		ASSERT_EQ(11, add(x, y));
	}

	ITEST(two_plus_three) {
		ASSERT_EQ(5, add(2, 3));
	}

	ITEST_SUITE_END()
}

ITEST_SUITE_BEGIN(COMPLEX_TESTS) {
	ITEST(a_plus_b) {
		int x = 1;
		int y = 2;
		ASSERT_LT(x + y, 4);		
	}
	ITEST_SUITE_END()
}
