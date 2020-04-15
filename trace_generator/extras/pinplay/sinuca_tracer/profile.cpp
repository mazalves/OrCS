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
#include <inttypes.h>
#include <string>
#include <stdio.h>
#include "tracer_log_procedures.hpp"
#include "../../../../defines.hpp"
#include "../../../../utils/enumerations.hpp"
#include "../../../../main_memory/memory_request_client.cpp"
#include "opcodes.hpp"

#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"

#ifdef TRACE_GENERATOR_DEBUG
    #define TRACE_GENERATOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define TRACE_GENERATOR_DEBUG_PRINTF(...)
#endif

//==============================================================================
// Global Variables
//==============================================================================

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 56  // 64 byte line size: 64-8

// a running count of the instructions
class thread_data_t {
  public:
    UINT64 counter;
    PIN_LOCK dyn_lock;
    UINT8 pad[PADSIZE];
};
thread_data_t* thread_data;

PIN_LOCK lock;                  /// Lock for methods shared among multiple threads

uint32_t max_threads = 0;       /// Max number of threads (if intel Add +1)
uint64_t total = 0;       /// Max number of threads (if intel Add +1)

std::ofstream DynamicTraceFile;

//==============================================================================
// Commandline Switches
//==============================================================================
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "o", "output_profile.out", "specify the trace base file name.");
KNOB<uint32_t> KnobNumberThreads(KNOB_MODE_WRITEONCE, "pintool",
    "t", "1", "specify the total number of threads.");

//==============================================================================
// Sinuca
//==============================================================================
const char* get_label() {
    return "Trace Generator";
};


//==============================================================================
// Print Help Message
//==============================================================================
uint32_t Usage() {
    cerr <<
        "This tool produces a compressed (dynamic) instruction trace.\n"
        "The trace is still in textual form but repeated sequences\n"
        "of the same code are abbreviated with a number which dramatically\n"
        "reduces the output size and the overhead of the tool.\n"
        "\n";
    cerr << KNOB_BASE::StringKnobSummary();
    cerr << "Example of Usage: >$ ../../../pin -t obj-intel64/trace.so -o ~/trace_mg/mg_S -c (compress) 1 -t (threads) 9 -i (intel) 1 -- ~/NPB3.3-OMP/bin/mg.S\n";
    cerr << endl;

    return -1;
};


//==============================================================================
// This function is called before every block
// Use the fast linkage for calls
VOID PIN_FAST_ANALYSIS_CALL docount(ADDRINT size, THREADID threadid) {
    PIN_GetLock(&thread_data[threadid].dyn_lock, threadid);
        thread_data[threadid].counter += size;
    PIN_ReleaseLock(&thread_data[threadid].dyn_lock);
};


//==============================================================================
// Pin calls this function every time a new basic block is encountered
// It inserts a call to docount
VOID trace_instruction(TRACE trace, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("trace_instruction()\n");

    // Visit every basic block  in the trace
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Insert a call to docount before every bbl, passing the number of instructions
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT32, BBL_NumIns(bbl), IARG_THREAD_ID, IARG_END);
    }
}


//==============================================================================
VOID DynamicOMP_char(char *sync_str, THREADID threadid) {
    if (threadid != 0) return;

    TRACE_GENERATOR_DEBUG_PRINTF("==>(%d) %s", threadid, sync_str);

    //char vector[1024];
    //vector[0] = '\0';
    std::stringstream vector;
    uint64_t sum = 0;

    for (uint32_t j = 0; j < max_threads; j++) {
        PIN_GetLock(&thread_data[j].dyn_lock, j);
            vector << " " << thread_data[j].counter;
            //sprintf(vector, "%s %" PRIu64 "", vector, thread_data[j].counter);
            sum += thread_data[j].counter;
            thread_data[j].counter = 0;
        PIN_ReleaseLock(&thread_data[j].dyn_lock);
    }
    char buffer[1024];
    buffer[0] = '\0';

    total += sum;
    std::string tempStr = vector.str();
    sprintf(buffer, "%s %s %" PRIu64 " %" PRIu64 "\n", sync_str, tempStr.c_str(), sum, total);
    // ~ printf("%s\n", buffer);

    DynamicTraceFile.write(buffer, strlen(buffer));
};

