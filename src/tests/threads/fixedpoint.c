#undef NDEBUG

#include "lib/fixedpoint.h"

void test_fixed_point(void) {
	int ints[17] = {1,2,3,4,8,16,32,145,754,63,-73,-1,-2,-3,-4,-8,-128};
	real reals[17];
	int i;
    for(i=0; i<17; i++) {
		reals[i]= int2real(ints[i]);
		print_real(reals[i]);
		printf(" %x\n", (int)reals[i]);
	}
}

