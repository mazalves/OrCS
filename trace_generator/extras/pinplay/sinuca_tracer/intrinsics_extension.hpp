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
// INTRINSICS EXTENSION HEADER
///###########################################################################
///  This file defines functions for intrinsics extensions.
///###########################################################################

#ifndef TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_INTRINSICS_EXTENSION_H_
    #define TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_INTRINSICS_EXTENSION_H_
#endif  //  TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_INTRINSICS_EXTENSION_H_

#include <zlib.h>
#include <vector>
#include <string>

#include "../../../../utils/enumerations.hpp"
#include "./opcodes.hpp"
#include "./vima_defines.h"
#include "./mps_defines.h"
#include "./hmc_defines.h"

#ifdef TRACE_GENERATOR_DEBUG
    #define TRACE_GENERATOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define TRACE_GENERATOR_DEBUG_PRINTF(...)
#endif


//==============================================================================
// Commandline Switches
//==============================================================================
KNOB<string> KnobTrace(KNOB_MODE_WRITEONCE, "pintool", "trace", "null",
    "simulates intrinsics-x86, instrisics-hmc, VIMA, or MIPS");

//==============================================================================
// Global Variables
//==============================================================================

// Force each thread's data to be in its own data cache line so that
// multiple threads do not contend for the same data cache line.
// This avoids the false sharing problem.
#define PADSIZE 64

class thread_data_t {
 public:
        PIN_LOCK dyn_lock;
        bool is_instrumented_bbl;

        gzFile gzDynamicTraceFile;
        gzFile gzMemoryTraceFile;

        UINT8 pad[PADSIZE];
};

PIN_LOCK lock;              /// Lock for methods shared among multiple threads
thread_data_t* thread_data;  /// Lock for methods shared among multiple threads

uint32_t count_trace = 0;   /// Current BBL trace number
bool is_instrumented = false;  /// Will be enabled by PinPoints

std::ofstream StaticTraceFile;
gzFile gzStaticTraceFile;

// Other architectures structure (HMC/VIMA)
struct data_instr {
    char const *rtn_name;
    char const *arch_instr_name;
    char const *x86_instr_name;
    UINT32 instr_len;
};

//==============================================================================
// Header Functions
//==============================================================================
VOID initialize_intrinsics_hmc(data_instr hmc_x86_data[HMC_INS_COUNT]);
VOID initialize_intrinsics_vima(data_instr vim_x86_data[VIMA_INS_COUNT]);
VOID initialize_intrinsics_mips(data_instr mps_x86_data[MPS_INS_COUNT]);
INT icheck_conditions(std::string rtn_name);
INT icheck_conditions_hmc(std::string rtn_name);
INT icheck_conditions_vima(std::string rtn_name);
INT icheck_conditions_mips(std::string rtn_name);
INT icheck_1parameter(std::string rtn_name);
INT icheck_2parameters(std::string rtn_name);
VOID write_dynamic_char(char *dyn_str, THREADID threadid);
VOID write_static_char(char *stat_str);
VOID hmc_write_memory_1param(ADDRINT read, UINT32 size,
                            UINT32 bbl, THREADID threadid);
VOID hmc_write_memory_2param(ADDRINT read, ADDRINT write,
                            UINT32 size, UINT32 bbl, THREADID threadid);
VOID hmc_write_memory_3param(ADDRINT read1, ADDRINT read2,
                            ADDRINT write, UINT32 size,
                            UINT32 bbl, THREADID threadid);
VOID arch_x86_set_data_instr(data_instr *hmc_x86_data,
                            char const *rtn_name,
                            char const *hmc_instr_name,
                            char const *x86_instr_name,
                            UINT32 instr_len);
VOID arch_x86_trace_instruction(RTN hmc_x86_rtn, data_instr *hmc_x86_data);
VOID synthetic_trace_generation(std::string rtn_name,
                                data_instr hmc_x86_data[HMC_INS_COUNT],
                                data_instr vim_x86_data[VIMA_INS_COUNT],
                                data_instr mps_x86_data[MPS_INS_COUNT],
                                RTN rtn);
VOID specific_trace_generation(std::string rtn_name,
                                const char *arch_name,
                                int n_instr,
                                data_instr *arch_x86_data,
                                RTN rtn);
