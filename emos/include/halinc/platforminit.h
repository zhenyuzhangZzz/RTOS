#ifndef __PLATFORMINIT_H__
#define __PLATFORMINIT_H__

LKHEAD_T void platform_init(void);

#ifdef CFG_S5PV210_PLATFORM

LKHEAD_T void s5pv210_mmu_init(void);
LKHEAD_T void s5pv210_set_ttb(uint_t ttb_phy_addr);
LKHEAD_T void s5pv210_set_domain(uint_t domain);
LKHEAD_T void s5pv210_invalid_tlb_cache(void);
LKHEAD_T void s5pv210_enable_cache(void);
LKHEAD_T void s5pv210_enable_mmu(void);
LKHEAD_T void s5pv210_disable_mmu(void);

LKHEAD_T void s5pv210_set_vector_base(uint_t addr);

#endif

#endif