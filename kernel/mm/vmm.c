#include "vmm.h"
#include "pmm.h"
#include <string.h>
#include <kprintf.h>
#include <panic.h>

#define IDENTITY_MB 128

static pde_t page_dir[1024] ALIGN(4096);

void vmm_init(void) {
    memset(page_dir, 0, sizeof(page_dir));

    for (u32 i = 0; i < (IDENTITY_MB / 4); i++)
        page_dir[i] = (pde_t)((u32)(i * 0x400000) |
                               PDE_PRESENT | PDE_WRITABLE | PDE_PS);

    u32 cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1u << 4);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));

    __asm__ volatile("mov %0, %%cr3" : : "r"((u32)page_dir));

    u32 cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= (1u << 31);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    kprintf("[VMM] Paging enabled. Identity mapped 0-128 MiB.\n");
}

void vmm_flush_tlb(uptr virt) {
    __asm__ volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void vmm_map(uptr virt, uptr phys, u32 flags) {
    u32 pdi = virt >> 22;
    u32 pti = (virt >> 12) & 0x3FF;

    if (page_dir[pdi] & PDE_PS)
        kpanic("vmm_map: cannot map within PSE region (virt=%x)", virt);

    pte_t *pt;
    if (!(page_dir[pdi] & PDE_PRESENT)) {
        uptr pa = pmm_alloc();
        if (!pa) kpanic("vmm_map: out of physical memory");
        pt = (pte_t *)pa;
        memset(pt, 0, PAGE_SIZE);
        page_dir[pdi] = (pde_t)((u32)pa | PDE_PRESENT | PDE_WRITABLE);
    } else {
        pt = (pte_t *)(page_dir[pdi] & ~0xFFF);
    }

    pt[pti] = (pte_t)((phys & ~0xFFF) | (flags & 0xFFF) | PTE_PRESENT);
    vmm_flush_tlb(virt);
}

void vmm_unmap(uptr virt) {
    u32 pdi = virt >> 22;
    u32 pti = (virt >> 12) & 0x3FF;
    if (!(page_dir[pdi] & PDE_PRESENT) || (page_dir[pdi] & PDE_PS)) return;
    pte_t *pt = (pte_t *)(page_dir[pdi] & ~0xFFF);
    pt[pti] = 0;
    vmm_flush_tlb(virt);
}

uptr vmm_phys(uptr virt) {
    u32 pdi = virt >> 22;
    u32 pti = (virt >> 12) & 0x3FF;
    if (!(page_dir[pdi] & PDE_PRESENT)) return 0;
    if (page_dir[pdi] & PDE_PS)
        return (uptr)(page_dir[pdi] & 0xFFC00000) | (virt & 0x3FFFFF);
    pte_t *pt = (pte_t *)(page_dir[pdi] & ~0xFFF);
    if (!(pt[pti] & PTE_PRESENT)) return 0;
    return (uptr)(pt[pti] & ~0xFFF) | (virt & 0xFFF);
}
