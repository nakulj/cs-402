#undef NDEBUG

#include "lib/fixedpoint.h"
#include "debug.h"

void test_fixed_point(void) {
	int ints[19] = {1,2,3,4,8,16,0,32,145,754,63,-73,-1,-2,-3,0,-4,-8,-128};
	real reals[19];
	int i;
	int iSum=0;
	int rSum=0;
    
    // Check int2real conversion.
    for (i=0; i<19; i++) {
        reals[i] = int2real(ints[i]);
        if (ints[i] != real2int_round(reals[i])) {
            printf("ERR conv: %d %d", ints[i]);
            print_real(reals[i]);
            printf("\n");
            return;
        }
    }

    // Check arithmatic functions.
    for (i=1; i<19; i++) {
        // variables for saving test results.
        int res1, res2, res3;
        
        // add_int2real
        res1 = real2int_round(add_int2real(reals[i], ints[i-1]));
        res2 = real2int_round(add_int2real(reals[i-1], ints[i]));
 
        if ((ints[i] + ints[i-1] != res1) || (ints[i] + ints[i-1] != res2)) {
            printf("ERR add_int2real: %d + %d != %d or %d\n", ints[i], ints[i-1], res1, res2);
            return;
        }
      
        // add_reals
        res1 = real2int_round(add_reals(reals[i], reals[i-1]));
        if ((ints[i] + ints[i-1] != res1)) {
            printf("ERR add_reals: %d + %d != %d\n", ints[i], ints[i-1], res1);
            return;
        }

        // sub_int2real
        res1 = real2int_round(sub_int2real(reals[i], ints[i-1]));
        res2 = real2int_round(sub_int2real(reals[i-1], ints[i]));
 
        if ((ints[i] - ints[i-1] != res1) || (ints[i] - ints[i-1] != (-1)*res2)) {
            printf("ERR add_int2real: %d - %d != %d or %d\n", ints[i], ints[i-1], res1, (-1)*res2);
            return;
        }

        // sub_reals
        res1 = real2int_round(sub_reals(reals[i], reals[i-1]));
        if ((ints[i] - ints[i-1] != res1)) {
            printf("ERR add_reals: %d - %d != %d\n", ints[i], ints[i-1], res1);
            return;
        }

        // mult_reals
        res1 = real2int_round(mult_reals(reals[i], reals[i-1]));
        if ((ints[i] * ints[i-1] != res1)) {
            printf("ERR add_reals: %d * %d != %d\n", ints[i], ints[i-1], res1);
            return;
        }
       
        // mult_int2real
        res1 = real2int_round(mult_int2real(reals[i], ints[i-1]));
        res2 = real2int_round(mult_int2real(reals[i-1], ints[i]));
        if ((ints[i] * ints[i-1] != res1) || (ints[i] * ints[i-1] != res2)) {
            printf("ERR mult_int2real: %d * %d != %d or %d\n", ints[i], ints[i-1], res1, res2);
            return;
        }

        // Test divisions. No way to test automatically since system doesn't support fraction.
        print_real(div_reals(reals[i], reals[i-1]));
        printf("  ");
        print_real(div_reals(reals[i], ints[i-1]));
        printf("\n");
    }
}

/*
      Nakul's code.
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

      ASSERT(ints[i]*ints[i-1] == real2int_round(mult_reals(reals[i],reals[i-1])));*/


