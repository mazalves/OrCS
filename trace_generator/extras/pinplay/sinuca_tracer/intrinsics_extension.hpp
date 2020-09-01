// INTRINSICS EXTENSION HEADER
#include "../../../../utils/enumerations.hpp"
#include "opcodes.hpp"
#include <zlib.h>
#include <vector>
#include <string>

#ifdef TRACE_GENERATOR_DEBUG
    #define TRACE_GENERATOR_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define TRACE_GENERATOR_DEBUG_PRINTF(...)
#endif

#define HMC_INS_COUNT 20
#define VIMA_INS_COUNT 112
#define MPS_INS_COUNT 28
#define x86_INS_COUNT 160

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

PIN_LOCK lock;                  /// Lock for methods shared among multiple threads
thread_data_t* thread_data;     /// Lock for methods shared among multiple threads

uint32_t count_trace = 0;       /// Current BBL trace number
bool is_instrumented = false;   /// Will be enabled by PinPoints

std::ofstream StaticTraceFile;
gzFile gzStaticTraceFile;

// Other architectures structure (HMC/VIMA)
struct data_instr {
    char const *rtn_name;
    char const *arch_instr_name;
    char const *x86_instr_name;
    UINT32 instr_len;
};

// HMC Instruction names
std::string cmp_name0 ("_hmc128_saddimm_s");
std::string cmp_name1 ("_hmc64_incr_s");
std::string cmp_name2 ("_hmc64_bwrite_s");      
std::string cmp_name3 ("_hmc128_bswap_s");
std::string cmp_name4 ("_hmc128_or_s");         
std::string cmp_name5 ("_hmc128_and_s");
std::string cmp_name6 ("_hmc128_nor_s");        
std::string cmp_name7 ("_hmc128_xor_s");
std::string cmp_name8 ("_hmc128_nand_s");       
std::string cmp_name9 ("_hmc64_equalto_s");
std::string cmp_name10 ("_hmc128_equalto_s");   
std::string cmp_name11 ("_hmc64_cmpswapgt_s");
std::string cmp_name12 ("_hmc64_cmpswaplt_s");  
std::string cmp_name13 ("_hmc128_cmpswapz_s");
std::string cmp_name14 ("_hmc64_cmpswapeq_s");  
std::string cmp_name15 ("_hmc128_cmpswapgt_s");
std::string cmp_name16 ("_hmc128_cmpswaplt_s"); 
std::string cmp_name17 ("_hmc64_cmpgteq_s");
std::string cmp_name18 ("_hmc64_cmplteq_s");    
std::string cmp_name19 ("_hmc64_cmplt_s");


const char *hmc_init[] =	{ "_hmc128_saddim_s",
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

#define HMC_ROA_COUNT 8
const char *hmc_roa_init[] = { "_hmc128_saddim_s",
							"_hmc64_incr_s",
							"_hmc64_bwrite_s",
							"_hmc64_equalto_s",
							"_hmc128_equalto_s",   
							"_hmc64_cmpgteq_s",
							"_hmc64_cmplteq_s",    
							"_hmc64_cmplt_s"
							};

const static std::vector<std::string> hmc_inst_names(hmc_init, hmc_init + HMC_INS_COUNT);
const static std::vector<std::string> hmc_roa_names(hmc_roa_init, hmc_roa_init + HMC_ROA_COUNT);

//HMC MEMN NAMES

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
							 "HMC_CMPLT_SINGLE_64OPER" };

const static std::vector<std::string> hmc_mnem_names(hmc_mnem_init, hmc_mnem_init + HMC_INS_COUNT);

