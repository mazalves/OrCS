//==============================================================================
//
// Copyright (C) 2010, 2011, 2012
// Marco Antonio Zanata Alves
//
// GPPD - Parallel and Distributed Processing Group
// Universidade Federal do Rio Grande do Sul
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
//==============================================================================
/// Define macros to use uint_64 for printf (PRIu64) and scanf (SCNu64)
#define __STDC_FORMAT_MACROS

#include <sys/types.h>
#include <inttypes.h>
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iomanip>

#include <iostream>
#include <fstream>

#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"


/*
  for i in ~/Dropbox/projetoLLVM/run_me.txt; do
    k=awk -F" " '{print $1}' $i
    j=`basename $i`;
    echo $j
    ../../../pin -t ../bin/intel64/branch_profiler.so -o ~/Dropbox/projetoLLVM/profile/$j -t 1 --  ~/Dropbox/projetoLLVM/native/$i >> /dev/null;
  done
*/

/*
while read -r line
do
    i=$line
    k=`echo $i | awk '{print $1}'`
    j=`basename $k`;
    echo $j
    echo "~/Dropbox/SiNUCA/trace_generator/pin -t ~/Dropbox/SiNUCA/trace_generator/extras/pinplay/bin/intel64/branch_profiler.so -o ~/Dropbox/projetoLLVM/profile/$j -t 1 -- ~/Dropbox/projetoLLVM/native/$i >> /dev/null;"
    ~/Dropbox/SiNUCA/trace_generator/pin -t ~/Dropbox/SiNUCA/trace_generator/extras/pinplay/bin/intel64/branch_profiler.so -o ~/Dropbox/projetoLLVM/profile/$j -t 1 -- ~/Dropbox/projetoLLVM/native/$i >> /dev/null;
done < ~/Dropbox/projetoLLVM/run_me.txt

*/

// ~ #define DEBUG
#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf(__VA_ARGS__);
#else
    #define DEBUG_PRINTF(...)
#endif

//==============================================================================
// Global Variables
//==============================================================================

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 56  // 64 byte line size: 64-8


enum instruction_operation_t {
    INSTRUCTION_BRANCH,
    INSTRUCTION_JUMP,
    INSTRUCTION_OTHER
};

// a running count of the instructions
class thread_data_t {
  public:
    UINT64 cond_taken;
    UINT64 cond_not_taken;

    UINT64 jump_taken;
    UINT64 jump_not_taken;

    UINT64 other_inst;

    UINT64 previous_instruction;
    UINT64 previous_address;
    UINT32 previous_size;

    UINT8 pad[PADSIZE];

    thread_data_t(){
        cond_taken = 0;
        cond_not_taken = 0;

        jump_taken = 0;
        jump_not_taken = 0;

        other_inst = 0;
    };

};
thread_data_t* thread_data;

PIN_LOCK lock;                  /// Lock for methods shared among multiple threads
uint32_t max_threads = 0;       /// Max number of threads (if Intel Compiler Add +1)
std::ofstream DynamicTraceFile;

//==============================================================================
// Commandline Switches
//==============================================================================
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "output_trace.out", "specify the trace base file name.");
KNOB<uint32_t> KnobNumberThreads(KNOB_MODE_WRITEONCE, "pintool",
    "t", "1", "specify the total number of threads.");

//==============================================================================
// Print Help Message
//==============================================================================
uint32_t Usage() {
    cerr <<
        "This tool evaluates the number of taken vs. not taken branches.\n"
        "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << "Example of Usage: >$ ../../../pin -t obj-intel64/branch_profiler.so -o ~/trace_mg/mg_S  -t (threads) 9  -- ~/NPB3.3-OMP/bin/mg.S\n";
    cerr << endl;

    return -1;
};

//==============================================================================
VOID docount(uint32_t inst_type, uint64_t inst_address, uint32_t inst_size, THREADID threadid)
{

    DEBUG_PRINTF("Previous: %" PRIu64 " + %" PRIu32 " => Now %" PRIu64 "  ", thread_data[threadid].previous_address, thread_data[threadid].previous_size, inst_address);


    if (thread_data[threadid].previous_instruction == INSTRUCTION_BRANCH) {
        // NOT TAKEN
        if (thread_data[threadid].previous_address + thread_data[threadid].previous_size == inst_address) {
            DEBUG_PRINTF("cond_not_taken()\n");
            thread_data[threadid].cond_not_taken += 1;
        }
        // TAKEN
        else {
            DEBUG_PRINTF("cond_taken()\n");
            thread_data[threadid].cond_taken += 1;
        }
    }
    else if (thread_data[threadid].previous_instruction == INSTRUCTION_JUMP) {
        // NOT TAKEN
        if (thread_data[threadid].previous_address + thread_data[threadid].previous_size == inst_address) {
            DEBUG_PRINTF("jump_not_taken()\n");
            thread_data[threadid].jump_not_taken += 1;
        }
        // TAKEN
        else {
            DEBUG_PRINTF("jump_taken()\n");
            thread_data[threadid].jump_taken += 1;
        }
    }
    else {
        DEBUG_PRINTF("other()\n");
        thread_data[threadid].other_inst += 1;
    }

    thread_data[threadid].previous_instruction = inst_type;
    thread_data[threadid].previous_address = inst_address;
    thread_data[threadid].previous_size = inst_size;
}

//==============================================================================
// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID trace_instruction(TRACE trace, VOID *v) {
    DEBUG_PRINTF("trace_instruction()\n");

    instruction_operation_t instruction;

    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

            instruction = INSTRUCTION_OTHER;

            if (INS_IsBranchOrCall(ins)) {
                instruction = INSTRUCTION_JUMP;
                if (INS_HasFallThrough(ins)) {
                    instruction = INSTRUCTION_BRANCH;
                }
            }

            BBL_InsertCall(bbl, IPOINT_BEFORE, AFUNPTR(docount), IARG_UINT32, (uint32_t)instruction, IARG_ADDRINT, INS_Address(ins), IARG_UINT32, (uint32_t)INS_Size(ins),IARG_THREAD_ID, IARG_END);
        }
    }
}


