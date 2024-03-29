/*BEGIN_LEGAL
Intel Open Source License

Copyright (c) 2002-2017 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
#include <iostream>
#include <fstream>
#include "pin.H"
#include "counters.h"

ofstream OutFile;

/*
 * Increase the number of instructions executed by 1
 */
void InsCount() {
    icount++;
}

/*
 * Check whether the required number of instructions have been fast forwarded
 * Returns 1 in case of sufficient instructions fast forwarded
 * Returns 0 otherwise
 */
ADDRINT FastForwardCheck(void) {
    return (icount >= fast_forward_count && icount < fast_forward_count + total_count);
}

void PredicatedAnalysisRoutine(UINT32 num_loads, UINT32 num_stores,
                               UINT32 ins_type, UINT ins_mem_size,
                               UINT32 num_mem_read_op, UINT32 num_mem_write_op,
                               ADDRDELTA min_ins_displacement,
                               ADDRDELTA max_ins_displacement
                               ) {
    // Part A and B
    ins_type_count[LOADS] += num_loads;
    ins_type_count[STORES] += num_stores;
    ins_type_count[ins_type] ++;

    // Part D
    num_mem_op_count[num_mem_read_op+num_mem_write_op] += 1;
    if (num_mem_read_op+num_mem_write_op > 0){
        num_mem_read_op_count[num_mem_read_op] += 1;
        num_mem_write_op_count[num_mem_write_op] += 1;
    }

    num_mem_ins += (ins_mem_size > 0);
    total_mem_ins_size += ins_mem_size;
    max_mem_bytes_touched = (ins_mem_size > max_mem_bytes_touched)?
                             ins_mem_size : max_mem_bytes_touched;
    min_displacement = (min_displacement < min_ins_displacement)?
                        min_displacement : min_ins_displacement;
    max_displacement = (max_displacement > max_ins_displacement)?
                        max_displacement : max_ins_displacement;
}

void NonPredicatedAnalysisRoutine(UINT32 ins_len, UINT32 num_operands,
                                  UINT32 num_reg_reads, UINT32 num_reg_writes,
                                  INT32 min_ins_immediate,
                                  INT32 max_ins_immediate) {
    // Part D
    ins_len_count[ins_len] += 1;
    num_operand_count[num_operands] += 1;
    num_reg_read_op_count[num_reg_reads] += 1;
    num_reg_write_op_count[num_reg_writes] += 1;
    min_immediate = (min_immediate < min_ins_immediate)?
                        min_immediate : min_ins_immediate;
    max_immediate = (max_immediate > max_ins_immediate)?
                        max_immediate : max_ins_immediate;
}

/*
 * Part C
 *
 * addr_start : The start address for the memory space(byte address)
 * size : The size of the memory space(in bytes)
 * select : A self defined enum type to differentiate between instruction and data footprint
 *
 * Determine the start and end blocks memory space and set those memory locations
 * to true in the memory footprint
 */
void FootprintRoutine(ADDRINT addr_start, UINT32 size, UINT32 selec){
    // cout<<addr_start<<size<<selec;
    UINT32 block_start = addr_start>>5;
    UINT32 block_end = (addr_start+size-1)>>5;
    for(UINT32 i=block_start; i<=block_end; i++){
        footprint[selec][i] = true;
    }
}

/*
 * Check if the required number of instructions have been executed
 * return 1 in case required number of instructions completed
 * return 0 otherwise
 */
ADDRINT TerminateCheck(void) {
    return (icount >= fast_forward_count + total_count);
}

/*
 * For Part A and B of the assignment
 * Counts the number of memory read and memory write in an instruction
 * Also classifies it among type A instructions
 */
UINT32 DecodeInsInfo(INS ins, UINT32& num_loads, UINT32& num_stores) {
    num_loads = 0, num_stores = 0;

    // Iterate over each memory operand of the instruction to get number
    // of loads and stores
    UINT32 num_mem_operands = INS_MemoryOperandCount(ins);
    for (UINT32 memOp = 0; memOp < num_mem_operands; memOp++) {

        // Get number of words to be loaded/stored
        UINT32 num_words = (INS_MemoryOperandSize(ins, memOp) + 3)>>2;
        if (INS_MemoryOperandIsRead(ins, memOp))
            num_loads += num_words;

        if (INS_MemoryOperandIsWritten(ins, memOp))
            num_stores += num_words;
    }

    INT32 ins_category = INS_Category(ins);

    // Check the type of instruction
    if (ins_category == XED_CATEGORY_NOP) {
        return NOPS;
    } else if (ins_category == XED_CATEGORY_CALL) {
        if (INS_IsDirectCall(ins))
            return DIRECT_CALLS;
        else
            return INDIRECT_CALLS;
    } else if (ins_category == XED_CATEGORY_RET) {
        return RETURNS;
    } else if (ins_category == XED_CATEGORY_UNCOND_BR) {
        return UNCONDITIONAL_BRANCHES;
    } else if (ins_category == XED_CATEGORY_COND_BR) {
        return CONDITIONAL_BRANCHES;
    } else if (ins_category == XED_CATEGORY_LOGICAL) {
        return LOGICAL_OPERATIONS;
    } else if ((ins_category == XED_CATEGORY_ROTATE) ||
        (ins_category == XED_CATEGORY_SHIFT)) {
        return ROTATE_AND_SHIFT;
    } else if (ins_category == XED_CATEGORY_FLAGOP) {
        return FLAG_OPERATIONS;
    } else if ((ins_category == XED_CATEGORY_AVX) ||
             (ins_category == XED_CATEGORY_AVX2) ||
             (ins_category == XED_CATEGORY_AVX2GATHER)) {
        return VECTOR_INSTRUCTIONS;
    } else if (ins_category == XED_CATEGORY_CMOV) {
        return CONDITIONAL_MOVES;
    } else if ((ins_category == XED_CATEGORY_MMX) ||
             (ins_category == XED_CATEGORY_SSE)) {
        return MMX_AND_SSE_INSTRUCTIONS;
    } else if (ins_category == XED_CATEGORY_SYSCALL) {
        return SYSTEM_CALLS;
    } else if (ins_category == XED_CATEGORY_X87_ALU) {
        return FLOATING_POINT;
    } else {
        return THE_REST;
    }
}

