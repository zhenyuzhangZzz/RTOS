u32_t system_call(int nr, int var1, int var2, int var3, int var4, int var5, int var6, int var7)
{
	uint32_t ret;
	__asm__ __volatile__(
		"mov r0 %1\n"
		"mov r1 %2\n"
		"mov r2 %3\n"
		"mov r3 %4\n"
		"mov r4 %5\n"
		"mov r5 %6\n"
		"mov r6 %7\n"
		"mov r7 %8\n"
		"swi 0\n"
		"mov %0, r0\n"
		:"=&r" (ret)
		:"r" (nr), "r" (var1), "r" (var2), "r" (var3), 
		 "r" (var4),"r" (var5), "r" (var6), "r" (var7)
		:"cc", "memory"
		);
	return ret;
}