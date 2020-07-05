#ifndef __CPU_H__
#define __CPU_H__

typedef struct armreg
{
	reg_t r0;
	reg_t r1;
	reg_t r2;
	reg_t r3;
	reg_t r4;
	reg_t r5;
	reg_t r6;
	reg_t r7;
	reg_t r8;
	reg_t r9;
	reg_t r10;
	reg_t r11;
	reg_t r12;
	reg_t sp;
	reg_t lr;
}armreg_t;

#endif