const char *x86_mnem_init[] = { "x86_ADD_SINGLE_128OPER",
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
							"x86_CMPLT_SINGLE_64OPER",
							"x86_IADDS_64VECTOR_32OPER",
							"x86_IADDS_2KVECTOR_32OPER",
							"x86_IADDU_64VECTOR_32OPER",
							"x86_IADDU_2KVECTOR_32OPER",
							"x86_ISUBS_64VECTOR_32OPER",
							"x86_ISUBS_2KVECTOR_32OPER",
							"x86_ISUBU_64VECTOR_32OPER",
							"x86_ISUBU_2KVECTOR_32OPER",
							"x86_IABSS_64VECTOR_32OPER",
							"x86_IABSS_2KVECTOR_32OPER",
							"x86_IMAXS_64VECTOR_32OPER",
							"x86_IMAXS_2KVECTOR_32OPER",
							"x86_IMINS_64VECTOR_32OPER",
							"x86_IMINS_2KVECTOR_32OPER",
							"x86_ICPYS_64VECTOR_32OPER",
							"x86_ICPYS_2KVECTOR_32OPER",
							"x86_ICPYU_64VECTOR_32OPER",
							"x86_ICPYU_2KVECTOR_32OPER",
							"x86_IANDU_64VECTOR_32OPER",
							"x86_IANDU_2KVECTOR_32OPER",
							"x86_IORUN_64VECTOR_32OPER",
							"x86_IORUN_2KVECTOR_32OPER",
							"x86_IXORU_64VECTOR_32OPER",
							"x86_IXORU_2KVECTOR_32OPER",
							"x86_INOTS_64VECTOR_32OPER",
							"x86_INOTS_2KVECTOR_32OPER",
							"x86_ISLTS_64VECTOR_32OPER",
							"x86_ISLTS_2KVECTOR_32OPER",
							"x86_ISLTU_64VECTOR_32OPER",
							"x86_ISLTU_2KVECTOR_32OPER",
							"x86_ICMQS_64VECTOR_32OPER",
							"x86_ICMQS_2KVECTOR_32OPER",
							"x86_ICMQU_64VECTOR_32OPER",
							"x86_ICMQU_2KVECTOR_32OPER",
							"x86_ISLLU_64VECTOR_32OPER",
							"x86_ISLLU_2KVECTOR_32OPER",
							"x86_ISRLU_64VECTOR_32OPER",
							"x86_ISRLU_2KVECTOR_32OPER",
							"x86_ISRAS_64VECTOR_32OPER",
							"x86_ISRAS_2KVECTOR_32OPER",
							"x86_IDIVS_64VECTOR_32OPER",
							"x86_IDIVS_2KVECTOR_32OPER",
							"x86_IDIVU_64VECTOR_32OPER",
							"x86_IDIVU_2KVECTOR_32OPER",
							"x86_IDIVS_32VECTOR_64OPER",
							"x86_IDIVS_1KVECTOR_32OPER",
							"x86_IDIVU_32VECTOR_64OPER",
							"x86_IDIVU_1KVECTOR_32OPER",
							"x86_IMULS_64VECTOR_32OPER",
							"x86_IMULS_2KVECTOR_32OPER",
							"x86_IMULU_64VECTOR_32OPER",
							"x86_IMULU_2KVECTOR_32OPER",
							"x86_IMULS_32VECTOR_64OPER",
							"x86_IMULS_1KVECTOR_32OPER",
							"x86_IMULU_32VECTOR_64OPER",
							"x86_IMULU_1KVECTOR_32OPER",
							"x86_IMADU_64VECTOR_32OPER",
							"x86_IMADU_2KVECTOR_32OPER",
							"x86_IMADS_64VECTOR_32OPER",
							"x86_IMULU_2KVECTOR_32OPER",
							"x86_IMOVS_64VECTOR_32OPER",
							"x86_IMOVS_2KVECTOR_32OPER",
							"x86_IMOVU_64VECTOR_32OPER",
							"x86_IMOVU_2KVECTOR_32OPER",
							"x86_FADDS_64VECTOR_32OPER",
							"x86_FADDS_2KVECTOR_32OPER",
							"x86_FSUBS_64VECTOR_32OPER",
							"x86_FSUBS_2KVECTOR_32OPER",
							"x86_FABSS_64VECTOR_32OPER",
							"x86_FABSS_2KVECTOR_32OPER",
							"x86_FMAXS_64VECTOR_32OPER",
							"x86_FMAXS_2KVECTOR_32OPER",
							"x86_FMINS_64VECTOR_32OPER",
							"x86_FMINS_2KVECTOR_32OPER",
							"x86_FCPYS_64VECTOR_32OPER",
							"x86_FCPYS_2KVECTOR_32OPER",
							"x86_FSLTS_64VECTOR_32OPER",
							"x86_FSLTS_2KVECTOR_32OPER",
							"x86_FCMQS_64VECTOR_32OPER",
							"x86_FCMQS_2KVECTOR_32OPER",
							"x86_FDIVS_64VECTOR_32OPER",
							"x86_FDIVS_2KVECTOR_32OPER",
							"x86_FMULS_64VECTOR_32OPER",
							"x86_FMULS_2KVECTOR_32OPER",
							"x86_FMADS_64VECTOR_32OPER",
							"x86_FMADS_2KVECTOR_32OPER",
							"x86_FMOVS_64VECTOR_32OPER",
							"x86_FMOVS_2KVECTOR_32OPER",
							"x86_DADDS_32VECTOR_64OPER",
							"x86_DADDS_1KVECTOR_32OPER",
							"x86_DSUBS_32VECTOR_64OPER",
							"x86_DSUBS_1KVECTOR_32OPER",
							"x86_DABSS_32VECTOR_64OPER",
							"x86_DABSS_1KVECTOR_32OPER",
							"x86_DMAXS_32VECTOR_64OPER",
							"x86_DMAXS_1KVECTOR_32OPER",
							"x86_DMINS_32VECTOR_64OPER",
							"x86_DMINS_1KVECTOR_32OPER",
							"x86_DCPYS_32VECTOR_64OPER",
							"x86_DCPYS_1KVECTOR_32OPER",
							"x86_DSLTS_32VECTOR_64OPER",
							"x86_DSLTS_1KVECTOR_32OPER",
							"x86_DCMQS_32VECTOR_64OPER",
							"x86_DCMQS_1KVECTOR_32OPER",
							"x86_DDIVS_32VECTOR_64OPER",
							"x86_DDIVS_1KVECTOR_32OPER",
							"x86_DMULS_32VECTOR_64OPER",
							"x86_DMULS_1KVECTOR_32OPER",
							"x86_DMADS_32VECTOR_64OPER",
							"x86_DMADS_1KVECTOR_32OPER",
							"x86_DMOVS_32VECTOR_64OPER",
							"x86_DMOVS_1KVECTOR_32OPER",
							"x86_ADD32_OPER",
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
							"x86_IMSKU_2KVECTOR_32OPER",
							"x86_MULT64_OPER",
							"x86_MULTU64_OPER"};

