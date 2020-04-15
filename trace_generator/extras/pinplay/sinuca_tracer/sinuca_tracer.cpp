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
#include <sstream>
#include "tracer_log_procedures.hpp"
#include "../../../../defines.hpp"
#include "../../../../utils/enumerations.hpp"
#include "../../../../main_memory/memory_request_client.cpp"
#include "opcodes.hpp"
#include "conversions.cpp"
// ============================================================================

#undef ERROR // Required to avoid using backtrace
// ============================================================================

#include "pin.H"
#include "instlib.H"
#include "xed-interface.h"
#include "pinplay.H"
PINPLAY_ENGINE pinplay_engine;
// ============================================================================


// #include "intrinsics_extension.cpp"
#include "intrinsics_extension.cpp"
// ~ KNOB<BOOL> KnobLogger(KNOB_MODE_WRITEONCE, "pintool", "log", "0", "Activate the pinplay logger");
// ~ KNOB<BOOL> KnobReplayer(KNOB_MODE_WRITEONCE, "pintool", "replay", "0", "Activate the pinplay replayer");

#define KNOB_LOG_NAME  "log"
#define KNOB_REPLAY_NAME "replay"
#define KNOB_FAMILY "pintool:pinplay-driver"

KNOB_COMMENT pinplay_driver_knob_family(KNOB_FAMILY, "PinPlay Driver Knobs");

KNOB<BOOL>KnobReplayer(KNOB_MODE_WRITEONCE, KNOB_FAMILY,
                       KNOB_REPLAY_NAME, "0", "Replay a pinball");
KNOB<BOOL>KnobLogger(KNOB_MODE_WRITEONCE,  KNOB_FAMILY,
                     KNOB_LOG_NAME, "0", "Create a pinball");



// ============================================================================
/// Enumerates the synchronization type required by the dynamic trace.
enum sync_t {
    SYNC_BARRIER,
    SYNC_WAIT_CRITICAL_START,
    SYNC_CRITICAL_START,
    SYNC_CRITICAL_END,
    SYNC_FREE
};

// #ifdef TRACE_GENERATOR_DEBUG
//     #define TRACE_GENERATOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
// #else
//     #define TRACE_GENERATOR_DEBUG_PRINTF(...)
// #endif

//==============================================================================
// Global Variables
//==============================================================================

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
// #define PADSIZE 64
//
// class thread_data_t {
//     public:
//         PIN_LOCK dyn_lock;
//         bool is_instrumented_bbl;
//
//         gzFile gzDynamicTraceFile;
//         gzFile gzMemoryTraceFile;
//
//         UINT8 pad[PADSIZE];
// };
//
// thread_data_t* thread_data;     /// Lock for methods shared among multiple threads

LOCALVAR CONTROLLER::CONTROL_MANAGER control;

// ~ CONTROLLER::CONTROL control;       /// Control to PinPoints
// PIN_LOCK lock;                  /// Lock for methods shared among multiple threads
// bool is_instrumented = false;   /// Will be enabled by PinPoints
//
// std::ofstream StaticTraceFile;
// gzFile gzStaticTraceFile;
//
// uint32_t count_trace = 0;       /// Current BBL trace number
uint32_t max_threads = 0;       /// Max number of threads (if intel Add +1)

int32_t count_parallel_start = 0;
int32_t count_parallel_end = 0;

//==============================================================================
// Commandline Switches
//==============================================================================
// ~ KNOB_COMMENT pinplay_driver_knob_family("pintool:sinuca_tracer", "SiNUCA Tracer Driver Knobs");

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool",
    "output", "output_trace.out", "specify the trace base file name.");
KNOB<uint32_t> KnobNumberThreads(KNOB_MODE_WRITEONCE, "pintool",
    "threads", "1", "specify the total number of threads.");
KNOB<int32_t> KnobParallelStart(KNOB_MODE_WRITEONCE, "pintool",
    "parallel_start", "-1", "parallel start counter to enable the tracer.");
KNOB<int32_t> KnobParallelEnd(KNOB_MODE_WRITEONCE, "pintool",
    "parallel_end", "-1", "parallel end counter to enable the tracer.");

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
    cerr << "Example of Usage: >$ ../../../pin -t obj-intel64/trace.so -output <output_file> -threads <threads> -parallel_start <parallel_start> -parallel_end <parallel_end>  -- <executable>\n";
    cerr << endl;

    return -1;
};

