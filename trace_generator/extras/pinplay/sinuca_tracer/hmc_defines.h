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
///  hmc_defines.h
///###########################################################################
///  This file defines HMC instruction names, mnemonics, their total number,
///  and their equivalent x86 mnemonic.
///###########################################################################

#ifndef TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_HMC_DEFINES_H_
    #define TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_HMC_DEFINES_H_
#endif  //  TRACE_GENERATOR_EXTRAS_PINPLAY_SINUCA_TRACER_HMC_DEFINES_H_

#include<vector>
#include<string>

#define HMC_INS_COUNT 20
#define HMC_ROA_COUNT 8


#define HMC64_INS_COUNT 9
#define HMC64_INS_SIZE 8

#define HMC128_INS_COUNT 11
#define HMC128_INS_SIZE 16

const char *hmc_init[] = {  "_hmc128_saddim_s",
                            "_hmc64_incr_s",
                            "_hmc64_bwrite_s",
                            "_hmc128_bswap_s",
                            "_hmc128_or_s",
                            "_hmc128_and_s",
                            "_hmc128_nor_s",
                            "_hmc128_xor_s",
                            "_hmc128_nand_s",
                            "_hmc64_equalto_s",
                            "_hmc128_equalto_s",
                            "_hmc64_cmpswapgt_s",
                            "_hmc64_cmpswaplt_s",
                            "_hmc128_cmpswapz_s",
                            "_hmc64_cmpswapeq_s",
                            "_hmc128_cmpswapgt_s",
                            "_hmc128_cmpswaplt_s",
                            "_hmc64_cmpgteq_s",
                            "_hmc64_cmplteq_s",
                            "_hmc64_cmplt_s"
                            };

static const std::vector<std::string> hmc_inst_names(hmc_init,
                                        hmc_init + HMC_INS_COUNT);

const char *hmc64_init[] = { "_hmc64_incr_s",
                             "_hmc64_bwrite_s",
                             "_hmc64_equalto_s",
                             "_hmc64_cmpswapgt_s",
                             "_hmc64_cmpswaplt_s",
                             "_hmc64_cmpswapeq_s",
                             "_hmc64_cmpgteq_s",
                             "_hmc64_cmplteq_s",
                             "_hmc64_cmplt_s"
                            };
static const std::vector<std::string> hmc64_inst_names(hmc64_init,
                                        hmc64_init + HMC64_INS_COUNT);

const char *hmc128_init[] = {  "_hmc128_saddim_s",
                                "_hmc128_bswap_s",
                                "_hmc128_or_s",
                                "_hmc128_and_s",
                                "_hmc128_nor_s",
                                "_hmc128_xor_s",
                                "_hmc128_nand_s",
                                "_hmc128_equalto_s",
                                "_hmc128_cmpswapz_s",
                                "_hmc128_cmpswapgt_s",
                                "_hmc128_cmpswaplt_s"
                                };
static const std::vector<std::string> hmc128_inst_names(hmc128_init,
                                        hmc128_init + HMC128_INS_COUNT);


const char *hmc_roa_init[] = {  "_hmc128_saddim_s",
                                "_hmc64_incr_s",
                                "_hmc64_bwrite_s",
                                "_hmc64_equalto_s",
                                "_hmc128_equalto_s",
                                "_hmc64_cmpgteq_s",
                                "_hmc64_cmplteq_s",
                                "_hmc64_cmplt_s"
                                };
static const std::vector<std::string> hmc_roa_names(hmc_roa_init,
                                        hmc_roa_init + HMC_ROA_COUNT);

const char *hmc_mnem_init[] = { "HMC_ADD_SINGLE_128OPER",
                                "HMC_INCR_SINGLE_64OPER",
                                "HMC_BITWRITE_SINGLE_64OPER",
                                "HMC_BITSWAP_SINGLE_128OPER",
                                "HMC_AND_SINGLE_128OPER",
                                "HMC_NAND_SINGLE_128OPER",
                                "HMC_NOR_SINGLE_128OPER",
                                "HMC_OR_SINGLE_128OPER",
                                "HMC_XOR_SINGLE_128OPER",
                                "HMC_CMPSWAPGT_SINGLE_64OPER",
                                "HMC_CMPSWAPLT_SINGLE_64OPER",
                                "HMC_CMPSWAPZ_SINGLE_128OPER",
                                "HMC_CMPSWAPGT_SINGLE_128OPER",
                                "HMC_CMPSWAPLT_SINGLE_128OPER",
                                "HMC_CMPSWAPEQ_SINGLE_64OPER",
                                "HMC_EQUALTO_SINGLE_64OPER",
                                "HMC_EQUALTO_SINGLE_128OPER",
                                "HMC_CMPGTEQ_SINGLE_64OPER",
                                "HMC_CMPLTEQ_SINGLE_64OPER",
                                "HMC_CMPLT_SINGLE_64OPER"
                                };
static const std::vector<std::string> hmc_mnem_names(hmc_mnem_init,
                                        hmc_mnem_init + HMC_INS_COUNT);

