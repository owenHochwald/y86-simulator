#include "tester.h"
#include <stdint.h>
#include <stdio.h>
/*
 * How to approach this assignment.
 *
 * You need not maintain the O flag! As pointed out in class, there
 * is no y86 instruction whose behaviour can change based on its
 * value.
 *
 *
 * 6. Now, start adding instructions incrementally.
 *	Follow the structure described on the main page for this assignment.
 *	The tests we give you in main.c also follow this structure.  Test
 * 	each category before moving on to the next.  Think about error cases!
 *	Every time you think of a kind of error you need to check for,
 *encapsulate that check in a small function that you can easily test; then use
 *it every time you need to make that test. Be careful: on an error, you must
 *not have changed any state.
 *
 *	A note from Margo: I have been programming a long time. I redid this
 *	entire assignment after having already done it once earlier in the
 *	week; I had at least one bug to fix after every new thing I added.
 *	However, by testing each function and each instruction or each
 *	instruction class, the bugs were relatively easy to fix. Had I
 *	tried to do everything at the end, it would have taken two to
 *	three times longer (at least). I also used the main.c we give you
 *	to test entire sets before ever running the autograder.
 *	I also found that by building up lots of error functions, by the
 *	time I got to the last several classes, it was easy to assemble
 *	implementations for them.
 *
 *	While we provided a bunch of test cases, you are free to implement
 *	your own -- just copy a test case and edit it to do what you want!
 *	When you do run the autograder, if you fail a test, you will get
 *	output describing the test case. You can cut and paste the instructions
 *	and state directly into appropriate files in a new test case directory.
 *
 *	If you want to create your own tests, read the file tester.md.
 *
 * 7. When possible, identify helper functions you can write, e.g.,
 *	- Is there error checking that might be used by many instructions?
 *	- Can you think of functionality that might be shared across all
 *	  ALU ops?
 *	- What do the conditional jump and conditional move instructions
 *	  have in common?
 *
 * 8. Finally, use the main program we give you to debug -- you can call
 *functions from inside gdb and you will find this extraordinarily helpful,
 *e.g., call (void)dump_state(state)
 */

int is_memory_equal(y86_state_t *s1, y86_state_t *s2) {
  if (s1->start_addr != s2->start_addr)
    return 0;
  if (s1->valid_mem != s2->valid_mem)
    return 0;

  // at this point we know s1->valid_mem == s2->valid_mem
  for (int i = 0; i < s1->valid_mem; i++) {
    // add the offset of the start
    uint8_t s1_curr = s1->memory[i];
    uint8_t s2_curr = s2->memory[i];

    // compare
    if (s1_curr != s2_curr) {
      return 0;
    }
  }
  return 1;
}

int is_registers_equal(y86_state_t *s1, y86_state_t *s2) {
  int n_entries = sizeof(s1->registers) / sizeof(s1->registers[0]);
  // -1 since we only have 15 valid registers for y86
  for (int i = 0; i < n_entries - 1; i++) {
    uint64_t reg1 = s1->registers[i];
    uint64_t reg2 = s2->registers[i];

    if (reg1 != reg2) {
      return 0;
    }
  }
  return 1;
}

/*
 * is_equal compares two y86 machine states for equivalence.
 * It returns 1 if s1 and s2 are equivalent.
 *
 * Unusual conditions:
 * The memory states only need to match on valid bytes in memory.
 * The register states only need to match on valid y86 registers.
 * The flag bits only need to match on the specific flags supported
 * by the y86.
 */
int is_equal(y86_state_t *s1, y86_state_t *s2) {
  if (!is_memory_equal(s1, s2) || !is_registers_equal(s1, s2)) {
    return 0;
  }

  if (s1->pc != s2->pc || s1->flags != s2->flags) {
    return 0;
  }

  return 1;
}

/*
 * read_quad reads the 8-byte value at 'address' from the memory of
 * the machine state 'state' and stores the result in 'value.'
 * It returns 1 if a read is successful and 0 if it fails.
 */
