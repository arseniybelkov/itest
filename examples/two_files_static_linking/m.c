#include "../../itest.h"

int add(int x, int y) {
	return x + y;
}

int sub(int x, int y) {
	return x - y;
}


ITEST_SUITE_BEGIN(mathtests) {
	ITEST(add_test) {
		ASSERT_EQ(3, add(2, 1));
	}

	ITEST(sub_test) {
		ASSERT_EQ(1, sub(2, 1));
	}
	
	ITEST_SUITE_END();
}