const static std::vector<std::string> x86_mnem_names(x86_mnem_init,x86_mnem_init + x86_INS_COUNT);


// VIMA Instruction names
std::string cmp_name20("_vim64_iadds");     std::string cmp_name21("_vim2K_iadds");
std::string cmp_name22("_vim64_iaddu");     std::string cmp_name23("_vim2K_iaddu");
std::string cmp_name24("_vim64_isubs");     std::string cmp_name25("_vim2K_isubs");
std::string cmp_name26("_vim64_isubu");     std::string cmp_name27("_vim2K_isubu");
std::string cmp_name28("_vim64_iabss");     std::string cmp_name29("_vim2K_iabss");
std::string cmp_name30("_vim64_imaxs");     std::string cmp_name31("_vim2K_imaxs");
std::string cmp_name32("_vim64_imins");     std::string cmp_name33("_vim2K_imins");
std::string cmp_name34("_vim64_icpys");     std::string cmp_name35("_vim2K_icpys");
std::string cmp_name36("_vim64_icpyu");     std::string cmp_name37("_vim2K_icpyu");
std::string cmp_name38("_vim64_iandu");     std::string cmp_name39("_vim2K_iandu");
std::string cmp_name40("_vim64_iorun");     std::string cmp_name41("_vim2K_iorun");
std::string cmp_name42("_vim64_ixoru");     std::string cmp_name43("_vim2K_ixoru");
std::string cmp_name44("_vim64_inots");     std::string cmp_name45("_vim2K_inots");
std::string cmp_name46("_vim64_islts");     std::string cmp_name47("_vim2K_islts");
std::string cmp_name48("_vim64_isltu");     std::string cmp_name49("_vim2K_isltu");
std::string cmp_name50("_vim64_icmqs");     std::string cmp_name51("_vim2K_icmqs");
std::string cmp_name52("_vim64_icmqu");     std::string cmp_name53("_vim2K_icmqu");
std::string cmp_name54("_vim64_isllu");     std::string cmp_name55("_vim2K_isllu");
std::string cmp_name56("_vim64_isrlu");     std::string cmp_name57("_vim2K_isrlu");
std::string cmp_name58("_vim64_isras");     std::string cmp_name59("_vim2K_isras");
std::string cmp_name60("_vim64_idivs");     std::string cmp_name61("_vim2K_idivs");
std::string cmp_name62("_vim64_idivu");     std::string cmp_name63("_vim2K_idivu");
std::string cmp_name64("_vim32_idivs");     std::string cmp_name65("_vim1K_idivs");
std::string cmp_name66("_vim32_idivu");     std::string cmp_name67("_vim1K_idivu");
std::string cmp_name68("_vim64_imuls");     std::string cmp_name69("_vim2K_imuls");
std::string cmp_name70("_vim64_imulu");     std::string cmp_name71("_vim2K_imulu");
std::string cmp_name72("_vim32_imuls");     std::string cmp_name73("_vim1K_imuls");
std::string cmp_name74("_vim32_imulu");     std::string cmp_name75("_vim1K_imulu");
std::string cmp_name76("_vim64_icumu");     std::string cmp_name77("_vim2K_icumu");
std::string cmp_name78("_vim64_icums");     std::string cmp_name79("_vim2K_icums");
std::string cmp_name80("_vim64_imovs");     std::string cmp_name81("_vim2K_imovs");
std::string cmp_name82("_vim64_imovu");     std::string cmp_name83("_vim2K_imovu");
std::string cmp_name84("_vim64_fadds");     std::string cmp_name85("_vim2K_fadds");
std::string cmp_name86("_vim64_fsubs");     std::string cmp_name87("_vim2K_fsubs");
std::string cmp_name88("_vim64_fabss");     std::string cmp_name89("_vim2K_fabss");
std::string cmp_name90("_vim64_fmaxs");     std::string cmp_name91("_vim2K_fmaxs");
std::string cmp_name92("_vim64_fmins");     std::string cmp_name93("_vim2K_fmins");
std::string cmp_name94("_vim64_fcpys");     std::string cmp_name95("_vim2K_fcpys");
std::string cmp_name96("_vim64_fslts");     std::string cmp_name97("_vim2K_fslts");
std::string cmp_name98("_vim64_fcmqs");     std::string cmp_name99("_vim2K_fcmqs");
std::string cmp_name100("_vim64_fdivs");     std::string cmp_name101("_vim2K_fdivs");
std::string cmp_name102("_vim64_fmuls");     std::string cmp_name103("_vim2K_fmuls");
std::string cmp_name104("_vim64_fcums");     std::string cmp_name105("_vim2K_fcums");
std::string cmp_name106("_vim64_fmovs");     std::string cmp_name107("_vim2K_fmovs");
std::string cmp_name108("_vim32_dadds");     std::string cmp_name109("_vim1K_dadds");
std::string cmp_name110("_vim32_dsubs");     std::string cmp_name111("_vim1K_dsubs");
std::string cmp_name112("_vim32_dabss");     std::string cmp_name113("_vim1K_dabss");
std::string cmp_name114("_vim32_dmaxs");     std::string cmp_name115("_vim1K_dmaxs");
std::string cmp_name116("_vim32_dmins");     std::string cmp_name117("_vim1K_dmins");
std::string cmp_name118("_vim32_dcpys");     std::string cmp_name119("_vim1K_dcpys");
std::string cmp_name120("_vim32_dslts");     std::string cmp_name121("_vim1K_dslts");
std::string cmp_name122("_vim32_dcmqs");     std::string cmp_name123("_vim1K_dcmqs");
std::string cmp_name124("_vim32_ddivs");     std::string cmp_name125("_vim1K_ddivs");
std::string cmp_name126("_vim32_dmuls");     std::string cmp_name127("_vim1K_dmuls");
std::string cmp_name128("_vim32_dcums");     std::string cmp_name129("_vim1K_dcums");
std::string cmp_name130("_vim32_dmovs");     std::string cmp_name131("_vim1K_dmovs");