int read_quad(y86_state_t *state, uint64_t address, uint64_t *value) {
  uint64_t mem_limit;

  mem_limit = state->start_addr + state->valid_mem;

  // check if the address range [address, address+7] is valid
  if (address < state->start_addr || address + 8 > mem_limit)
    return 0;

  // find the start offset
  uint64_t offset = address - state->start_addr;

  uint64_t result = 0;
  for (int i = 0; i < 8; i++) {
    uint64_t byte = state->memory[offset + i];
    // shift the byte one full byte over
    byte = byte << (i * 8);
    // change the result bits at that spot
    result |= byte;
  }

  *value = result;
  return 1;
}

/*
 * write_quad writes the 8-byte item 'value' to the machine state at memory
 * address 'address'.
 * It returns 1 if a write is successful and 0 if it fails.
 */
int write_quad(y86_state_t *state, uint64_t address, uint64_t value) {
  uint64_t mem_limit;
  uint8_t byte;
  uint64_t offset;
  const uint64_t BIT_MASK = 0x00000000000000ff;

  mem_limit = state->start_addr + state->valid_mem;

  // check if the address range [address, address+7] is valid
  if (address < state->start_addr || address + 8 > mem_limit)
    return 0;

  offset = address - state->start_addr;

  // write 8 bytes in little-endian format
  for (int i = 0; i < 8; i++) {
    byte = value & BIT_MASK;
    state->memory[i + offset] = byte;
    value >>= 8;
  }

  return 1;
}

// ----------- INSTRUCTION HANDLING ---------------

int valid_register(uint64_t reg_num) { return reg_num < 16 && reg_num >= 0; }

/*
 * Increments the state->pc by `amount` bytes
 */
void increment_pc(y86_state_t *state, uint64_t amount) { state->pc += amount; }

int handle_halt(y86_state_t *state) {
  // increment_pc(state, 2);
  return 1;
}

int handle_irmovq(y86_state_t *state, y86_inst_t instruction) {
  // test if dest register is valid
  if (!valid_register(instruction.rB))
    return 1;

  // write valC to R[rB]
  state->registers[instruction.rB] = instruction.constval;
  increment_pc(state, 10);
  return 0;
}

int handle_rrmovq(y86_state_t *state, y86_inst_t instruction) {
  if (!valid_register(instruction.rB) || !valid_register(instruction.rA))
    return 1;

  uint64_t val = state->registers[instruction.rA];
  state->registers[instruction.rB] = val;
  increment_pc(state, 2);
  return 0;
}

/*
 * Executes a single y86 instruction, modifying the state as needed.
 */
int execute_single_instruction(y86_state_t *state, y86_inst_t instruction) {
  inst_t op_code = inst_to_enum(instruction.instruction);

  switch (op_code) {
  case I_HALT:
    return handle_halt(state);
  case I_IRMOVQ:
    return handle_irmovq(state, instruction);
  case I_RRMOVQ:
    return handle_rrmovq(state, instruction);
  default:
    return 1;
  }

  return 0;
}

/*
 * y86_check returns 0 if the y86sim_func properly simulates
 * the n_inst instructions described in the instructions array, and
 * non-zero otherwise. Note that even though the simulator should update
 * the value of the program counter as a real program would, it always
 * executes the n_inst instructions sequentially. That is, instructions
 * such as call, ret, or jXX do not change which instruction is to be
 * executed next.
 *
 * Hint: To validate that 'simfunc' executed properly, you will need
 * to produce the correct end state. That means that you must write
 * your own simulator to produce that correct end state.
 *
 * Unusual conditions:
 * On halt: stop execution, the state should reflect the last executed
 * 	instruction.
 * On an invalid command: stop execution, the state should reflect the last
 * 	executed instruction.
 * On any bad arguments, e.g., invalid register number or access to an
 *	invalid address, divide by 0, stop execution, the state should
 * 	reflect the last executed instruction
 */
int y86_check(y86_state_t *state, y86_inst_t *instructions, int n_inst,
              y86sim_func simfunc) {

  y86_state_t actual_state = *state;
  y86_state_t expected_state = *state;

  simfunc(&actual_state, instructions, n_inst);

  for (int i = 0; i < n_inst; i++) {
    int stop = execute_single_instruction(&expected_state, instructions[i]);

    // check for a stoping condition
    if (stop)
      break;
  }

  // printf("Here is the actual state: \n\n\n");
  // dump_state(&actual_state);

  // printf("Here is the expected state: \n\n\n");
  // dump_state(&expected_state);

  return !is_equal(&actual_state, &expected_state);
}
