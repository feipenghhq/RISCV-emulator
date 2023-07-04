#include "rvemu.h"

int main (int argc, char **argv) {
    // check if arguments are valid.
    if (argc < 2) {
        fatal("No input files");
    }

    machine_t machine = {0};
    machine_load_program(&machine, argv[1]);
    machine_setup(&machine, argc, argv);

    while(true) {
        enum exit_reason_t reason = machine_step(&machine);
        assert(reason == ecall);

        u64 syscall = machine_get_gp_reg(&machine, a7);
        u64 ret = do_syscall(&machine, syscall);
        machine_set_gp_reg(&machine, a0, ret);
        machine.state.exit_reason = none; // reset the exit_reason
    }

    return 0;
}