//==============================================================================
// VOID write_static_char(char *stat_str) {
//     TRACE_GENERATOR_DEBUG_PRINTF("write_static_char()\n");
//
//     PIN_GetLock(&lock, 1);
//         gzwrite(gzStaticTraceFile, stat_str, strlen(stat_str));
//     PIN_ReleaseLock(&lock);
// };

//==============================================================================
VOID control_instrumented_bbl(THREADID threadid){
    TRACE_GENERATOR_DEBUG_PRINTF("write_dynamic_char()\n");

    /// If the pin-points disabled this region
    if (!is_instrumented) {
        ///=================================================================
        /// If (parallel instrumentation)
        bool is_last = true;
        for (uint32_t i = 0 ; i < max_threads; i++) {
            if (i != threadid && thread_data[i].is_instrumented_bbl == true) {
                is_last = false;
                break;
            }
        }

        /// If this is the (last thread active) and (the trace was active) and (this is a parallel trace)
        if (is_last == true &&
            thread_data[threadid].is_instrumented_bbl == true &&
            KnobParallelStart.Value() >= 0 && KnobParallelEnd.Value() >= 0) {

            /// Can stop the memory trace
            thread_data[threadid].is_instrumented_bbl = false;

            /// EXIT the application after trace the slice wanted **********
            PIN_ExitApplication(EXIT_SUCCESS);
        }
        ///=================================================================

        // Can stop the memory trace
        thread_data[threadid].is_instrumented_bbl = false;
        return;
    }
    else {
        // Can start the memory trace
        thread_data[threadid].is_instrumented_bbl = true;
    }
};

//==============================================================================
// VOID write_dynamic_char(char *dyn_str, THREADID threadid) {
//     TRACE_GENERATOR_DEBUG_PRINTF("write_dynamic_char()\n");
//
//     /// If the pin-points disabled this region
//     if (!is_instrumented) {
//         return;
//     }
//     else {
//
//         // This lock is necessary because when using a parallel program
//         // the thread master may write on multiple threads
//         // ex: omp_parallel_start / omp_parallel_end
//         PIN_GetLock(&thread_data[threadid].dyn_lock, threadid);
//             // ~ gzwrite(thread_data[threadid].gzDynamicTraceFile, dyn_str, strlen(dyn_str));
//             gzwrite(thread_data[threadid].gzDynamicTraceFile, dyn_str, strlen(dyn_str));
//         PIN_ReleaseLock(&thread_data[threadid].dyn_lock);
//     }
// };


// =====================================================================
VOID write_memory(BOOL is_Read, ADDRINT addr, INT32 size, UINT32 bbl, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("write_memory()\n");

    if (thread_data[threadid].is_instrumented_bbl == false) return;     /// If the pin-points disabled this region

    char mem_str[TRACE_LINE_SIZE];
    char rw;

    if (is_Read) {
        rw = 'R';
    }
    else {
        rw = 'W';
    }

    sprintf(mem_str, "%c %d %" PRIu64 " %d\n",  rw, size, (uint64_t)addr, bbl);

    gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));

};
// =====================================================================

VOID unknown_memory_size_f(PIN_MULTI_MEM_ACCESS_INFO* multi_size, UINT32 bbl, THREADID threadid) {
    uint32_t i;
    uint32_t max = multi_size->numberOfMemops;
    // Salva loads e stores
    for(i = 0; i < max; ++i) {

        PIN_MEM_ACCESS_INFO *info = &multi_size->memop[i];

        TRACE_GENERATOR_DEBUG_PRINTF("write_memory()\n");

        if (thread_data[threadid].is_instrumented_bbl == false) return;     /// If the pin-points disabled this region
        
        char mem_str[TRACE_LINE_SIZE];
        char rw;
        if (info->memopType == PIN_MEMOP_LOAD) {
            rw = 'R';
        }
        else {
            rw = 'W';
        }

        sprintf(mem_str, "%c %d %" PRIu64 " %d\n", 
                          rw, static_cast<int32_t> (info->bytesAccessed),
                          (uint64_t)info->memoryAddress, bbl);

        gzwrite(thread_data[threadid].gzMemoryTraceFile, mem_str, strlen(mem_str));

    }

}