const char *vima_int_alu_init[] = {	"_vim64_iadds",     "_vim2K_iadds",	//20
									"_vim64_iaddu",     "_vim2K_iaddu",
									"_vim64_isubs",     "_vim2K_isubs",
									"_vim64_isubu",     "_vim2K_isubu",
									"_vim64_iabss",     "_vim2K_iabss",
									"_vim64_imaxs",     "_vim2K_imaxs",
									"_vim64_imins",     "_vim2K_imins",
									"_vim64_icpys",     "_vim2K_icpys",
									"_vim64_icpyu",     "_vim2K_icpyu",
									"_vim64_iandu",     "_vim2K_iandu",
									"_vim64_iorun",     "_vim2K_iorun",
									"_vim64_ixoru",     "_vim2K_ixoru",
									"_vim64_inots",     "_vim2K_inots",
									"_vim64_islts",     "_vim2K_islts",
									"_vim64_isltu",     "_vim2K_isltu",
									"_vim64_icmqs",     "_vim2K_icmqs",
									"_vim64_icmqu",     "_vim2K_icmqu",
									"_vim64_isllu",     "_vim2K_isllu",
									"_vim64_isrlu",     "_vim2K_isrlu",
									"_vim64_isras",     "_vim2K_isras",//59,60
									"_vim64_imovs",     "_vim2K_imovs",//80
									"_vim64_imovu",     "_vim2K_imovu",//83
									"_vim64_fabss",     "_vim2K_fabss",//88,89
									"_vim64_fcpys",     "_vim2K_fcpys",//94,95
									"_vim64_fmovs",     "_vim2K_fmovs",//106,107
									"_vim32_dabss",     "_vim1K_dabss",//112,113
									"_vim32_dcpys",     "_vim1K_dcpys",//118,119
									"_vim32_dmovs",     "_vim1K_dmovs"//130,131
									};

