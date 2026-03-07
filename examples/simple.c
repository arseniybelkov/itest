#include <stdio.h>
#include "../itest.c"


int do_smth(int x, int y) {
	int z = x + y;
	printf("x + y = %d", z);
	return z;
}

// ------------------
// TESTS
// ------------------

ITEST(two_plus_two) {
		
}

ITEST(two_plus_three) {
	
}
