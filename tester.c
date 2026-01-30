#include "tester.h"
#include <stdint.h>
#include <stdio.h>

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

// Register 0xF means "no register" - it's valid in the instruction encoding
// but shouldn't be read from or written to
int valid_register(uint64_t reg_num) { return reg_num < 15; }

/*
 * Increments the state->pc by `amount` bytes
 */
void increment_pc(y86_state_t *state, uint64_t amount) { state->pc += amount; }

/*
 * Returns if a memory address is within the valid range, 8 byte addresses
 */
int valid_address(y86_state_t *state, uint64_t address) {
  uint64_t mem_limit = state->start_addr + state->valid_mem;

  // check if the address range [address, address+7] is valid
  if (address < state->start_addr || address + 8 > mem_limit)
    return 0;
  return 1;
}

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

int handle_opq(y86_state_t *state, y86_inst_t instruction, inst_t op_code) {
  if (!valid_register(instruction.rB) || !valid_register(instruction.rA))
    return 1;

  // Treat register values as signed for arithmetic operations
  int64_t valA = (int64_t)state->registers[instruction.rA];
  int64_t valB = (int64_t)state->registers[instruction.rB];
  int64_t val;

  switch (op_code) {
  case I_ADDQ:
    val = valB + valA;
    break;
  case I_SUBQ:
    val = valB - valA;
    break;
  case I_MODQ:
    // Check for divide by zero
    if (valA == 0)
      return 1;
    val = valB % valA;
    break;
  case I_MULQ:
    val = valB * valA;
    break;
  case I_DIVQ:
    // Check for divide by zero
    if (valA == 0)
      return 1;
    val = valB / valA;
    break;
  case I_ANDQ:
    val = valB & valA;
    break;
  case I_XORQ:
    val = valB ^ valA;
    break;
  default:
    return 1;
  }

  state->flags = 0;

  // make sure to set condition flags
  if (val == 0)
    state->flags |= FLAG_Z;

  if (val < 0)
    state->flags |= FLAG_S;

  state->registers[instruction.rB] = (uint64_t)val;
  increment_pc(state, 2);
  return 0;
}

/*
 * Conditional move looks to the flags to determine which path to take
 */
int handle_cmovxx(y86_state_t *state, y86_inst_t instruction, inst_t op_code) {
  if (!valid_register(instruction.rB) || !valid_register(instruction.rA))
    return 1;

  int64_t val = -1;
  uint64_t valA = state->registers[instruction.rA];

  switch (op_code) {
  case I_CMOVEQ:
    if (state->flags & FLAG_Z)
      val = valA;
    break;
  case I_CMOVG:
    if (!(state->flags & FLAG_S) && !(state->flags & FLAG_Z))
      val = valA;
    break;
  case I_CMOVGE:
    if ((state->flags & FLAG_Z) || !(state->flags & FLAG_S))
      val = valA;
    break;
  case I_CMOVL:
    if ((state->flags & FLAG_S))
      val = valA;
    break;
  case I_CMOVLE:
    if ((state->flags & FLAG_Z) || (state->flags & FLAG_S))
      val = valA;
    break;
  case I_CMOVNE:
    if (!(state->flags & FLAG_Z))
      val = valA;
    break;
  default:
    return 1;
  }

  if (val != -1) {
    state->registers[instruction.rB] = val;
  }

  increment_pc(state, 2);
  return 0;
}

int handle_jump(y86_state_t *state, y86_inst_t instruction) {
  // Unconditional jump: set PC to destination
  state->pc = instruction.constval;
  return 0;
}

int handle_nop(y86_state_t *state) {
  increment_pc(state, 1);
  return 0;
}

int handle_mrmovq(y86_state_t *state, y86_inst_t instruction) {
  if (!valid_register(instruction.rB) || !valid_register(instruction.rA))
    return 1;

  int64_t displacement = (int64_t)instruction.constval;
  uint64_t valE = state->registers[instruction.rB] + displacement;
  uint64_t valM;
  if (!read_quad(state, valE, &valM))
    return 1;

  state->registers[instruction.rA] = valM;

  increment_pc(state, 10);
  return 0;
}