/*
 * For printing stats found in parts A, B, C, and D of the assignment
 */
void PrintStats(void) {
    OutFile << "Instruction Type Results: \n";
    // OutFile << footprint_data[0] <<"\n";

    // Part A,B
    UINT64 ins_count_sum = 0;
    for(int i = 0; i < NUM_INS_TYPE; ++ i)
        ins_count_sum += ins_type_count[i];

    for(int i = 0; i < NUM_INS_TYPE; ++ i) {
        OutFile << InsTypeLiterals[i] << ": " << ins_type_count[i] <<
           " (" << 1.0D*ins_type_count[i]/ins_count_sum << ")" << '\n';
    }

    UINT64 load_store_count = ins_type_count[LOADS] + ins_type_count[STORES];
    UINT64 total_cycles = (ins_count_sum - load_store_count) +
                            50 * load_store_count;

    OutFile << "CPI: " << 1.0D*total_cycles/
               (icount - fast_forward_count) << '\n';

    // Part D
    OutFile << "Instruction Size Results:" << endl;
    for(int i=0; i<=MAX_INS_SIZE;i++){
        OutFile << i << ": " << ins_len_count[i] << endl;
    }

    OutFile << "Memory Instruction Operand Results:" << endl;
    for(int i=0; i<=MAX_NUM_MEMOPS;i++){
        OutFile << i << ": " << num_mem_op_count[i] << endl;
    }

    OutFile << "Memory Instruction Read Operand Results:" << endl;
    for(int i=0; i<=MAX_NUM_MEMOPS;i++){
        OutFile << i << ": " << num_mem_read_op_count[i] << endl;
    }

    OutFile << "Memory Instruction Write Operand Results:" << endl;
    for(int i=0; i<=MAX_NUM_MEMOPS;i++){
        OutFile << i << ": " << num_mem_write_op_count[i] << endl;
    }

    OutFile << "Instruction Operand Results:" << endl;
    for(int i=0; i<=MAX_NUM_OPERANDS;i++){
        OutFile << i << ": " << num_operand_count[i] << endl;
    }

    OutFile << "Instruction Register Read Operand Results:" << endl;
    for(int i=0; i<=MAX_NUM_OPERANDS;i++){
        OutFile << i << ": " << num_reg_read_op_count[i] << endl;
    }

    OutFile << "Instruction Register Write Operand Results:" << endl;
    for(int i=0; i<=MAX_NUM_OPERANDS;i++){
        OutFile << i << ": " << num_reg_write_op_count[i] << endl;
    }

    // Part C
    int num_ins_blocks = 0, num_data_blocks = 0;
    for(int i=0;i<NUM_MEMORY_BLOCKS;i++){
        if(footprint[INSTRUCTION_FOOTPRINT][i]) num_ins_blocks++;
        if(footprint[DATA_FOOTPRINT][i]) num_data_blocks++;
    }

    OutFile << FootprintTypeLiterals[INSTRUCTION_FOOTPRINT] << ": " << num_ins_blocks<< "\n";
    OutFile << FootprintTypeLiterals[DATA_FOOTPRINT] << ": " << num_data_blocks<< "\n";

    // Part D
    OutFile << "Maximum number of bytes touched by an instruction: " <<
            max_mem_bytes_touched << "\n";
    OutFile << "Average number of bytes touched by an instruction: " <<
            ((float)(total_mem_ins_size))/num_mem_ins << "\n";
    OutFile << "Maximum value of immediate: " << max_immediate << "\n";
    OutFile << "Minimum value of immediate: " << min_immediate << "\n";
    OutFile << "Maximum value of displacement used in memory addressing: " <<
            max_displacement << "\n";
    OutFile << "Minimum value of displacement used in memory addressing: " <<
            min_displacement << "\n";
}

/*
 * Handles the premature exiting of the programs
 * print out the stats gathered till now and exit
 */
