#include "rvemu.h"


/**
 * @brief execute multiple instructions till we hit a ecall
 *
 * @param m pointer to machine
 * @return enum exit_reason_t
 */
enum exit_reason_t machine_step(machine_t *m) {
    while(true) {
        exec_block_interp(&m->state);

        // continue execution if it is indirect branch or direct branch
        if (m->state.exit_reason == indirect_branch || m->state.exit_reason == direct_branch) {
            m->state.exit_reason = none;    // reset the exit_reason
            continue;
        }

        // continue execution if it is mret
        if (m->state.exit_reason == mret) {
            m->state.exit_reason = none;    // reset the exit_reason
            continue;
        }

        // break on ecall.
        assert(m->state.exit_reason == ecall);
        break;
    }

    return ecall;
}

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