int handle_rmmovq(y86_state_t *state, y86_inst_t instruction) {
  if (!valid_register(instruction.rB) || !valid_register(instruction.rA))
    return 1;

  uint64_t valA = state->registers[instruction.rA];
  int64_t displacement = (int64_t)instruction.constval;
  uint64_t valE = state->registers[instruction.rB] + displacement;

  if (!write_quad(state, valE, valA))
    return 1;

  increment_pc(state, 10);
  return 0;
}

int handle_pushq(y86_state_t *state, y86_inst_t instruction) {
  if (!valid_register(instruction.rA))
    return 1;

  const int RSP = 4;

  uint64_t valA = state->registers[instruction.rA];

  uint64_t new_rsp = state->registers[RSP] - 8;
  if (!write_quad(state, new_rsp, valA)) {
    return 1;
  }

  state->registers[RSP] = new_rsp;

  increment_pc(state, 2);
  return 0;
}

int handle_popq(y86_state_t *state, y86_inst_t instruction) {
  if (!valid_register(instruction.rA))
    return 1;

  const int RSP = 4;

  uint64_t valM;
  if (!read_quad(state, state->registers[RSP], &valM))
    return 1;

  state->registers[instruction.rA] = valM;

  state->registers[RSP] += 8;

  increment_pc(state, 2);
  return 0;
}

int handle_call(y86_state_t *state, y86_inst_t instruction) {
  const int RSP = 4;

  uint64_t return_addr = state->pc + 9;

  state->registers[RSP] -= 8;

  if (!write_quad(state, state->registers[RSP], return_addr)) {
    state->registers[RSP] += 8;
    return 1;
  }

  state->pc = instruction.constval;

  return 0;
}

int handle_ret(y86_state_t *state) {
  const int RSP = 4;

  uint64_t return_addr;
  if (!read_quad(state, state->registers[RSP], &return_addr))
    return 1;

  state->registers[RSP] += 8;

  state->pc = return_addr;

  return 0;
}

/*
 * Conditional jump checks the flags to determine whether to jump
 */
int handle_jxx(y86_state_t *state, y86_inst_t instruction, inst_t op_code) {
  int should_jump = 0;

  switch (op_code) {
  case I_JEQ:
    if (state->flags & FLAG_Z)
      should_jump = 1;
    break;
  case I_JNE:
    if (!(state->flags & FLAG_Z))
      should_jump = 1;
    break;
  case I_JL:
    if (state->flags & FLAG_S)
      should_jump = 1;
    break;
  case I_JLE:
    if ((state->flags & FLAG_Z) || (state->flags & FLAG_S))
      should_jump = 1;
    break;
  case I_JG:
    if (!(state->flags & FLAG_S) && !(state->flags & FLAG_Z))
      should_jump = 1;
    break;
  case I_JGE:
    if ((state->flags & FLAG_Z) || !(state->flags & FLAG_S))
      should_jump = 1;
    break;
  default:
    return 1;
  }

  if (should_jump) {
    state->pc = instruction.constval;
  } else {
    increment_pc(state, 9);
  }

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
  case I_NOP:
    return handle_nop(state);
  case I_IRMOVQ:
    return handle_irmovq(state, instruction);
  case I_RRMOVQ:
    return handle_rrmovq(state, instruction);
  case I_ADDQ:
  case I_SUBQ:
  case I_MODQ:
  case I_MULQ:
  case I_DIVQ:
  case I_ANDQ:
  case I_XORQ:
    return handle_opq(state, instruction, op_code);
  case I_CMOVEQ:
  case I_CMOVG:
  case I_CMOVGE:
  case I_CMOVL:
  case I_CMOVLE:
  case I_CMOVNE:
    return handle_cmovxx(state, instruction, op_code);
  case I_J:
    return handle_jump(state, instruction);
  case I_JEQ:
  case I_JNE:
  case I_JL:
  case I_JLE:
  case I_JG:
  case I_JGE:
    return handle_jxx(state, instruction, op_code);
  case I_MRMOVQ:
    return handle_mrmovq(state, instruction);
  case I_RMMOVQ:
    return handle_rmmovq(state, instruction);
  case I_PUSHQ:
    return handle_pushq(state, instruction);
  case I_POPQ:
    return handle_popq(state, instruction);
  case I_CALL:
    return handle_call(state, instruction);
  case I_RET:
    return handle_ret(state);
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