//==============================================================================
// Instrumentation Routines
//==============================================================================
// This routine is executed for each image.
VOID ImageLoad(IMG img, VOID *) {
    TRACE_GENERATOR_DEBUG_PRINTF("ImageLoad()\n");
    /// Only the thread master runs these calls
    std::vector<const char*> OMP_barrier_master;
        OMP_barrier_master.push_back("GOMP_parallel_start");
        OMP_barrier_master.push_back("GOMP_parallel_end");
        OMP_barrier_master.push_back("GOMP_parallel_loop_dynamic_start");
/*
    /// All the threads runs these calls
    std::vector<const char*> OMP_barrier_simple;
        OMP_barrier_simple.push_back(("GOMP_barrier"));
        OMP_barrier_simple.push_back(("GOMP_loop_dynamic_start"));
        OMP_barrier_simple.push_back(("GOMP_loop_ordered_static_start"));
        OMP_barrier_simple.push_back(("GOMP_loop_guided_start"));
        OMP_barrier_simple.push_back(("GOMP_loop_end"));
        OMP_barrier_simple.push_back(("GOMP_parallel_sections_start"));
        OMP_barrier_simple.push_back(("GOMP_sections_end"));

    /// Enters in a critical section
    std::vector<const char*> OMP_critical_start;
        OMP_critical_start.push_back(("GOMP_atomic_start"));
        OMP_critical_start.push_back(("GOMP_critical_start"));
        OMP_critical_start.push_back(("GOMP_critical_name_start"));

    /// Exits in a critical section
    std::vector<const char*> OMP_critical_end;
        OMP_critical_end.push_back(("GOMP_atomic_end"));
        OMP_critical_end.push_back(("GOMP_critical_end"));
        OMP_critical_end.push_back(("GOMP_critical_name_end"));

    /// No Sync or very fast sync that will be ignored
    std::vector<const char*> OMP_ignore;
        /// No Wait
        OMP_ignore.push_back(("GOMP_loop_end_nowait"));
        OMP_ignore.push_back(("GOMP_sections_end_nowait"));
        /// Check if it is running alone
        OMP_ignore.push_back(("GOMP_single_start"));
        /// Reduction
        OMP_ignore.push_back(("GOMP_ordered_start"));
        OMP_ignore.push_back(("GOMP_ordered_end"));
        /// Lock to get the next chunck of iterations
        OMP_ignore.push_back(("GOMP_loop_dynamic_next"));
        OMP_ignore.push_back(("GOMP_loop_ordered_static_next"));
        OMP_ignore.push_back(("GOMP_loop_guided_next"));
        OMP_ignore.push_back(("GOMP_sections_next"));
        /// "loop_next" should be implemented with instrument BEFORE and AFTER
        /// Pin’s implementation of RTN_InsertCall() with IPOINT_AFTER is not perfect.
        /// Pin statically scans the instruction in the function looking for RET.
        /// However, weird code due to compiler optimizations or hand-coded assembly can confuse Pin, and prevent it from finding all the RET’s.
        /// Note that RTN_InsertCall() with IPOINT_BEFORE does not have this problem.
*/
    bool found_GOMP;
    std::string rtn_name;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // RTN_InsertCall() and INS_InsertCall() are executed in order of
        // appearance.  The IPOINT_AFTER may be executed before the IPOINT_BEFORE.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            found_GOMP = false;
            rtn_name = RTN_Name(rtn);

            if (rtn_name.compare(0, 4, "GOMP") != 0) continue;

            /// Barrier only on Master, insert on all the traces
            for (uint32_t i = 0; i < OMP_barrier_master.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_barrier_master[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[100];
                    sprintf(sync_str, "%s", rtn_name.c_str());
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }
/*
            /// Barrier on all the threads
            for (uint32_t i = 0; i < OMP_barrier_simple.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_barrier_simple[i]) == 0) {
                    RTN_Open(rtn);

                    char *sync_str = new char[100];
                    sprintf(sync_str, "%s", rtn_name.c_str());
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Enter in a critical section
            for (uint32_t i = 0; i < OMP_critical_start.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_critical_start[i]) == 0) {
                    RTN_Open(rtn);

                    // ~ char *sync_str = new char[100];
                    // ~ sprintf(sync_str, "%s", rtn_name.c_str());
                    // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Exit the critical section
            for (uint32_t i = 0; i < OMP_critical_end.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_critical_end[i]) == 0) {
                    RTN_Open(rtn);

                    // ~ char *sync_str = new char[100];
                    // ~ sprintf(sync_str, "%s", rtn_name.c_str());
                    // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Ignore the primitive OpenMP
            for (uint32_t i = 0; i < OMP_ignore.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_ignore[i]) == 0) {
                    #ifdef TRACE_GENERATOR_DEBUG
                        RTN_Open(rtn);

                        char *sync_str = new char[100];
                        sprintf(sync_str, "#Ignoring GOMP call:\"%s\"\n", rtn_name.c_str());
                        // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                        RTN_Close(rtn);
                    #endif
                    found_GOMP = true;
                    break;
                }
            }

            if (!found_GOMP) {
                /// If a different OpenMP call is found on the binary
                RTN_Open(rtn);

                char *sync_str = new char[100];
                sprintf(sync_str, "#Found different GOMP call:\"%s\"\n", rtn_name.c_str());
                // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                RTN_Close(rtn);
            }
            * */
        }
    }
};


//==============================================================================
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("ThreadStart()\n");
    PIN_GetLock(&lock, threadid);
        ASSERTX(threadid < max_threads);
    PIN_ReleaseLock(&lock);
};


//==============================================================================
VOID Fini(INT32 code, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("Fini()\n");
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
    printf("GCC Threads = %d\n", max_threads);

    /// Initialize the pin lock
    PIN_InitLock(&lock);
    thread_data = new thread_data_t[max_threads];
    for (uint32_t i = 0; i < max_threads; i++) {
        PIN_InitLock(&thread_data[i].dyn_lock);
        thread_data[i].counter = 0;
    }

    //==========================================================================
    // Dynamic Trace Files
    //==========================================================================
    static char dyn_file_name[500];
    dyn_file_name[0] = '\0';
    sprintf(dyn_file_name, "%s.out", KnobOutputFile.Value().c_str());
    DynamicTraceFile.open(dyn_file_name);
    DynamicTraceFile.write(tempStr.c_str(), strlen(tempStr.c_str()));
    printf("Real Dynamic File = %s => READY !\n", dyn_file_name);


    //=======================================================================
    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Static and Dynamic Trace
    TRACE_AddInstrumentFunction(trace_instruction, 0);

    // Close the Files
    PIN_AddFiniFunction(Fini, 0);

    // Never returns
    PIN_StartProgram();

    return EXIT_SUCCESS;
};

//==============================================================================
// eof
//==============================================================================
