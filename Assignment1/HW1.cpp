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
 * helper functions used across the tool
 */

void InsCount() {
    icount++;
}

ADDRINT FastForwardCheck(void) {
    return (icount >= fast_forward_count && icount < fast_forward_count + total_count);
}

void PredicatedAnalysisRoutine(UINT32 num_loads, UINT32 num_stores, UINT32 ins_type) {
    // Part A and B
    ins_type_count[LOADS] += num_loads;
    ins_type_count[STORES] += num_stores;
    ins_type_count[ins_type] ++;
}

ADDRINT TerminateCheck(void) {
    return (icount >= fast_forward_count + total_count);
}

/*
 * For Part A and B of the assignment
 * Counts the number of memory read and memory write in an instruction
 * Also classifies it among type A instructions
 *
 */
UINT32 DecodeInsInfo(INS ins, UINT32& num_loads, UINT32& num_stores) {
    num_loads = 0, num_stores = 0;

    // Iterate over each memory operand of the instruction to get number
    // of loads and stores
    UINT32 memOperands = INS_MemoryOperandCount(ins);
    for (UINT32 memOp = 0; memOp < memOperands; memOp++) {

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
 * For printing stats found in part A and B of the assignment
 */
void PrintStatsAB(void) {
    OutFile << "Instruction type data: \n";

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
}

void ExitHandler(void) {
    // OutFile << fixed << setprecision(6);
    OutFile << "The number of instructions executed: " << icount - fast_forward_count << endl;
    PrintStatsAB();
    exit(0);
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v) {
    // If terminate or not
    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) TerminateCheck, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) ExitHandler, IARG_END);

    // get instruction type, number of reads and writes from/to Memory
    UINT32 num_loads, num_stores;
    UINT32 ins_type = DecodeInsInfo(ins, num_loads, num_stores);

    // Instrument the instruction with a check and analysis calls
    INS_InsertIfPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForwardCheck,
            IARG_END);
    INS_InsertThenPredicatedCall(
            ins, IPOINT_BEFORE, (AFUNPTR) PredicatedAnalysisRoutine,
            IARG_UINT32, num_loads,
            IARG_UINT32, num_stores,
            IARG_UINT32, ins_type,
            IARG_END);

    // Insert a call to InsCount before every instruction
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);
}

/* ===================================================================== */
/* Knob Settings                                                         */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "", "specify output file name");

KNOB<UINT64> KnobFastForwardCount(KNOB_MODE_WRITEONCE, "pintool",
    "f", "0", "no of instructions to fast forward before starting the benchmarking");

KNOB<UINT64> KnobInsCount(KNOB_MODE_WRITEONCE, "pintool",
    "c", "0", "no of instructions to run during the benchmarking");

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
