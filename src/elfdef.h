// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format

#ifndef __RVEMU_ELFDEF_H__
#define __RVEMU_ELFDEF_H__

#include "types.h"

// Index to e_ident
#define EI_MAG0         0
#define EI_MAG1         1
#define EI_MAG2         2
#define EI_MAG3         3
#define EI_CLASS        4
#define EI_DATA         5
#define EI_VERSION      6
#define EI_OSABI        7
#define EI_ABIVERSION   8
#define EI_PAD          9

// size of the e_ident
#define EI_SIZE     15

// ELF magic number
#define ELFMAG          "\177ELF"

// ELF ,machine(ISA)
#define ELF_MACHINE_RISCV    0xF3

// ELF class
#define ELFCLASS32  1
#define ELFCLASS64  2

/**
 * @brief File header
 *
 */
typedef struct {
    u8  e_ident[EI_SIZE];
    u16 e_type;             // Identifies object file type.
    u16 e_machine;          // Specifies target instruction set architecture.
    u32 e_version;          // Set to 1 for the original version of ELF.
    u64 e_entry;            // This is the memory address of the entry point from where the process starts executing.
    u64 e_phoff;            // Points to the start of the program header table.
    u64 e_shoff;            // Points to the start of the section header table.
    u32 e_flags;            // Interpretation of this field depends on the target architecture.
    u16 e_ehsize;           // Contains the size of this header, normally 64 Bytes for 64-bit and 52 Bytes for 32-bit format.
    u16 e_phentsize;        // Contains the size of a program header table entry.
    u16 e_phnum;            // Contains the number of entries in the program header table.
    u16 e_shentsize;        // Contains the size of a section header table entry.
    u16 e_shnum;            // Contains the number of entries in the section header table.
    u16 e_shstrndx;         // Contains index of the section header table entry that contains the section names.
} elf64_ehdr_t;

/**
 * @brief Program header
 *
 */
typedef struct {
    u32 p_type;     // Identifies the type of the segment.
    u32 p_flags;    // Segment-dependent flags (position for 64-bit structure).
    u64 p_offset;   // Offset of the segment in the file image.
    u64 p_vaddr;    // Virtual address of the segment in memory.
    u64 p_paddr;    // On systems where physical address is relevant, reserved for segment's physical address.
    u64 p_filesz;   // Size in bytes of the segment in the file image. May be 0.
    u64 p_memsz;    // Size in bytes of the segment in memory. May be 0.
    u64 p_align;    // 0 and 1 specify no alignment. Otherwise should be a positive, integral power of 2,
                    // with p_vaddr equating p_offset modulus p_align.
} elf64_phdr_t;

#endif