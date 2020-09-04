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
///  mps_defines.h
///###########################################################################
/// This file defines MIPS instruction names, mnemonics, their total number,
/// and their equivalent x86 mnemonic.
///###########################################################################

#ifndef TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_MPS_DEFINES_H_
    #define TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_MPS_DEFINES_H_
#endif  //  TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_MPS_DEFINES_H_

#include<vector>
#include<string>

#define MPS_INS_COUNT 28
#define MPS_INS_SIZE 32
const char *mps_init[] = {  "_mps32_add",
                            "_mps32_addu",
                            "_mps32_sub",
                            "_mps32_subu",
                            "_mps32_addi",
                            "_mps32_addiu",
                            "_mps32_and",
                            "_mps32_nor",
                            "_mps32_or",
                            "_mps32_xor",
                            "_mps32_andi",
                            "_mps32_ori",
                            "_mps32_xori",
                            "_mps32_slt",
                            "_mps32_sltu",
                            "_mps32_slti",
                            "_mps32_sltiu",
                            "_mps32_sll",
                            "_mps32_srl",
                            "_mps32_sra",
                            "_mps32_div",
                            "_mps32_divu",
                            "_mps32_mod",
                            "_mps32_modu",
                            "_mps32_mult",
                            "_mps32_multu",
                            "_mps64_mult",
                            "_mps64_multu"
                            };

static const std::vector<string> mps_inst_names(mps_init,
                                    mps_init + MPS_INS_COUNT);

const char *mps_mnem_init[] = { "MIPS_ADD32_OPER",
                                "MIPS_ADDU32_OPER",
                                "MIPS_SUB32_OPER",
                                "MIPS_SUBU32_OPER",
                                "MIPS_ADDI32_OPER",
                                "MIPS_ADDIU32_OPER",
                                "MIPS_AND32_OPER",
                                "MIPS_NOR32_OPER",
                                "MIPS_OR32_OPER",
                                "MIPS_XOR32_OPER",
                                "MIPS_ANDI32_OPER",
                                "MIPS_ORI32_OPER",
                                "MIPS_XORI32_OPER",
                                "MIPS_SLT32_OPER",
                                "MIPS_SLTU32_OPER",
                                "MIPS_SLTI32_OPER",
                                "MIPS_SLTIU32_OPER",
                                "MIPS_SLL32_OPER",
                                "MIPS_SRL32_OPER",
                                "MIPS_SRA32_OPER",
                                "MIPS_DIV32_OPER",
                                "MIPS_DIVU32_OPER",
                                "MIPS_MOD32_OPER",
                                "MIPS_MODU32_OPER",
                                "MIPS_MULT32_OPER",
                                "MIPS_MULTU32_OPER",
                                "MIPS_MULT64_OPER",
                                "MIPS_MULTU64_OPER"
                                };
static const std::vector<string> mps_mnem_names(mps_mnem_init,
                                    mps_mnem_init + MPS_INS_COUNT);


const char *mps_x86mnem_init[] = { "x86_ADD32_OPER",
                                "x86_ADDU32_OPER",
                                "x86_SUB32_OPER",
                                "x86_SUBU32_OPER",
                                "x86_ADDI32_OPER",
                                "x86_ADDIU32_OPER",
                                "x86_AND32_OPER",
                                "x86_NOR32_OPER",
                                "x86_OR32_OPER",
                                "x86_XOR32_OPER",
                                "x86_ANDI32_OPER",
                                "x86_ORI32_OPER",
                                "x86_XORI32_OPER",
                                "x86_SLT32_OPER",
                                "x86_SLTU32_OPER",
                                "x86_SLTI32_OPER",
                                "x86_SLTIU32_OPER",
                                "x86_SLL32_OPER",
                                "x86_SRL32_OPER",
                                "x86_SRA32_OPER",
                                "x86_DIV32_OPER",
                                "x86_DIVU32_OPER",
                                "x86_MOD32_OPER",
                                "x86_IMODU32_OPER",
                                "x86_MULT32_OPER",
                                "x86_MULTU32_OPER",
                                "x86_MULT64_OPER",
                                "x86_MULTU64_OPER"
                                };

static const std::vector<string> mps_x86mnem_names(mps_x86mnem_init,
                                    mps_x86mnem_init + MPS_INS_COUNT);
