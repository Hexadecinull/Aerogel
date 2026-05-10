#pragma once
#include <types.h>

#define PTE_PRESENT  0x01ULL
#define PTE_WRITABLE 0x02ULL
#define PTE_USER     0x04ULL
#define PTE_HUGE     0x80ULL
#define PTE_NX       (1ULL << 63)

void  vmm_init(void);
void  vmm_map(uptr virt, uptr phys, u64 flags);
void  vmm_unmap(uptr virt);
uptr  vmm_phys(uptr virt);
void  vmm_flush_tlb(uptr virt);
