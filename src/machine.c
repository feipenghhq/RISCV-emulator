#include "rvemu.h"

/**
 * @brief Load the program into memory
 * @param m: pointer to a machine
 * @param prog: program to be loaded
 */
void machine_load_program(machine_t *m, char *prog) {
    // open the program and check if it is opened correctly
    int fd = open(prog, O_RDONLY);
    if (fd == -1) {
        fatal(strerror(errno));
    }

    // load ELF information to MMU
    mmu_load_elf(&(m->mmu), fd);
    close(fd);

    // assign the program entry to current PC
    m->state.pc = m->mmu.entry;
}