//==============================================================================
VOID trace_instruction(TRACE trace, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("trace_instruction()\n");
    char bbl_count_str[TRACE_LINE_SIZE];

    std::string rtn_name;
    RTN rtn = TRACE_Rtn(trace);
    if(RTN_Valid(rtn)) {
        RTN_Open(rtn);
        rtn_name = RTN_Name(rtn);
        sprintf(bbl_count_str, "#%s\n", rtn_name.c_str());
        write_static_char(bbl_count_str);    // Write the static trace
        RTN_Close(rtn);
    }
    if (strcmp(rtn_name.c_str(), "omp_get_num_procs") == 0 ||           // <== SpinLock Static Linked (its binary has a PAUSE instruction)
        strcmp(rtn_name.c_str(), "gomp_barrier_wait_end") == 0 ||       // <== SpinLock Static Linked (its binary has a PAUSE instruction)
        strcmp(rtn_name.c_str(), "gomp_team_barrier_wait_end") == 0){   // <== SpinLock Static Linked (its binary has a PAUSE instruction)
        return;
    }

    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {

        // HMC Traces
        if (((KnobTrace.Value().compare(0, 3, "x86")) != 0) && icheck_conditions(rtn_name))
            continue;

        //----------------------------------------------------------------------
        // Write the static trace (assembly instructions)
        //----------------------------------------------------------------------
        count_trace++;              // Identify basic blocks with a counter id
        sprintf(bbl_count_str, "@%u\n", count_trace);
        write_static_char(bbl_count_str);    // Write the static trace


        //----------------------------------------------------------------------
        // Write the dynamic trace (basic block numbers)
        //----------------------------------------------------------------------
        char *bbl_str = new char[32];
        sprintf(bbl_str, "%u\n", count_trace);

        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(control_instrumented_bbl),
                        IARG_THREAD_ID, IARG_END);

        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins)) {

            if(INS_hasKnownMemorySize(ins)) {
                opcode_package_t pck = x86_to_static(ins);       // pin::ins => static trace

                //--------------------------------------------------------------------------
                // Write into the static trace
                //--------------------------------------------------------------------------
                char opcode_str[TRACE_LINE_SIZE];
                opcodes::opcode_to_trace_string(pck, opcode_str);
                write_static_char(opcode_str);
                

                //--------------------------------------------------------------------------
                // Write the Memory
                // is_instrumenteds loads using a predicated call, i.e.
                // the call happens if the load will be actually executed
                //--------------------------------------------------------------------------
                if (INS_IsMemoryRead(ins)) {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_memory, IARG_BOOL, true, IARG_MEMORYREAD_EA, IARG_MEMORYREAD_SIZE, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
                }
                if (INS_HasMemoryRead2(ins)) {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_memory, IARG_BOOL, true, IARG_MEMORYREAD2_EA, IARG_MEMORYREAD_SIZE, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
                }
                if (INS_IsMemoryWrite(ins)) {
                    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)write_memory, IARG_BOOL, false, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
                }

            } else {

                std::vector<opcode_package_t> *op = vgather_vscatter_to_static(ins);
                std::vector<opcode_package_t>::iterator it;

                it = op->begin();
                while(it != op->end()) {
                    
                    //--------------------------------------------------------------------------
                    // Write into the static trace
                    //--------------------------------------------------------------------------
                    char opcode_str[TRACE_LINE_SIZE];
                    opcodes::opcode_to_trace_string(*it, opcode_str);
                    write_static_char(opcode_str);

                    ++it;
                }
                delete(op);

                //--------------------------------------------------------------------------
                // Memory accesses list
                //--------------------------------------------------------------------------
                INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)unknown_memory_size_f, IARG_MULTI_MEMORYACCESS_EA, IARG_UINT32, count_trace, IARG_THREAD_ID, IARG_END);
            }
        }

        // HMC Traces
        if (((KnobTrace.Value().compare(0, 3, "x86")) != 0) && icheck_conditions(rtn_name))
            continue;

        INS_InsertCall(BBL_InsTail(bbl), IPOINT_BEFORE, AFUNPTR(write_dynamic_char), IARG_PTR, bbl_str, IARG_THREAD_ID, IARG_END);

    }
};

