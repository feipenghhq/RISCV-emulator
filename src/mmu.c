#include "rvemu.h"

void mmu_load_elf(mmu_t *mmu, int fd) {
    // allocate memory for elf header
    u8 buf[sizeof(elf64_ehdr_t)];

    // open the file with read only access and read as binary.
    FILE *file = fdopen(fd, "rb");

    // read the elf header and check if the read size is good.
    if (fread(buf, 1, sizeof(elf64_ehdr_t), file) != sizeof(elf64_ehdr_t)) {
        fatal("file too small");
    }

    elf64_ehdr_t *elf64_ehdr = (elf64_ehdr_t *) buf;

    // check if the magic number is correct for ELF files.
    if (*(u32 *) (elf64_ehdr->e_ident) != *(u32 *) ELFMAG) {
        fatal("bad elf file");
    }

    // check if it's a 64 bit format ELF
    if (elf64_ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        fatal("Only RISCV V 64 elf file is supported. Wrong class.");
    }

    // check if the ISA is correct (RISC-V)
    if (elf64_ehdr->e_machine != ELF_MACHINE_RISCV) {
        printf("%x\n", elf64_ehdr->e_machine);
        fatal("Only RISCV V 64 elf file is supported. Wrong machine");
    }

    mmu->entry = elf64_ehdr->e_entry;
}