void ExitHandler(void) {
    // OutFile << fixed << setprecision(6);
    // OutFile << "The number of instructions executed: " << icount - fast_forward_count << endl;

    PrintStats();
    exit(0);
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v) {
    // If terminate or not
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) TerminateCheck, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) ExitHandler, IARG_END);

    // Part A, B
    // Get instruction type, number of reads and writes from/to Memory
    UINT32 num_loads, num_stores;
    UINT32 ins_type = DecodeInsInfo(ins, num_loads, num_stores);

    // Part C, D
    UINT32 ins_len = INS_Size(ins);
    UINT32 num_mem_operands = INS_MemoryOperandCount(ins);
    UINT32 num_operands = INS_OperandCount(ins);
    UINT32 num_reg_reads = INS_MaxNumRRegs(ins);
    UINT32 num_reg_writes = INS_MaxNumWRegs(ins);
    UINT32 num_mem_read_op = 0;
    UINT32 num_mem_write_op = 0;
    UINT32 ins_mem_size = 0;
    ADDRDELTA min_ins_displacement = INT_MAX, max_ins_displacement = INT_MIN;
    INT32 min_ins_immediate= INT_MAX, max_ins_immediate= INT_MIN;


    // Part C
    // Instrument the instruction for its memory footprint
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForwardCheck, IARG_END);
    INS_InsertThenCall(
            ins, IPOINT_BEFORE, (AFUNPTR) FootprintRoutine,
            IARG_INST_PTR,
            IARG_UINT32, ins_len,
            IARG_UINT32, INSTRUCTION_FOOTPRINT,
            IARG_END);

    // Iterate over each memory operand of the instruction to get its memory footprint
    for (UINT32 memOp = 0; memOp < num_mem_operands; memOp++) {
        UINT32 mem_op_size = INS_MemoryOperandSize(ins, memOp);
        ins_mem_size += mem_op_size;

        // Get footprint for the memory operands
        INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForwardCheck, IARG_END);
        INS_InsertThenCall(
                ins, IPOINT_BEFORE, (AFUNPTR) FootprintRoutine,
                IARG_MEMORYOP_EA, memOp,
                IARG_UINT32, mem_op_size,
                IARG_UINT32, DATA_FOOTPRINT,
                IARG_END);

        // Update the counts of read and write operations
        if (INS_MemoryOperandIsRead(ins, memOp)){
            ++ num_mem_read_op;
        }
        if (INS_MemoryOperandIsWritten(ins, memOp)){
            ++ num_mem_write_op;
        }

        ADDRDELTA displacement = INS_OperandMemoryDisplacement(ins, memOp);
        min_ins_displacement = (displacement < min_ins_displacement) ?
                            displacement : min_ins_displacement;
        max_ins_displacement = (displacement > max_ins_displacement) ?
                            displacement : max_ins_displacement;
    }

    // Iterate over all operands to find max/min immediates
    for (UINT32 op = 0; op < num_operands; op++) {
        if (INS_OperandIsImmediate(ins, op)) {
            INT32 immediate = INS_OperandImmediate(ins, op);
            max_ins_immediate = (immediate > max_ins_immediate)?
                immediate:max_ins_immediate;
            min_ins_immediate = (immediate < min_ins_immediate)?
                immediate:min_ins_immediate;
        }
    }

    INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForwardCheck,
            IARG_END);
    INS_InsertThenPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR) PredicatedAnalysisRoutine,

            // Part A
            IARG_UINT32, num_loads,
            IARG_UINT32, num_stores,
            IARG_UINT32, ins_type,

            // Part D
            IARG_UINT32, ins_mem_size,
            IARG_UINT32, num_mem_read_op,
            IARG_UINT32, num_mem_write_op,
            IARG_ADDRINT, min_ins_displacement,
            IARG_ADDRINT, max_ins_displacement,
            IARG_END);

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForwardCheck, IARG_END);
    INS_InsertThenCall(
            ins, IPOINT_BEFORE, (AFUNPTR) NonPredicatedAnalysisRoutine,

            // Part D
            IARG_UINT32, ins_len,
            IARG_UINT32, num_operands,
            IARG_UINT32, num_reg_reads,
            IARG_UINT32, num_reg_writes,
            IARG_ADDRINT, min_ins_immediate,
            IARG_ADDRINT, max_ins_immediate,
            IARG_END);

    // Insert a call to InsCount before every instruction
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);
}

/* ===================================================================== */
/* Knob Settings                                                         */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "", "Specify output file name");

KNOB<UINT64> KnobFastForwardCount(KNOB_MODE_WRITEONCE, "pintool",
    "f", "0", "No of instructions to fast forward before starting the benchmarking");

KNOB<UINT64> KnobInsCount(KNOB_MODE_WRITEONCE, "pintool",
    "c", "0", "No of instructions to run during the benchmarking");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
    // Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    OutFile << "This should not be called" << endl <<
        "Yet it is printing the instruction count as " << icount << endl;
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool solves Assignment1 of Computer Architecture" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    OutFile.open(KnobOutputFile.Value().c_str());
    total_count = 1e9 * KnobInsCount.Value();
    fast_forward_count = 1e9 * KnobFastForwardCount.Value();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