//==============================================================================
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
    DEBUG_PRINTF("ThreadStart()\n");
    PIN_GetLock(&lock, threadid);
        ASSERTX(threadid < max_threads);
    PIN_ReleaseLock(&lock);
};

//==============================================================================
VOID Fini(INT32 code, VOID *v) {
    DEBUG_PRINTF("Fini()\n");

    //char trace_header[5000];
    std::stringstream trace_header;
    for (uint32_t i = 0; i < max_threads; i++) {
        uint64_t total_inst = 0;
        total_inst += thread_data[i].cond_taken;
        total_inst += thread_data[i].cond_not_taken;
        total_inst += thread_data[i].jump_taken;
        total_inst += thread_data[i].jump_not_taken;
        total_inst += thread_data[i].other_inst;

        trace_header.str(std::string());
        trace_header << "============================================\n"
                     << " Thread:" << i << "\n"
                     << " BRANCH_TAKEN:" << thread_data[i].cond_taken << " " << std::setprecision(4) << ((thread_data[i].cond_taken/((double)thread_data[i].cond_taken+thread_data[i].cond_not_taken))*100) << "%\n"
                     << " BRANCH_NOT_TAKEN:" << thread_data[i].cond_not_taken << " " << std::setprecision(4) << ((thread_data[i].cond_not_taken/((double)thread_data[i].cond_taken+thread_data[i].cond_not_taken))*100) << "%\n"
                     
                     << " JUMP_TAKEN:" << thread_data[i].jump_taken << " " << std::setprecision(4) << ((thread_data[i].jump_taken/((double)thread_data[i].jump_taken+thread_data[i].jump_not_taken))*100) << "%\n"
                     << " JUMP_NOT_TAKEN:" << thread_data[i].jump_not_taken << " " << ((thread_data[i].jump_not_taken/((double)thread_data[i].jump_taken+thread_data[i].jump_not_taken))*100) << "%\n"
                     << " OTHER_INST:" << thread_data[i].other_inst << "\n"
                     << " TOTAL:" << total_inst << "\n";
        /*
        sprintf(trace_header, "============================================\n");
        sprintf(trace_header, "%s Thread:%d\n", trace_header, i);

        sprintf(trace_header, "%s BRANCH_TAKEN:%" PRIu64 " %0.4f%%\n",trace_header, thread_data[i].cond_taken, (thread_data[i].cond_taken/((double)thread_data[i].cond_taken+thread_data[i].cond_not_taken))*100);
        sprintf(trace_header, "%s BRANCH_NOT_TAKEN:%" PRIu64 " %0.4f%%\n",trace_header, thread_data[i].cond_not_taken, (thread_data[i].cond_not_taken/((double)thread_data[i].cond_taken+thread_data[i].cond_not_taken))*100);

        sprintf(trace_header, "%s JUMP_TAKEN:%" PRIu64 " %0.4f%%\n",trace_header, thread_data[i].jump_taken, (thread_data[i].jump_taken/((double)thread_data[i].jump_taken+thread_data[i].jump_not_taken))*100);
        sprintf(trace_header, "%s JUMP_NOT_TAKEN:%" PRIu64 " %0.4f%%\n",trace_header, thread_data[i].jump_not_taken, (thread_data[i].jump_not_taken/((double)thread_data[i].jump_taken+thread_data[i].jump_not_taken))*100);

        sprintf(trace_header, "%s OTHER_INST:%" PRIu64 "\n",trace_header, thread_data[i].other_inst);
        sprintf(trace_header, "%s TOTAL:%" PRIu64 "\n",trace_header, total_inst);
        */
        std::string tempStr = trace_header.str();
        DynamicTraceFile.write(tempStr.c_str(), strlen(tempStr.c_str()));
    }
    DynamicTraceFile.close();
};

//==============================================================================
// Main
//==============================================================================
int main(int argc, char *argv[]) {
    std::stringstream trace_header;
    trace_header    << "#\n"
                    << "# Compressed Profile Generated By Pin to SiNUCA\n"
                    << "#\n";
    std::string tempStr = trace_header.str();
    
    
    /*
    char trace_header[300];
    sprintf(trace_header, "#\n");
    sprintf(trace_header, "%s# Compressed Profile Generated By Pin to SiNUCA\n", trace_header);
    sprintf(trace_header, "%s#\n", trace_header);
    */
    if (PIN_Init(argc, argv)) {
        return Usage();
    }
    PIN_InitSymbols();

    max_threads = KnobNumberThreads.Value();
    DEBUG_PRINTF("Threads = %d\n", max_threads);

    /// Initialize the pin lock
    PIN_InitLock(&lock);
    thread_data = new thread_data_t[max_threads];

    //==========================================================================
    // Dynamic Trace Files
    //==========================================================================
    static char dyn_file_name[500];
    dyn_file_name[0] = '\0';
    sprintf(dyn_file_name, "%s.out", KnobOutputFile.Value().c_str());
    DynamicTraceFile.open(dyn_file_name);
    DynamicTraceFile.write(tempStr.c_str(), strlen(tempStr.c_str()));
    DEBUG_PRINTF("Real Dynamic File = %s => READY !\n", dyn_file_name);


    //=======================================================================
    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Static and Dynamic Trace
    TRACE_AddInstrumentFunction(trace_instruction, 0);

    // Close the Files
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return 0;
};

//==============================================================================
// eof
//==============================================================================
