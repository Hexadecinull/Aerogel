#include "vmm.h"
#include <pmm.h>
#include <string.h>
#include <kprintf.h>

typedef u64 pte_t;

static pte_t pml4[512] ALIGN(4096);

static pte_t *get_or_create(pte_t *table, u32 idx) {
    if (!(table[idx] & PTE_PRESENT)) {
        uptr p = pmm_alloc();
        if (!p) return NULL;
        memset((void *)p, 0, 4096);
        table[idx] = (pte_t)p | PTE_PRESENT | PTE_WRITABLE;
    }
    return (pte_t *)(uptr)(table[idx] & ~0xFFFULL);
}

void vmm_init(void) {
    memset(pml4, 0, sizeof(pml4));

    for (u32 i = 0; i < 4; i++) {
        uptr pdpt_p = pmm_alloc();
        memset((void *)pdpt_p, 0, 4096);
        pml4[i] = (pte_t)pdpt_p | PTE_PRESENT | PTE_WRITABLE;

        pte_t *pdpt = (pte_t *)pdpt_p;
        for (u32 j = 0; j < 512; j++) {
            uptr pd_p = pmm_alloc();
            memset((void *)pd_p, 0, 4096);
            pdpt[j] = (pte_t)pd_p | PTE_PRESENT | PTE_WRITABLE;

            pte_t *pd = (pte_t *)pd_p;
            for (u32 k = 0; k < 512; k++)
                pd[k] = ((u64)i * 512ULL * 512ULL * 0x200000ULL
                         + (u64)j * 512ULL * 0x200000ULL
                         + (u64)k * 0x200000ULL)
                        | PTE_PRESENT | PTE_WRITABLE | PTE_HUGE;
        }
    }

    __asm__ volatile("mov %0, %%cr3" : : "r"((u64)pml4) : "memory");
    kprintf("[VMM] 4-level paging active. Identity 0-8 GiB (2 MiB pages).\n");
}

void vmm_flush_tlb(uptr virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_map(uptr virt, uptr phys, u64 flags) {
    u32  pml4i = (virt >> 39) & 0x1FF;
    u32  pdpti = (virt >> 30) & 0x1FF;
    u32  pdi   = (virt >> 21) & 0x1FF;
    u32  pti   = (virt >> 12) & 0x1FF;

    pte_t *pdpt = get_or_create(pml4, pml4i);
    pte_t *pd   = get_or_create(pdpt, pdpti);
    if (pd[pdi] & PTE_HUGE) return;
    pte_t *pt   = get_or_create(pd, pdi);
    pt[pti] = (pte_t)((phys & ~0xFFFULL) | flags | PTE_PRESENT);
    vmm_flush_tlb(virt);
}

void vmm_unmap(uptr virt) {
    u32 pml4i = (virt >> 39) & 0x1FF;
    u32 pdpti = (virt >> 30) & 0x1FF;
    u32 pdi   = (virt >> 21) & 0x1FF;
    u32 pti   = (virt >> 12) & 0x1FF;

    if (!(pml4[pml4i] & PTE_PRESENT)) return;
    pte_t *pdpt = (pte_t *)(uptr)(pml4[pml4i] & ~0xFFFULL);
    if (!(pdpt[pdpti] & PTE_PRESENT)) return;
    pte_t *pd = (pte_t *)(uptr)(pdpt[pdpti] & ~0xFFFULL);
    if (!(pd[pdi] & PTE_PRESENT) || (pd[pdi] & PTE_HUGE)) return;
    pte_t *pt = (pte_t *)(uptr)(pd[pdi] & ~0xFFFULL);
    pt[pti] = 0;
    vmm_flush_tlb(virt);
}

uptr vmm_phys(uptr virt) {
    u32 pml4i = (virt >> 39) & 0x1FF;
    u32 pdpti = (virt >> 30) & 0x1FF;
    u32 pdi   = (virt >> 21) & 0x1FF;
    u32 pti   = (virt >> 12) & 0x1FF;

    if (!(pml4[pml4i] & PTE_PRESENT)) return 0;
    pte_t *pdpt = (pte_t *)(uptr)(pml4[pml4i] & ~0xFFFULL);
    if (!(pdpt[pdpti] & PTE_PRESENT)) return 0;
    pte_t *pd = (pte_t *)(uptr)(pdpt[pdpti] & ~0xFFFULL);
    if (!(pd[pdi] & PTE_PRESENT)) return 0;
    if (pd[pdi] & PTE_HUGE)
        return (uptr)(pd[pdi] & ~0x1FFFFFULL) | (virt & 0x1FFFFF);
    pte_t *pt = (pte_t *)(uptr)(pd[pdi] & ~0xFFFULL);
    if (!(pt[pti] & PTE_PRESENT)) return 0;
    return (uptr)(pt[pti] & ~0xFFFULL) | (virt & 0xFFF);
}
