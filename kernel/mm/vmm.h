#pragma once
#include <types.h>

#define PDE_PRESENT  0x01
#define PDE_WRITABLE 0x02
#define PDE_USER     0x04
#define PDE_PS       0x80

#define PTE_PRESENT  0x01
#define PTE_WRITABLE 0x02
#define PTE_USER     0x04

typedef u32 pde_t;
typedef u32 pte_t;

void  vmm_init(void);
void  vmm_map(uptr virt, uptr phys, u32 flags);
void  vmm_unmap(uptr virt);
uptr  vmm_phys(uptr virt);
void  vmm_flush_tlb(uptr virt);