const char *hmc64_mnem_init[] = {   "HMC_INCR_SINGLE_64OPER",
                                    "HMC_BITWRITE_SINGLE_64OPER",
                                    "HMC_CMPSWAPGT_SINGLE_64OPER",
                                    "HMC_CMPSWAPLT_SINGLE_64OPER",
                                    "HMC_CMPSWAPEQ_SINGLE_64OPER",
                                    "HMC_EQUALTO_SINGLE_64OPER",
                                    "HMC_CMPGTEQ_SINGLE_64OPER",
                                    "HMC_CMPLTEQ_SINGLE_64OPER",
                                    "HMC_CMPLT_SINGLE_64OPER"
                                    };
static const std::vector<std::string> hmc64_mnem_names(hmc64_mnem_init,
                                        hmc64_mnem_init + HMC64_INS_COUNT);

const char *hmc128_mnem_init[] = {  "HMC_ADD_SINGLE_128OPER",
                                    "HMC_BITSWAP_SINGLE_128OPER",
                                    "HMC_AND_SINGLE_128OPER",
                                    "HMC_NAND_SINGLE_128OPER",
                                    "HMC_NOR_SINGLE_128OPER",
                                    "HMC_OR_SINGLE_128OPER",
                                    "HMC_XOR_SINGLE_128OPER",
                                    "HMC_CMPSWAPZ_SINGLE_128OPER",
                                    "HMC_CMPSWAPGT_SINGLE_128OPER",
                                    "HMC_CMPSWAPLT_SINGLE_128OPER",
                                    "HMC_EQUALTO_SINGLE_128OPER"
                                    };
static const std::vector<std::string> hmc128_mnem_names(hmc128_mnem_init,
                                        hmc128_mnem_init + HMC128_INS_COUNT);

const char *hmc_x86mnem_init[] = {  "x86_ADD_SINGLE_128OPER",
                                    "x86_INCR_SINGLE_64OPER",
                                    "x86_BITWRITE_SINGLE_64OPER",
                                    "x86_BITSWAP_SINGLE_128OPER",
                                    "x86_AND_SINGLE_128OPER",
                                    "x86_NAND_SINGLE_128OPER",
                                    "x86_NOR_SINGLE_128OPER",
                                    "x86_OR_SINGLE_128OPER",
                                    "x86_XOR_SINGLE_128OPER",
                                    "x86_CMPSWAPGT_SINGLE_64OPER",
                                    "x86_CMPSWAPLT_SINGLE_64OPER",
                                    "x86_CMPSWAPZ_SINGLE_128OPER",
                                    "x86_CMPSWAPGT_SINGLE_128OPER",
                                    "x86_CMPSWAPLT_SINGLE_128OPER",
                                    "x86_CMPSWAPEQ_SINGLE_64OPER",
                                    "x86_EQUALTO_SINGLE_64OPER",
                                    "x86_EQUALTO_SINGLE_128OPER",
                                    "x86_CMPGTEQ_SINGLE_64OPER",
                                    "x86_CMPLTEQ_SINGLE_64OPER",
                                    "x86_CMPLT_SINGLE_64OPER"
                                    };
static const std::vector<std::string> hmc_x86mnem_names(hmc_x86mnem_init,
                                        hmc_x86mnem_init + HMC_INS_COUNT);

const char *hmc64_x86mnem_init[]  = {   "x86_INCR_SINGLE_64OPER",
                                        "x86_BITWRITE_SINGLE_64OPER",
                                        "x86_CMPSWAPGT_SINGLE_64OPER",
                                        "x86_CMPSWAPLT_SINGLE_64OPER",
                                        "x86_CMPSWAPEQ_SINGLE_64OPER",
                                        "x86_EQUALTO_SINGLE_64OPER",
                                        "x86_CMPGTEQ_SINGLE_64OPER",
                                        "x86_CMPLTEQ_SINGLE_64OPER",
                                        "x86_CMPLT_SINGLE_64OPER"
                                        };

static const std::vector<std::string> hmc64_x86mnem_names(hmc64_x86mnem_init,
                                        hmc64_x86mnem_init + HMC64_INS_COUNT);

const char *hmc128_x86mnem_init[] = {   "x86_ADD_SINGLE_128OPER",
                                        "x86_BITSWAP_SINGLE_128OPER",
                                        "x86_AND_SINGLE_128OPER",
                                        "x86_NAND_SINGLE_128OPER",
                                        "x86_NOR_SINGLE_128OPER",
                                        "x86_OR_SINGLE_128OPER",
                                        "x86_XOR_SINGLE_128OPER",
                                        "x86_CMPSWAPZ_SINGLE_128OPER",
                                        "x86_CMPSWAPGT_SINGLE_128OPER",
                                        "x86_CMPSWAPLT_SINGLE_128OPER",
                                        "x86_EQUALTO_SINGLE_128OPER"
                                        };
static const std::vector<std::string> hmc128_x86mnem_names(hmc128_x86mnem_init,
                                        hmc128_x86mnem_init + HMC128_INS_COUNT);