const char *vima_fp_alu_init[] = {
							"_vim64_fadds",     "_vim2K_fadds",//84,85
							"_vim64_fsubs",     "_vim2K_fsubs",//86,87
							"_vim64_fmaxs",     "_vim2K_fmaxs",//90,91
							"_vim64_fmins",     "_vim2K_fmins",//92,93
							"_vim64_fslts",     "_vim2K_fslts",
							"_vim64_fcmqs",     "_vim2K_fcmqs",
							"_vim32_dadds",     "_vim1K_dadds",
							"_vim32_dsubs",     "_vim1K_dsubs",
							"_vim32_dmaxs",     "_vim1K_dmaxs",
							"_vim32_dmins",     "_vim1K_dmins",
							"_vim32_dslts",     "_vim1K_dslts",
							"_vim32_dcmqs",     "_vim1K_dcmqs"
						};

const char *vima_int_mul_init[] = {
							"_vim64_imuls",     "_vim2K_imuls",
							"_vim64_imulu",     "_vim2K_imulu",
							"_vim32_imuls",     "_vim1K_imuls",
							"_vim32_imulu",     "_vim1K_imulu"
							};

const char *vima_fp_mul_init[] = {
							"_vim64_fmuls",     "_vim2K_fmuls",
							"_vim32_dmuls",     "_vim1K_dmuls",
							};

const char *vima_int_div_init[] = {
							"_vim64_idivs",     "_vim2K_idivs",
							"_vim64_idivu",     "_vim2K_idivu",
							"_vim32_idivs",     "_vim1K_idivs",
							"_vim32_idivu",     "_vim1K_idivu"
							};

const char *vima_fp_div_init[] = {
							"_vim64_fdivs",     "_vim2K_fdivs",
							"_vim32_ddivs",     "_vim1K_ddivs",
							};

const char *vima_int_mla_init[] = {
							"_vim64_icumu",     "_vim2K_icumu",
							"_vim64_icums",     "_vim2K_icums",
							};
const char *vima_fp_mla_init[] = {
							"_vim64_fcums",     "_vim2K_fcums",
							"_vim32_dcums",     "_vim1K_dcums",
							};

const char *vima_init[] = {	"_vim64_iadds",     "_vim2K_iadds",
							"_vim64_iaddu",     "_vim2K_iaddu",
							"_vim64_isubs",     "_vim2K_isubs",
							"_vim64_isubu",     "_vim2K_isubu",
							"_vim64_iabss",     "_vim2K_iabss",
							"_vim64_imaxs",     "_vim2K_imaxs",
							"_vim64_imins",     "_vim2K_imins",
							"_vim64_icpys",     "_vim2K_icpys",
							"_vim64_icpyu",     "_vim2K_icpyu",
							"_vim64_iandu",     "_vim2K_iandu",
							"_vim64_iorun",     "_vim2K_iorun",
							"_vim64_ixoru",     "_vim2K_ixoru",
							"_vim64_inots",     "_vim2K_inots",
							"_vim64_islts",     "_vim2K_islts",
							"_vim64_isltu",     "_vim2K_isltu",
							"_vim64_icmqs",     "_vim2K_icmqs",
							"_vim64_icmqu",     "_vim2K_icmqu",
							"_vim64_isllu",     "_vim2K_isllu",
							"_vim64_isrlu",     "_vim2K_isrlu",
							"_vim64_isras",     "_vim2K_isras",
							"_vim64_idivs",     "_vim2K_idivs",
							"_vim64_idivu",     "_vim2K_idivu",
							"_vim32_idivs",     "_vim1K_idivs",
							"_vim32_idivu",     "_vim1K_idivu",
							"_vim64_imuls",     "_vim2K_imuls",
							"_vim64_imulu",     "_vim2K_imulu",
							"_vim32_imuls",     "_vim1K_imuls",
							"_vim32_imulu",     "_vim1K_imulu",
							"_vim64_icumu",     "_vim2K_icumu",
							"_vim64_icums",     "_vim2K_icums",
							"_vim64_imovs",     "_vim2K_imovs",
							"_vim64_imovu",     "_vim2K_imovu",
							"_vim64_fadds",     "_vim2K_fadds",
							"_vim64_fsubs",     "_vim2K_fsubs",
							"_vim64_fabss",     "_vim2K_fabss",
							"_vim64_fmaxs",     "_vim2K_fmaxs",
							"_vim64_fmins",     "_vim2K_fmins",
							"_vim64_fcpys",     "_vim2K_fcpys",
							"_vim64_fslts",     "_vim2K_fslts",
							"_vim64_fcmqs",     "_vim2K_fcmqs",
							"_vim64_fdivs",     "_vim2K_fdivs",
							"_vim64_fmuls",     "_vim2K_fmuls",
							"_vim64_fcums",     "_vim2K_fcums",
							"_vim64_fmovs",     "_vim2K_fmovs",
							"_vim32_dadds",     "_vim1K_dadds",
							"_vim32_dsubs",     "_vim1K_dsubs",
							"_vim32_dabss",     "_vim1K_dabss",
							"_vim32_dmaxs",     "_vim1K_dmaxs",
							"_vim32_dmins",     "_vim1K_dmins",
							"_vim32_dcpys",     "_vim1K_dcpys",
							"_vim32_dslts",     "_vim1K_dslts",
							"_vim32_dcmqs",     "_vim1K_dcmqs",
							"_vim32_ddivs",     "_vim1K_ddivs",
							"_vim32_dmuls",     "_vim1K_dmuls",
							"_vim32_dcums",     "_vim1K_dcums",
							"_vim32_dmovs",     "_vim1K_dmovs"
};

