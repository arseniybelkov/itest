#include <stdio.h>
#include "../../itest.h"

extern int add(int x, int y);
extern int sub(int x, int y);

int main() {
	printf("main add(2, 1) = %d\n", add(2, 1));
	printf("main sub(2, 1) = %d\n", sub(2, 1));

	return 0;	
}
