# itest
Inlined tests for C/C++


`itest` allows you to write tests right where your implementation is:
```C
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

	ITEST_SUITE_END()
}
```