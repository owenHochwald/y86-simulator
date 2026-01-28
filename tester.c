#include <stdio.h>
#include "tester.h"

/*
 * How to approach this assignment.
 *
 * You need not maintain the O flag! As pointed out in class, there 
 * is no y86 instruction whose behaviour can change based on its
 * value.
 *
 * 1. First, write each of the helper functions. Write them one
 *      at a time and test them with the corresponding test cases.
 *      You can test them using the main program we give you; this
 *      will be much easier to debug.
 *
 * 2. Design an API that can call to execute a single instruction
 *	from the array of instructions you need to process.  Figure
 *	out what parameters that function should take. Write the
 *	function and have it do nothing; add the right call to it
 *	in your main function. You are now ready to start building
 *	both your simulator and checker.
 *
 * 3. Write the y86_check function. Use the answer to question  Lab3-Ind.8
 *	to help you do this.
 *
 * 4. Start building out your simulator (the function you designed in #2).
 *	Start by just having it return success.
 *	You should find that even this simple simulator passes the first test.
 *	(Once again, use the main program we give you.)
 *
 * 5. Now, start building it out for real!
 * 	Write code to transform the string representation of an instruction
 *	to an enum (hint: look at the utility functions we provide for you;
 *	they are documented in tester.h).
 *
 * 6. Now, start adding instructions incrementally.
 *	Follow the structure described on the main page for this assignment.
 *	The tests we give you in main.c also follow this structure.  Test
 * 	each category before moving on to the next.  Think about error cases!
 *	Every time you think of a kind of error you need to check for, encapsulate
 *	that check in a small function that you can easily test; then use it
 *	every time you need to make that test.
 *	Be careful: on an error, you must not have changed any state.
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
 * 8. Finally, use the main program we give you to debug -- you can call functions
 *	from inside gdb and you will find this extraordinarily helpful, e.g.,
 *	call (void)dump_state(state)
 */

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
int is_equal(y86_state_t *s1, y86_state_t *s2){
	return 0;
}

/*
 * read_quad reads the 8-byte value at 'address' from the memory of
 * the machine state 'state' and stores the result in 'value.'
 * It returns 1 if a read is successful and 0 if it fails.
 */
int read_quad(y86_state_t *state, uint64_t address, uint64_t *value) {
	return 0;
}

/* 
 * write_quad writes the 8-byte item 'value' to the machine state at memory
 * address 'address'.
 * It returns 1 if a write is successful and 0 if it fails.
 */
int write_quad(y86_state_t *state, uint64_t address, uint64_t value) {
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
	return -1;
}