#define VIMA_INT_ALU_COUNT 56
#define VIMA_FP_ALU_COUNT 24 
#define VIMA_INT_MUL_COUNT 8
#define VIMA_FP_MUL_COUNT 4
#define VIMA_INT_DIV_COUNT 8
#define VIMA_FP_DIV_COUNT 4
#define VIMA_INT_MLA_COUNT 4
#define VIMA_FP_MLA_COUNT 4

const static std::vector<string> vima_inst_names(vima_init, vima_init + VIMA_INS_COUNT);
const static std::vector<string> vima_int_alu_names(vima_int_alu_init, vima_int_alu_init + VIMA_INT_ALU_COUNT);
const static std::vector<string> vima_fp_alu_names(vima_fp_alu_init, vima_fp_alu_init + VIMA_FP_ALU_COUNT);
const static std::vector<string> vima_int_mul_names(vima_int_mul_init, vima_int_mul_init + VIMA_INT_MUL_COUNT);
const static std::vector<string> vima_fp_mul_names(vima_fp_mul_init, vima_fp_mul_init + VIMA_FP_MUL_COUNT);
const static std::vector<string> vima_int_div_names(vima_int_div_init, vima_int_div_init + VIMA_INT_DIV_COUNT);
const static std::vector<string> vima_fp_div_names(vima_fp_div_init, vima_fp_div_init + VIMA_FP_DIV_COUNT);
const static std::vector<string> vima_int_mla_names(vima_int_mla_init, vima_int_mla_init + VIMA_INT_MLA_COUNT);
const static std::vector<string> vima_fp_mla_names(vima_fp_mla_init, vima_fp_mla_init + VIMA_INT_MLA_COUNT);

