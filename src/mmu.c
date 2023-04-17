#include "rvemu.h"

/**
 * @brief Load program header
 *
 * @param phdr pointer to program header
 * @param ehdr pointer to elf header
 * @param i    i-th program header
 * @param file file
 */
static void load_phdr(elf64_phdr_t *phdr, elf64_ehdr_t *ehdr, int i, FILE *file) {
    // Advance the file pointer to the start of the i-th program header
    if (fseek(file, ehdr->e_phoff + ehdr->e_phentsize * i, SEEK_SET) != 0) {
        fatal("fseek failed");
    }

    // Load the program header to phdr
    if (fread((void *) phdr, 1, sizeof(elf64_phdr_t), file) != sizeof(elf64_phdr_t)) {
        fatal("file too small");
    }
}

/**
 * @brief Convert the program header to flag to prot in mmap function
 *
 * @param flags program header flags
 * @return int
 */
static int flags_to_mmap_prot(u32 flags) {
    int prot = 0;
    if (flags & PF_X) prot |= PROT_EXEC;
    if (flags & PF_R) prot |= PROT_READ;
    if (flags & PF_W) prot |= PROT_WRITE;
    return prot;
}

/**
 * @brief
 *
 * @param mmu
 * @param phdr
 * @param fd
 */
static void mmu_load_segment(mmu_t *mmu, elf64_phdr_t *phdr, int fd) {
    int page_size = getpagesize();
    u64 offset = phdr->p_offset;
    // Here we want to virtual address to be mapped to the host machine
    u64 vaddr = TO_HOST(phdr->p_vaddr);
    // address and offset in mmap function needs to be page aligned
    u64 aligned_vaddr = ROUNDDOWN(vaddr, page_size);
    u64 filesz = phdr->p_filesz + (vaddr - aligned_vaddr);
    u64 memsz = phdr->p_memsz + (vaddr - aligned_vaddr);
    int prot = flags_to_mmap_prot(phdr->p_flags);
    // map the files into memory
    u64 addr = (u64) mmap((void *) aligned_vaddr, filesz, prot, (MAP_PRIVATE | MAP_FIXED),
                          fd, ROUNDDOWN(offset, page_size));
    // make sure that the mapped address is the address we want
    assert(addr == aligned_vaddr);
    // bss is not in filesz but in memsz (the difference between p_memsz and p_filesz is bss)
    // we need to check if we need to map additional memory space for bss if after adding bss
    // the size exceeds the current page size.
    u64 remaining_bss = ROUNDUP(memsz, page_size) - ROUNDUP(filesz, page_size);
    if (remaining_bss > 0) {
        u64 addr = (u64) mmap((void *) (aligned_vaddr + ROUNDUP(offset, page_size)), remaining_bss, prot,
                              (MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED), -1, 0);
        assert(addr == (aligned_vaddr + ROUNDUP(offset, page_size)));
    }
    //
    mmu->host_alloc = MAX(mmu->host_alloc, (aligned_vaddr + ROUNDUP(memsz, page_size)));
    mmu->base = mmu->alloc = TO_GUEST(mmu->host_alloc);
}

/**
 * @brief Load the ELF file to MMU
 *
 * @param mmu pointer to mmu
 * @param fd file descriptor
 */
void mmu_load_elf(mmu_t *mmu, int fd) {

    u8 buf[sizeof(elf64_ehdr_t)];   // allocate memory for elf header
    elf64_phdr_t phdr;

    ///////////////////////////////////////////
    // open the ELF file and load ELF header
    ///////////////////////////////////////////

    // open the file with read only access and read as binary.
    FILE *file = fdopen(fd, "rb");

    // read the elf header and check if the read size is good.
    if (fread(buf, 1, sizeof(elf64_ehdr_t), file) != sizeof(elf64_ehdr_t)) {
        fatal("file too small");
    }

    elf64_ehdr_t *ehdr = (elf64_ehdr_t *) buf;

    // check if the magic number is correct for ELF files.
    if (*(u32 *) (ehdr->e_ident) != *(u32 *) ELFMAG) {
        fatal("Bad ELF file");
    }

    // check if it's a 64 bit format ELF
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        fatal("Only RISCV V 64 ELF file is supported. Wrong class.");
    }

    // check if the ISA is correct (RISC-V)
    if (ehdr->e_machine != ELF_MACHINE_RISCV) {
        printf("%x\n", ehdr->e_machine);
        fatal("Only RISCV V 64 ELF file is supported. Wrong machine");
    }

    mmu->entry = ehdr->e_entry;

    ///////////////////////////////////////////
    // Load the program header
    ///////////////////////////////////////////

    // iterate over all the program header sections
    for (int i = 0; i < ehdr->e_phnum; i++) {
        // load program header from the file
        load_phdr(&phdr, ehdr, i, file);

        // load the segment into mmu
        if (phdr.p_type == PT_LOAD) {
            mmu_load_segment(mmu, &phdr, fd);
        }
    }
}