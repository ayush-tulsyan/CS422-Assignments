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

ofstream OutFile;

// The running count of instructions is kept here
// make it static to help the compiler optimize docount
static UINT64 icount = 0;
static UINT64 total_count = 0;
static UINT64 fast_forward_count = 0;

// Knob Modification/Analysis Functions
void InsCount() {
    icount++;
}

ADDRINT FastForwardCheck(void) {
    return (icount >= fast_forward_count && icount < fast_forward_count + total_count);
}

void AnalysisRoutine(void) {
    return;
}

ADDRINT TerminateCheck(void) {
    return (icount >= fast_forward_count + total_count);
}

void ExitRoutine(void) {
    OutFile << "The number of instructions executed: " << icount - fast_forward_count << endl;
    exit(0);
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
    // Insert a call to docount before every instruction, no arguments are passed
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)InsCount, IARG_END);

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) TerminateCheck, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) ExitRoutine, IARG_END);

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR) FastForwardCheck, IARG_END);
    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR) AnalysisRoutine, IARG_END);
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
    total_count = 1e6 * KnobInsCount.Value();
    fast_forward_count = 1e6 * KnobFastForwardCount.Value();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
