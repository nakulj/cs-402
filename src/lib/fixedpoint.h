/* 
 * File:   fixedpoint.h
 * Author: nakul
 *
 * Created on October 8, 2013, 9:35 PM
 */

#include "debug.h"
#include "stdint.h"
#include "stdbool.h"

#ifndef FIXEDPOINT_H
#define	FIXEDPOINT_H

#define DEBUG
#define UPPER 17
#define LOWER 14
#define EXP 16384
#define UPPER_MAX 131072
#define SIGN_BITMASK	0x80000000
#define UPPER_BITMASK	0x7fffc000
#define LOWER_BITMASK	0x00003fff

typedef unsigned int real;

real int2real(int num) {
#ifdef DEBUG
	ASSERT (num<UPPER_MAX && num>-UPPER_MAX);
#endif
	return (num>=0)?num*EXP:(num*(-1)*EXP | SIGN_BITMASK);
}

int real2int_floor(real num) {
	return num/EXP;
}

int real2int_round(real num) {
	return num+(EXP/2)*(num>=0?1:-1);
}

real add_int2real(int num1, real num2) {
	return int2real(num1) + num2;
}

real sub_int2real(int num1, real num2) {
	return -int2real(num1) + num2;
}

real mult_reals(real num1, real num2) {
	return ((int64_t)num1)*num2/EXP;
}

real div_reals(real num1, real num2) {
	return ((int64_t)num1)*EXP/num2;

}

#ifdef DEBUG
void print_real(real num) {
	bool sign = num<0;
	printf(
		"%c%d.%d",
		sign?'-':'+',
		(UPPER_BITMASK & num)/EXP,
		(LOWER_BITMASK & num)
	);
}
#endif DEBUG

#endif	/* FIXEDPOINT_H */