const char *vima_mnem_init[] = { "VIMA_IADDS_64VECTOR_32OPER",
								"VIMA_IADDS_2KVECTOR_32OPER",
								"VIMA_IADDU_64VECTOR_32OPER",
								"VIMA_IADDU_2KVECTOR_32OPER",
								"VIMA_ISUBS_64VECTOR_32OPER",
								"VIMA_ISUBS_2KVECTOR_32OPER",
								"VIMA_ISUBU_64VECTOR_32OPER",
								"VIMA_ISUBU_2KVECTOR_32OPER",
								"VIMA_IABSS_64VECTOR_32OPER",
								"VIMA_IABSS_2KVECTOR_32OPER",
								"VIMA_IMAXS_64VECTOR_32OPER",
								"VIMA_IMAXS_2KVECTOR_32OPER",
								"VIMA_IMINS_64VECTOR_32OPER",
								"VIMA_IMINS_2KVECTOR_32OPER",
								"VIMA_ICPYS_64VECTOR_32OPER",
								"VIMA_ICPYS_2KVECTOR_32OPER",
								"VIMA_ICPYU_64VECTOR_32OPER",
								"VIMA_ICPYU_2KVECTOR_32OPER",
								"VIMA_IANDU_64VECTOR_32OPER",
								"VIMA_IANDU_2KVECTOR_32OPER",
								"VIMA_IORUN_64VECTOR_32OPER",
								"VIMA_IORUN_2KVECTOR_32OPER",
								"VIMA_IXORU_64VECTOR_32OPER",
								"VIMA_IXORU_2KVECTOR_32OPER",
								"VIMA_INOTS_64VECTOR_32OPER",
								"VIMA_INOTS_2KVECTOR_32OPER",
								"VIMA_ISLTS_64VECTOR_32OPER",
								"VIMA_ISLTS_2KVECTOR_32OPER",
								"VIMA_ISLTU_64VECTOR_32OPER",
								"VIMA_ISLTU_2KVECTOR_32OPER",
								"VIMA_ICMQS_64VECTOR_32OPER",
								"VIMA_ICMQS_2KVECTOR_32OPER",
								"VIMA_ICMQU_64VECTOR_32OPER",
								"VIMA_ICMQU_2KVECTOR_32OPER",
								"VIMA_ISLLU_64VECTOR_32OPER",
								"VIMA_ISLLU_2KVECTOR_32OPER",
								"VIMA_ISRLU_64VECTOR_32OPER",
								"VIMA_ISRLU_2KVECTOR_32OPER",
								"VIMA_ISRAS_64VECTOR_32OPER",
								"VIMA_ISRAS_2KVECTOR_32OPER",
								"VIMA_IDIVS_64VECTOR_32OPER",
								"VIMA_IDIVS_2KVECTOR_32OPER",
								"VIMA_IDIVU_64VECTOR_32OPER",
								"VIMA_IDIVU_2KVECTOR_32OPER",
								"VIMA_IDIVS_32VECTOR_64OPER",
								"VIMA_IDIVS_1KVECTOR_32OPER",
								"VIMA_IDIVU_32VECTOR_64OPER",
								"VIMA_IDIVU_1KVECTOR_32OPER",
								"VIMA_IMULS_64VECTOR_32OPER",
								"VIMA_IMULS_2KVECTOR_32OPER",
								"VIMA_IMULU_64VECTOR_32OPER",
								"VIMA_IMULU_2KVECTOR_32OPER",
								"VIMA_IMULS_32VECTOR_64OPER",
								"VIMA_IMULS_1KVECTOR_32OPER",
								"VIMA_IMULU_32VECTOR_64OPER",
								"VIMA_IMULU_1KVECTOR_32OPER",
								"VIMA_IMADU_64VECTOR_32OPER",
								"VIMA_IMADU_2KVECTOR_32OPER",
								"VIMA_IMADS_64VECTOR_32OPER",
								"VIMA_IMADS_2KVECTOR_32OPER",
								"VIMA_IMOVS_64VECTOR_32OPER",
								"VIMA_IMOVS_2KVECTOR_32OPER",
								"VIMA_IMOVU_64VECTOR_32OPER",
								"VIMA_IMOVU_2KVECTOR_32OPER",
								"VIMA_FADDS_64VECTOR_32OPER",
								"VIMA_FADDS_2KVECTOR_32OPER",
								"VIMA_FSUBS_64VECTOR_32OPER",
								"VIMA_FSUBS_2KVECTOR_32OPER",
								"VIMA_FABSS_64VECTOR_32OPER",
								"VIMA_FABSS_2KVECTOR_32OPER",
								"VIMA_FMAXS_64VECTOR_32OPER",
								"VIMA_FMAXS_2KVECTOR_32OPER",
								"VIMA_FMINS_64VECTOR_32OPER",
								"VIMA_FMINS_2KVECTOR_32OPER",
								"VIMA_FCPYS_64VECTOR_32OPER",
								"VIMA_FCPYS_2KVECTOR_32OPER",
								"VIMA_FSLTS_64VECTOR_32OPER",
								"VIMA_FSLTS_2KVECTOR_32OPER",
								"VIMA_FCMQS_64VECTOR_32OPER",
								"VIMA_FCMQS_2KVECTOR_32OPER",
								"VIMA_FDIVS_64VECTOR_32OPER",
								"VIMA_FDIVS_2KVECTOR_32OPER",
								"VIMA_FMULS_64VECTOR_32OPER",
								"VIMA_FMULS_2KVECTOR_32OPER",
								"VIMA_FMADS_64VECTOR_32OPER",
								"VIMA_FMADS_2KVECTOR_32OPER",
								"VIMA_FMOVS_64VECTOR_32OPER",
								"VIMA_FMOVS_2KVECTOR_32OPER",
								"VIMA_DADDS_32VECTOR_64OPER",
								"VIMA_DADDS_1KVECTOR_32OPER",
								"VIMA_DSUBS_32VECTOR_64OPER",
								"VIMA_DSUBS_1KVECTOR_32OPER",
								"VIMA_DABSS_32VECTOR_64OPER",
								"VIMA_DABSS_1KVECTOR_32OPER",
								"VIMA_DMAXS_32VECTOR_64OPER",
								"VIMA_DMAXS_1KVECTOR_32OPER",
								"VIMA_DMINS_32VECTOR_64OPER",
								"VIMA_DMINS_1KVECTOR_32OPER",
								"VIMA_DCPYS_32VECTOR_64OPER",
								"VIMA_DCPYS_1KVECTOR_32OPER",
								"VIMA_DSLTS_32VECTOR_64OPER",
								"VIMA_DSLTS_1KVECTOR_32OPER",
								"VIMA_DCMQS_32VECTOR_64OPER",
								"VIMA_DCMQS_1KVECTOR_32OPER",
								"VIMA_DDIVS_32VECTOR_64OPER",
								"VIMA_DDIVS_1KVECTOR_32OPER",
								"VIMA_DMULS_32VECTOR_64OPER",
								"VIMA_DMULS_1KVECTOR_32OPER",
								"VIMA_DMADS_32VECTOR_64OPER",
								"VIMA_DMADS_1KVECTOR_32OPER",
								"VIMA_DMOVS_32VECTOR_64OPER",
								"VIMA_DMOVS_1KVECTOR_32OPER"
								};