//==============================================================================
VOID Fini(INT32 code, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("Fini()\n");
    char trace_end[TRACE_LINE_SIZE];
    sprintf(trace_end, "# eof\n");

    gzwrite(gzStaticTraceFile, trace_end, strlen(trace_end));
    gzclose(gzStaticTraceFile);
    for (UINT32 i = 0; i < max_threads; i++){
        gzwrite(thread_data[i].gzDynamicTraceFile, trace_end, strlen(trace_end));
        gzclose(thread_data[i].gzDynamicTraceFile);

        gzwrite(thread_data[i].gzMemoryTraceFile, trace_end, strlen(trace_end));
        gzclose(thread_data[i].gzMemoryTraceFile);
    }
};

//==============================================================================
// Control the Number of Parallel Regions to trace
//==============================================================================
VOID handleParallelControlEvent(bool is_start, THREADID threadid) {
    TRACE_GENERATOR_DEBUG_PRINTF("handleParallelControlEvent()\n");

    /// If no parallel instrumentation was done => return
    if (KnobParallelStart.Value() < 0 && KnobParallelEnd.Value() < 0) return;

    PIN_GetLock(&lock, threadid);
        if (is_start) {
            /// New PARALLEL START
            count_parallel_start++;
        }
        else {
            /// New PARALLEL END
            count_parallel_end++;
        }

        bool old_status = is_instrumented;

        /// Reach the start parallel region
        if (count_parallel_start >= KnobParallelStart.Value() && KnobParallelStart.Value() >= 0) {
            TRACE_GENERATOR_DEBUG_PRINTF("\t Parallel START tracing\n");
            is_instrumented = true;
        }

        /// Reach the end parallel region
        if (count_parallel_end >= KnobParallelEnd.Value() && KnobParallelEnd.Value() >= 0) {
            TRACE_GENERATOR_DEBUG_PRINTF("\t Parallel STOP tracing\n");
            is_instrumented = false;

            // ~ ///=================================================================
            // ~ /// EXIT the application after trace the slice wanted
            // ~ printf("\t Parallel %d[%s] => %d[%s]\n", count_parallel_start, old_status ? "TRUE":"FALSE", count_parallel_end, is_instrumented ? "TRUE":"FALSE");
            // ~ PIN_ExitApplication(EXIT_SUCCESS);
            // ~ ///=================================================================
        }

    if (old_status != is_instrumented) {
        printf("\t Parallel Start(%d) End(%d) - Trace Status [%s] => [%s]\n", count_parallel_start, count_parallel_end, old_status ? "ON":"OFF", is_instrumented ? "ON":"OFF");
    }
    PIN_ReleaseLock(&lock);
};

//==============================================================================
// Pin Point Events
//==============================================================================
// ~ VOID handleControlEvent(CONTROLLER::EVENT_TYPE ev, VOID *, CONTEXT * ctxt, VOID *, THREADID threadid) {
VOID handleControlEvent(CONTROLLER::EVENT_TYPE ev, VOID *val, CONTEXT * ctxt, VOID *ip, THREADID threadid, bool bcast) {
    TRACE_GENERATOR_DEBUG_PRINTF("handleControlEvent()\n");

    /// If (parallel instrumentation) => return
    if (KnobParallelStart.Value() >= 0 || KnobParallelEnd.Value() >= 0) return;

    PIN_GetLock(&lock, threadid);
        bool old_status = is_instrumented;

        switch(ev) {
            case CONTROLLER::EVENT_START:
                TRACE_GENERATOR_DEBUG_PRINTF("\t PinPoint START tracing\n");
                is_instrumented = true;
            break;

            case CONTROLLER::EVENT_STOP:
                TRACE_GENERATOR_DEBUG_PRINTF("\t PinPoint STOP tracing\n");
                is_instrumented = false;
            break;

            default:
                ASSERTX(false);
            break;
        }

        if (old_status != is_instrumented) {
            TRACE_GENERATOR_DEBUG_PRINTF("\t PinPoint %s => %s\n", old_status ? "TRUE":"FALSE", is_instrumented ? "TRUE":"FALSE");
        }

    PIN_ReleaseLock(&lock);
};

