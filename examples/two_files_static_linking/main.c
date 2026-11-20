#include <stdio.h>
#include "../../itest.h"

extern int add(int x, int y);
extern int sub(int x, int y);

int main() {
	printf("main add(2, 1) = %d\n", add(2, 1));
	printf("main sub(2, 1) = %d\n", sub(2, 1));

	return 0;	
}

ITEST_SUITE_BEGIN(onemoremathtest) {
	ITEST(add_test) {
		ASSERT_EQ(4, add(2, 2));
	}

	ITEST(sub_test) {
		ASSERT_NE(3, sub(2, 2));
	}
	ITEST_SUITE_END();
}
