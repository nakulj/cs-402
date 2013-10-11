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
	return (num>=0)?num*EXP:((num*-1)*EXP | SIGN_BITMASK);
}

int real2int_floor(real num) {
	return (num & (~SIGN_BITMASK))/EXP | (num & SIGN_BITMASK);
}

int real2int_round(real num) {
	bool sign = num & SIGN_BITMASK;
	return ((num+EXP/2) & (~SIGN_BITMASK))/EXP * (sign?-1:1);
}

real add_reals(real num1, real num2) {
    bool is_1gt2 = (num1&(~SIGN_BITMASK))>(num2&(~SIGN_BITMASK));
    bool is_diffsign = (num1 ^ num2) & SIGN_BITMASK;
    real sign = (is_diffsign && is_1gt2)?(num1&SIGN_BITMASK):(num2&SIGN_BITMASK);
	real val_abs = (is_diffsign?(is_1gt2?(num1-num2):(num2-num1)):(num1+num2)) & (~SIGN_BITMASK);
    if (val_abs == 0) return 0;
    else return val_abs | sign; 
}

real sub_reals(real num1, real num2) {
    return add_reals (num1 ^ SIGN_BITMASK, num2);
}

real add_int2real(int num1_i, real num2) {
    return add_reals(int2real(num1_i), num2);
}

real sub_int2real(int num1, real num2) {
	return add_int2real(-num1, num2);
}

real mult_reals(real num1, real num2) {
    real sign = (num1 ^ num2) & SIGN_BITMASK;
	real val_abs = (((int64_t)(num1&(~SIGN_BITMASK)))*(num2&(~SIGN_BITMASK))/EXP);
    if (val_abs == 0) return 0;
    else return val_abs | sign; 
}

real div_reals(real num1, real num2) {
    if (num2 == 0) return 0;
    real sign = (num1 ^ num2) & SIGN_BITMASK;
    real val_abs = (((int64_t)(num1&(~SIGN_BITMASK)))*EXP/(num2&(~SIGN_BITMASK)));
    if (val_abs == 0) return 0;
    else return val_abs | sign; 
}

#ifdef DEBUG
void print_real(real num) {
	bool sign = num & SIGN_BITMASK;
	printf(
		"%c%d.%d",
		sign?'-':'+',
		(UPPER_BITMASK & num)/EXP,
		((LOWER_BITMASK & num) * 10000)/EXP
	);
}
#endif DEBUG

#endif	/* FIXEDPOINT_H */