//==============================================================================
VOID ThreadStart(THREADID threadid, CONTEXT *ctxt, INT32 flags, VOID *v) {
    TRACE_GENERATOR_DEBUG_PRINTF("ThreadStart()\n");
    PIN_GetLock(&lock, threadid);
        ASSERTX(threadid < max_threads);
    PIN_ReleaseLock(&lock);
};

//==============================================================================
VOID DynamicOMP_char(char *sync_str, THREADID threadid, bool is_spawn) {
    TRACE_GENERATOR_DEBUG_PRINTF("==>(%d) %s", threadid, sync_str);
    write_dynamic_char(sync_str, threadid);

    if (is_spawn) {
        for (uint32_t j = 1; j < max_threads; j++) {
            TRACE_GENERATOR_DEBUG_PRINTF("==>(%d)>(%d) %s", threadid, j, sync_str);
            write_dynamic_char(sync_str, j);
        }
    }
};



//==============================================================================
// Instrumentation Routines
//==============================================================================
// This routine is executed for each image.
VOID ImageLoad(IMG img, VOID *) {
    printf("Loading %s, Image id = %d\n", IMG_Name(img).c_str(), IMG_Id(img));

    // HMC data initialization - HMC Traces
    data_instr hmc_x86_data[20], vim_x86_data[112], mps_x86_data[28];
    initialize_intrinsics(hmc_x86_data, vim_x86_data, mps_x86_data);

    TRACE_GENERATOR_DEBUG_PRINTF("ImageLoad()\n");
    /// Only the thread master runs these calls
    std::vector<const char*> OMP_barrier_master_start;
        OMP_barrier_master_start.push_back("GOMP_parallel_start");
        OMP_barrier_master_start.push_back("GOMP_parallel_loop_dynamic_start");
        OMP_barrier_master_start.push_back("GOMP_parallel_loop_static_start");

    std::vector<const char*> OMP_barrier_master_end;
        OMP_barrier_master_end.push_back("GOMP_parallel_end");

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

    bool found_GOMP;
    std::string rtn_name;

    for (SEC sec = IMG_SecHead(img); SEC_Valid(sec); sec = SEC_Next(sec)) {
        // RTN_InsertCall() and INS_InsertCall() are executed in order of
        // appearance.  The IPOINT_AFTER may be executed before the IPOINT_BEFORE.
        for (RTN rtn = SEC_RtnHead(sec); RTN_Valid(rtn); rtn = RTN_Next(rtn)) {
            found_GOMP = false;
            rtn_name = RTN_Name(rtn);

            // Syntetic Traces (HMC, x86, VIMA, MIPS)
            if ((KnobTrace.Value().compare(0, 3, "x86")) != 0) {
                synthetic_trace_generation(rtn_name, hmc_x86_data, vim_x86_data, mps_x86_data, rtn);
            }

            // ~ if (
            // ~ //==========================================================
            // ~ // Dynamic Linked Programs...
            // ~ // No Pause but inlined during compilation
            // ~ // strcmp(rtn_name.c_str(), "GOMP_barrier") == 0 ||    // <== Dynamic Linked, works only for spinlock end, because the RTN in inline
// ~
            // ~ //==========================================================
            // ~ // libgomp.so.1
            // ~ // strcmp(rtn_name.c_str(), "omp_set_nest_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "omp_test_nest_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "omp_get_num_procs") >= 0 ||
// ~
            // ~ //==========================================================
            // ~ // libgomp.a - Requires Static Linkage
            // ~ // strcmp(rtn_name.c_str(), "gomp_mutex_lock_slow") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "gomp_sem_wait_slow") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "gomp_ptrlock_get_slow") >= 0 ||
            // ~ strcmp(rtn_name.c_str(), "gomp_barrier_wait_end") == 0 ||      // <== Static Linked
            // ~ strcmp(rtn_name.c_str(), "gomp_team_barrier_wait_end") == 0    // <== Static Linked
// ~
            // ~ //==========================================================
            // ~ // libpthread.so.0
            // ~ // libpthread.a
            // ~ // strcmp(rtn_name.c_str(), "pthread_mutex_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "pthread_mutex_cond_lock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "pthread_spinlock") >= 0 ||
            // ~ // strcmp(rtn_name.c_str(), "pthread_mutex_timedlock") >= 0 ||
            // ~ ) {
                // ~ RTN_Open(rtn);
                // ~ // SPIN LOCK START
                // ~ char *sync_str = new char[200];
                // ~ sprintf(sync_str, "#BEGIN:\"%s\"\n", rtn_name.c_str());
                // ~ std::string tempStr = sync_str.str();  
                // ~ RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, false, IARG_END);
// ~
                // ~ // Examine each instruction in the routine.
                // ~ for( INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins) ) {
                    // ~ // instrument each return instruction.
                    // ~ if( INS_IsRet(ins) ) {
                        // ~ // SPIN LOCK END
                        // ~ char *sync_str = new char[200];
                        // ~ sprintf(sync_str, "#END:\"%s\"\n", rtn_name.c_str());
                        // ~ // IPOINT_TAKEN_BRANCH always occurs last.
                        // ~ // std::string tempStr = sync_str.str();
                        // ~ // INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, false, IARG_END);
                    // ~ }
                // ~ }
// ~
                // ~ RTN_Close(rtn);
            // ~ }

            if (rtn_name.compare(0, 4, "GOMP") != 0) {
                continue;
            }
            // ~ printf("%s\n", rtn_name.c_str());

            /// Barrier only on Master, insert on all the traces (PARALLEL_END)
            for (uint32_t i = 0; i < OMP_barrier_master_end.size() && !found_GOMP; i++){
                /// FOUND a Parallel END
                if (strcmp(rtn_name.c_str(), OMP_barrier_master_end[i]) == 0) {
                    RTN_Open(rtn);

                    
                    std::stringstream sync_str;
                    sync_str << "#" << rtn_name.c_str() << "\n"         \
                            << "$" << SYNC_BARRIER      << "\n";
                    /*
                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_BARRIER);
                    */
                    std::string tempStr = sync_str.str();
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, true, IARG_END);
                    /// Parallel End
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(handleParallelControlEvent), IARG_BOOL, false, IARG_THREAD_ID, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }
            /// Barrier only on Master, insert on all the traces (PARALLEL_START)
            for (uint32_t i = 0; i < OMP_barrier_master_start.size() && !found_GOMP; i++){
                /// FOUND a Parallel START
                if (strcmp(rtn_name.c_str(), OMP_barrier_master_start[i]) == 0) {
                    RTN_Open(rtn);

                    std::stringstream sync_str;
                    sync_str << "#" << rtn_name.c_str() << "\n"         \
                            << "$" << SYNC_BARRIER      << "\n";
                    /*
                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_BARRIER);
                    */
                    /// Parallel Start
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(handleParallelControlEvent), IARG_BOOL, true, IARG_THREAD_ID, IARG_END);
                    
                    std::string tempStr = sync_str.str();
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, true, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Barrier on all the threads
            for (uint32_t i = 0; i < OMP_barrier_simple.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_barrier_simple[i]) == 0) {
                    RTN_Open(rtn);

                    std::stringstream sync_str;
                    sync_str << "#" << rtn_name.c_str() << "\n"         \
                            << "$" << SYNC_BARRIER      << "\n";
                    /*
                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_BARRIER);
                    */
                    std::string tempStr = sync_str.str();
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Enter in a critical section
            for (uint32_t i = 0; i < OMP_critical_start.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_critical_start[i]) == 0) {
                    RTN_Open(rtn);

                    std::stringstream sync_str;
                    sync_str << "#" << rtn_name.c_str()     << "\n"         \
                            << "$" << SYNC_CRITICAL_START   << "\n";
                    /*
                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_CRITICAL_START);
                    */
                    std::string tempStr = sync_str.str();
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                    RTN_Close(rtn);
                    found_GOMP = true;
                    break;
                }
            }

            /// Exit the critical section
            for (uint32_t i = 0; i < OMP_critical_end.size() && !found_GOMP; i++){
                if (strcmp(rtn_name.c_str(), OMP_critical_end[i]) == 0) {
                    RTN_Open(rtn);

                    std::stringstream sync_str;
                    sync_str << "#" << rtn_name.c_str()     << "\n"         \
                            << "$" << SYNC_CRITICAL_END     << "\n";
                    /*
                    char *sync_str = new char[TRACE_LINE_SIZE];
                    sprintf(sync_str, "#%s\n", rtn_name.c_str());
                    sprintf(sync_str, "%s$%u\n", sync_str, SYNC_CRITICAL_END);
                    */
                    std::string tempStr = sync_str.str();
                    RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, tempStr.c_str(), IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

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

                        char *sync_str = new char[TRACE_LINE_SIZE];
                        sprintf(sync_str, "#Ignoring GOMP call:\"%s\"\n", rtn_name.c_str());
                        RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                        RTN_Close(rtn);
                    #endif
                    found_GOMP = true;
                    break;
                }
            }

            if (!found_GOMP) {
                /// If a different OpenMP call is found on the binary
                RTN_Open(rtn);

                char *sync_str = new char[TRACE_LINE_SIZE];
                sprintf(sync_str, "#Found different GOMP call:\"%s\"\n", rtn_name.c_str());
                RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(DynamicOMP_char), IARG_PTR, sync_str, IARG_THREAD_ID, IARG_BOOL, false, IARG_END);

                RTN_Close(rtn);
            }
        }
    }
};

