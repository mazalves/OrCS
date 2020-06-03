// INTRINSICS EXTENSION HEADER
#include "../../../../utils/enumerations.hpp"
#include "opcodes.hpp"
#include <zlib.h>

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