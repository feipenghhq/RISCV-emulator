#include "rvemu.h"

/**
 * Memory Mapping between host program and guest program
 *
 * host program: the emulator program itself
 * guest program: the program to be executed by the emulator
 *
 * host program is stored in the high address in the machine (e.g. machine address: 0x7ffx_xxxx_xxxx)
 * guest program is stored in the low address in the machine (e.g. entry: 0x10xxx)
 *
 * we add an offset to address of guest program to map the guest program address space
 * into host program memory space
 *
 * mmap related info: https://www.cnblogs.com/huxiao-tee/p/4660352.html
 *
 */


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
 * @brief load a segment in the ELF file to the host memory space
 *
 * @param mmu   a pointer to the mmu
 * @param phdr  a pointer to the program header
 * @param fd    file descriptor
 */
static void mmu_load_segment(mmu_t *mmu, elf64_phdr_t *phdr, int fd) {
    // get the page size of the host program, usually 4096
    int page_size = getpagesize();
    u64 offset = phdr->p_offset;
    // map the virtual address in the segment to the host memory space
    u64 vaddr = TO_HOST(phdr->p_vaddr);
    // address and offset in mmap function needs to be page aligned
    // align the address to the page boundary for mmap function
    u64 aligned_offset = ROUNDDOWN(offset, page_size);
    u64 aligned_vaddr = ROUNDDOWN(vaddr, page_size);
    // because we aligned the vaddr with the page size, the actual file/mem size we are
    // going to load might be larger then the original file size.
    u64 filesz = phdr->p_filesz + (vaddr - aligned_vaddr);
    u64 memsz = phdr->p_memsz + (vaddr - aligned_vaddr);
    int prot = flags_to_mmap_prot(phdr->p_flags);
    // map the segment to host memory
    u64 addr = (u64) mmap((void *) aligned_vaddr, filesz, prot, (MAP_PRIVATE | MAP_FIXED), fd, aligned_offset);
    // make sure that the mapped address is the address we assigned
    assert(addr == aligned_vaddr);
    // bss is not in filesz but in memsz (the difference between p_memsz and p_filesz is bss)
    // we need to check if we need another page to store the bss session.
    // we compare the memsz and filesz after rounding them to the upper page boundary to see if they are
    // the same or not, if they are not the same, then memsz need more page to be stored then filesz.
    u64 remaining_bss = ROUNDUP(memsz, page_size) - ROUNDUP(filesz, page_size);
    // the new address to be mapped here is the aligned_vaddr + the size of the file rounded up to the page boundary
    if (remaining_bss > 0) {
        u64 addr = (u64) mmap((void *) (aligned_vaddr + ROUNDUP(filesz, page_size)), remaining_bss, prot,
                              (MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED), -1, 0);
        assert(addr == (aligned_vaddr + ROUNDUP(filesz, page_size)));
    }
    // memory mapping view in HOST memory space
    // - ELF is the memory space storing the guest program (ELF)
    // - Execution is the memory space used after executing the guest program
    // - host_alloc stores the upper boundary of the malloced memory space in host view
    // [     ELF      |    malloc-ed spaced |   ]
    //                                      ^ host_alloc
    //
    // memory mapping view in GUEST memory space
    // - base is the guest view of host_alloc (host_alloc mapped to guest memory space)
    // - alloc stores the upper boundary of the malloced memory space in guest view
    // [     ELF      | malloc-ed spaced |] > in guest memory space
    //                ^ base             ^ alloc
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

/*
MMU Memory Map for the host program
[ program ][ stack ]|[ heap ]
                    ^ alloc
                    ^ stack bottom

sp -> stack top
stack size: 32M

stack memory map:
[                           | argc argv envp auxv]
                            ^ stack bottom (sp)
         <------ stack growing direction
         ^ stack top
*/

u64 mmu_alloc(mmu_t *mmu, i64 sz) {
    int page_size = getpagesize();
    u64 base = mmu->alloc;
    // mmu->base point to the end of the ELF (program) in the memory.
    // The base (mmu->alloc) should be greater then the base,
    // otherwise the user data (stack) will be overlapping with the program
    assert(base >= mmu->base);

    mmu->alloc += sz;
    assert(mmu->alloc >= mmu->base);

    // mmu->alloc is page aligned so
    // check if we need to apply for a new page.
    if (sz > 0 && mmu->alloc > TO_GUEST(mmu->host_alloc)) {
        if (mmap((void*) mmu->host_alloc, ROUNDUP(sz, page_size),
                 PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0) == MAP_FAILED) {
            fatal("mmap failed.");
        }
        mmu->host_alloc += ROUNDUP(sz, page_size);
    }
    else if (sz < 0 && ROUNDUP(mmu->alloc, page_size) < TO_GUEST(mmu->host_alloc)) {
        u64 len = TO_GUEST(mmu->host_alloc) - ROUNDUP(mmu->alloc, page_size);
        if (munmap((void *) ROUNDUP(mmu->alloc, page_size), len) == -1) {
            fatal(strerror(errno));
        }
        mmu->host_alloc -= len;
    }

    return base;
}
