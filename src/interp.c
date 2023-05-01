#include "rvemu.h"

typedef void (func_t)(state_t *, inst_t *);

// function pointers array
//static func_t *funcs[] = {};

/**
 * @brief execute the code by interpretation
 *
 * @param state CPU state
 */
void exec_block_interp(state_t *state) {
    static inst_t inst = {0};
    while (true) {
        // Get the instruction pointed by PC.
        // We already mapped the ELF file to the host memory space
        // so we can map the pc address to the host address and then
        // use indirection operator (*) to get the data pointed by the pc
        // in the host memory space which is the instruction pointed by the PC.
        u32 raw_inst = *(u32 *) TO_HOST(state->pc);

        // decode the instruction
        inst_decode(&inst, raw_inst);

        // execute the instruction
        //funcs[inst.type](state, &inst);

        // revert back register zero value.
        state->gp_regs[zero] = 0;

        if (inst.cont) break;

        // advance pc
        state-> pc += inst.rvc ? 2 : 4;
    }
}