const static std::vector<string> vima_mnem_names(vima_mnem_init, vima_mnem_init + VIMA_INS_COUNT);
// MIPS Instruction names
std::string cmp_name132("_mps32_add");
std::string cmp_name133("_mps32_addu");
std::string cmp_name134("_mps32_sub");
std::string cmp_name135("_mps32_subu");
std::string cmp_name136("_mps32_addi");
std::string cmp_name137("_mps32_addiu");
std::string cmp_name138("_mps32_and");
std::string cmp_name139("_mps32_nor");
std::string cmp_name140("_mps32_or");
std::string cmp_name141("_mps32_xor");
std::string cmp_name142("_mps32_andi");
std::string cmp_name143("_mps32_ori");
std::string cmp_name144("_mps32_xori");
std::string cmp_name145("_mps32_slt");
std::string cmp_name146("_mps32_sltu");
std::string cmp_name147("_mps32_slti");
std::string cmp_name148("_mps32_sltiu");
std::string cmp_name149("_mps32_sll");
std::string cmp_name150("_mps32_srl");
std::string cmp_name151("_mps32_sra");
std::string cmp_name152("_mps32_div");
std::string cmp_name153("_mps32_divu");
std::string cmp_name154("_mps32_mod");
std::string cmp_name155("_mps32_modu");
std::string cmp_name156("_mps32_mult");
std::string cmp_name157("_mps32_multu");
std::string cmp_name158("_mps64_mult");
std::string cmp_name159("_mps64_multu");

const char *mps_init[] = { "_mps32_add",
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

const static std::vector<string> mps_inst_names(mps_init, mps_init + MPS_INS_COUNT);

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

const static std::vector<string> mps_mnem_names(mps_mnem_init, mps_mnem_init + MPS_INS_COUNT);

//==============================================================================
// Header Functions
//==============================================================================
VOID initialize_intrinsics_hmc(data_instr hmc_x86_data[20]);
VOID initialize_intrinsics_vima(data_instr vim_x86_data[112]);
VOID initialize_intrinsics_mips(data_instr mps_x86_data[28]);
INT icheck_conditions(std::string rtn_name);
INT icheck_conditions_hmc(std::string rtn_name);
INT icheck_conditions_vima(std::string rtn_name);
INT icheck_conditions_mips(std::string rtn_name);
INT icheck_1parameter(std::string rtn_name);
INT icheck_2parameters(std::string rtn_name);
VOID write_dynamic_char(char *dyn_str, THREADID threadid);
VOID write_static_char(char *stat_str);
VOID hmc_write_memory_1param(ADDRINT read, UINT32 size, UINT32 bbl, THREADID threadid);
VOID hmc_write_memory_2param(ADDRINT read, ADDRINT write, UINT32 size, UINT32 bbl, THREADID threadid);
VOID hmc_write_memory_3param(ADDRINT read1, ADDRINT read2, ADDRINT write, UINT32 size, UINT32 bbl, THREADID threadid);
VOID arch_x86_set_data_instr(data_instr *hmc_x86_data, char const *rtn_name, char const *hmc_instr_name, char const *x86_instr_name, UINT32 instr_len);
VOID arch_x86_trace_instruction(RTN hmc_x86_rtn, data_instr *hmc_x86_data);
VOID synthetic_trace_generation(std::string rtn_name, data_instr hmc_x86_data[20], data_instr vim_x86_data[112], data_instr mps_x86_data[28], RTN rtn);
VOID specific_trace_generation(std::string rtn_name, const char *arch_name, int n_instr, data_instr *arch_x86_data, RTN rtn);
