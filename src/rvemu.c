#include "rvemu.h"

int main (int argc, char **argv) {
    // check if arguments are valid.
    if (argc < 2) {
        fatal("No input files");
    }

    machine_t machine;
    machine_load_program(&machine, argv[1]);

    printf("entry: %lx\n", machine.mmu.entry);
    return 0;
}