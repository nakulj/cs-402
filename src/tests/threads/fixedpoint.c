#undef NDEBUG

#include "lib/fixedpoint.h"
#include "debug.h"

void test_fixed_point(void) {
	int ints[17] = {1,2,3,4,8,16,32,145,754,63,-73,-1,-2,-3,-4,-8,-128};
	real reals[17];
	int i;
	int iSum=0;
	int rSum=0;
    for(i=0; i<17; i++) {
		reals[i]= int2real(ints[i]);
		print_real(reals[i]);
		printf(" %x\n", reals[i]);
		printf("%d %d \n",
			real2int_round(reals[i]),
			ints[i]
		);
		iSum+=ints[i];
		rSum=real2int_round(
			add_int2real(rSum, reals[i])
		);
		printf("%d %d\n",
			iSum,
			rSum
		);
        if (i==0) continue;

		printf("mul: %d %d\n",
            ints[i]*ints[i-1],
            real2int_round(mult_reals(reals[i],reals[i-1])));

        ASSERT(ints[i]*ints[i-1] == real2int_round(mult_reals(reals[i],reals[i-1])));

	}
}