//==============================================================================
// Main
//==============================================================================
int main(int argc, char *argv[]) {
    std::stringstream trace_header;
    trace_header << "#\n"                                                       \
                << "# Compressed Trace Generated By Pin to SiNUCA\n"            \
                << "#\n";
    std::string tempStr = trace_header.str();
    /*
    char trace_header[TRACE_LINE_SIZE];
    sprintf(trace_header, "#\n");
    sprintf(trace_header, "%s# Compressed Trace Generated By Pin to SiNUCA\n", trace_header);
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
        thread_data[i].is_instrumented_bbl = false;
    }

    pinplay_engine.Activate(argc, argv, KnobLogger, KnobReplayer);

    // Activate alarm, must be done before PIN_StartProgram
    control.RegisterHandler(handleControlEvent, 0, FALSE);
    control.Activate();

    //==========================================================================
    // Static Trace File
    //==========================================================================
    static char stat_file_name[500];
    stat_file_name[0] = '\0';

    printf("Inserted Output File Name = %s\n", KnobOutputFile.Value().c_str());

    sprintf(stat_file_name , "%s.tid0.stat.out.gz", KnobOutputFile.Value().c_str());
    gzStaticTraceFile = gzopen(stat_file_name, "wb");   /// Open the .gz file
    ASSERTX(gzStaticTraceFile != NULL);                 /// Check the .gz file
    gzwrite(gzStaticTraceFile, tempStr.c_str(), strlen(tempStr.c_str()));

    printf("Real Static File = %s => READY !\n",stat_file_name);

    //==========================================================================
    // Dynamic Trace Files
    //==========================================================================
    static char dyn_file_name[500];

    for (UINT32 i = 0; i < max_threads; i++){
        dyn_file_name[0] = '\0';
        sprintf(dyn_file_name, "%s.tid%d.dyn.out.gz", KnobOutputFile.Value().c_str(), i);
        thread_data[i].gzDynamicTraceFile = gzopen(dyn_file_name, "wb");    /// Open the .gz file
        ASSERTX(thread_data[i].gzDynamicTraceFile != NULL);                 /// Check the .gz file
        gzwrite(thread_data[i].gzDynamicTraceFile, tempStr.c_str(), strlen(tempStr.c_str()));
        printf("Real Dynamic File = %s => READY !\n", dyn_file_name);
    }

    //==========================================================================
    // Memory Trace Files
    //==========================================================================
    static char mem_file_name[500];

    for (UINT32 i = 0; i < max_threads; i++){
        mem_file_name[0] = '\0';
        sprintf(mem_file_name, "%s.tid%d.mem.out.gz", KnobOutputFile.Value().c_str(), i);
        thread_data[i].gzMemoryTraceFile = gzopen(mem_file_name, "wb");     /// Open the .gz file
        ASSERTX(thread_data[i].gzMemoryTraceFile != NULL);                  /// Check the .gz file
        gzwrite(thread_data[i].gzMemoryTraceFile, tempStr.c_str(), strlen(tempStr.c_str()));
        printf("Real Memory File = %s => READY !\n", mem_file_name);
    }

    //=======================================================================
    // Register ImageLoad to be called when an image is loaded
    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Register ThreadStart to be called when a thread starts.
    PIN_AddThreadStartFunction(ThreadStart, 0);

    // Static, Dynamic and Memory Trace
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
