// INTRINSICS EXTENSION HEADER
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
std::string cmp_name160("_vim64_ilmku");     std::string cmp_name161("_vim2K_ilmku");
std::string cmp_name162("_vim64_ilmks");     std::string cmp_name163("_vim2K_ilmks");
std::string cmp_name164("_vim64_flmks");     std::string cmp_name165("_vim2K_flmks");
std::string cmp_name166("_vim32_dlmks");     std::string cmp_name167("_vim1K_dlmks");
std::string cmp_name168("_vim64_ismku");     std::string cmp_name169("_vim2K_ismku");
std::string cmp_name170("_vim64_ismks");     std::string cmp_name171("_vim2K_ismks");
std::string cmp_name172("_vim64_fsmks");     std::string cmp_name173("_vim2K_fsmks");
std::string cmp_name174("_vim32_dsmks");     std::string cmp_name175("_vim1K_dsmks");
std::string cmp_name176("_vim64_irmku");     std::string cmp_name177("_vim2K_irmku");
std::string cmp_name178("_vim64_irmks");     std::string cmp_name179("_vim2K_irmks");
std::string cmp_name180("_vim64_frmks");     std::string cmp_name181("_vim2K_frmks");
std::string cmp_name182("_vim32_drmks");     std::string cmp_name183("_vim1K_drmks");
std::string cmp_name184("_vim64_ipmtu");     std::string cmp_name185("_vim2K_ipmtu");
std::string cmp_name186("_vim64_ipmts");     std::string cmp_name187("_vim2K_ipmts");
std::string cmp_name188("_vim64_fpmts");     std::string cmp_name189("_vim2K_fpmts");
std::string cmp_name190("_vim32_dpmts");     std::string cmp_name191("_vim1K_dpmts");
std::string cmp_name192("_vim64_imodu");     std::string cmp_name193("_vim2K_imodu");
std::string cmp_name194("_vim64_imods");     std::string cmp_name195("_vim2K_imods");
std::string cmp_name196("_vim64_igtru");     std::string cmp_name197("_vim2K_igtru");
std::string cmp_name198("_vim64_igtrs");     std::string cmp_name199("_vim2K_igtrs");
std::string cmp_name200("_vim64_fgtrs");     std::string cmp_name201("_vim2K_fgtrs");
std::string cmp_name202("_vim32_dgtrs");     std::string cmp_name203("_vim1K_dgtrs");
std::string cmp_name204("_vim64_isctu");     std::string cmp_name205("_vim2K_isctu");
std::string cmp_name206("_vim64_iscts");     std::string cmp_name207("_vim2K_iscts");
std::string cmp_name208("_vim64_fscts");     std::string cmp_name209("_vim2K_fscts");
std::string cmp_name210("_vim32_dscts");     std::string cmp_name211("_vim1K_dscts");
std::string cmp_name212("_vim64_idptu");     std::string cmp_name213("_vim2K_idptu");
std::string cmp_name214("_vim64_idpts");     std::string cmp_name215("_vim2K_idpts");
std::string cmp_name216("_vim64_iscou");     std::string cmp_name217("_vim2K_iscou");
std::string cmp_name218("_vim64_iscos");     std::string cmp_name219("_vim2K_iscos");

std::string cmp_name220("_vim128_iadds");   std::string cmp_name221("_vim1K_iadds");
std::string cmp_name222("_vim128_iaddu");   std::string cmp_name223("_vim1K_iaddu");
std::string cmp_name224("_vim128_isubs");   std::string cmp_name225("_vim1K_isubs");
std::string cmp_name226("_vim128_isubu");   std::string cmp_name227("_vim1K_isubu");
std::string cmp_name228("_vim128_iabss");   std::string cmp_name229("_vim1K_iabss");
std::string cmp_name230("_vim128_imaxs");   std::string cmp_name231("_vim1K_imaxs");
std::string cmp_name232("_vim128_imins");   std::string cmp_name233("_vim1K_imins");
std::string cmp_name234("_vim128_icpys");   std::string cmp_name235("_vim1K_icpys");
std::string cmp_name236("_vim128_icpyu");   std::string cmp_name237("_vim1K_icpyu");
std::string cmp_name238("_vim128_iandu");   std::string cmp_name239("_vim1K_iandu");
std::string cmp_name240("_vim128_iorun");   std::string cmp_name241("_vim1K_iorun");
std::string cmp_name242("_vim128_ixoru");   std::string cmp_name243("_vim1K_ixoru");
std::string cmp_name244("_vim128_inots");   std::string cmp_name245("_vim1K_inots");
std::string cmp_name246("_vim128_islts");   std::string cmp_name247("_vim1K_islts");
std::string cmp_name248("_vim128_isltu");   std::string cmp_name249("_vim1K_isltu");
std::string cmp_name250("_vim128_icmqs");   std::string cmp_name251("_vim1K_icmqs");
std::string cmp_name252("_vim128_icmqu");   std::string cmp_name253("_vim1K_icmqu");
std::string cmp_name254("_vim128_isllu");   std::string cmp_name255("_vim1K_isllu");
std::string cmp_name256("_vim128_isrlu");   std::string cmp_name257("_vim1K_isrlu");
std::string cmp_name258("_vim128_isras");   std::string cmp_name259("_vim1K_isras");
std::string cmp_name260("_vim128_idivs");   std::string cmp_name261("_vim1K_idivs");
std::string cmp_name262("_vim128_idivu");   std::string cmp_name263("_vim1K_idivu");
std::string cmp_name264("_vim64_idivs");    std::string cmp_name265("_vim512_idivs");
std::string cmp_name266("_vim64_idivu");    std::string cmp_name267("_vim512_idivu");
std::string cmp_name268("_vim128_imuls");   std::string cmp_name269("_vim1K_imuls");
std::string cmp_name270("_vim128_imulu");   std::string cmp_name271("_vim1K_imulu");
std::string cmp_name272("_vim64_imuls");    std::string cmp_name273("_vim512_imuls");
std::string cmp_name274("_vim64_imulu");    std::string cmp_name275("_vim512_imulu");
std::string cmp_name276("_vim128_icumu");   std::string cmp_name277("_vim1K_icumu");
std::string cmp_name278("_vim128_icums");   std::string cmp_name279("_vim1K_icums");
std::string cmp_name280("_vim128_imovs");   std::string cmp_name281("_vim1K_imovs");
std::string cmp_name282("_vim128_imovu");   std::string cmp_name283("_vim1K_imovu");
std::string cmp_name284("_vim128_fadds");   std::string cmp_name285("_vim1K_fadds");
std::string cmp_name286("_vim128_fsubs");    std::string cmp_name287("_vim1K_fsubs");
std::string cmp_name288("_vim128_fabss");   std::string cmp_name289("_vim1K_fabss");
std::string cmp_name290("_vim128_fmaxs");   std::string cmp_name291("_vim1K_fmaxs");
std::string cmp_name292("_vim128_fmins");   std::string cmp_name293("_vim1K_fmins");
std::string cmp_name294("_vim128_fcpys");    std::string cmp_name295("_vim1K_fcpys");
std::string cmp_name296("_vim128_fslts");   std::string cmp_name297("_vim1K_fslts");
std::string cmp_name298("_vim128_fcmqs");   std::string cmp_name299("_vim1K_fcmqs");
std::string cmp_name300("_vim128_fdivs");   std::string cmp_name301("_vim1K_fdivs");
std::string cmp_name302("_vim128_fmuls");   std::string cmp_name303("_vim1K_fmuls");
std::string cmp_name304("_vim128_fcums");   std::string cmp_name305("_vim1K_fcums");
std::string cmp_name306("_vim128_fmovs");   std::string cmp_name307("_vim1K_fmovs");
std::string cmp_name308("_vim64_dadds");    std::string cmp_name309("_vim512_dadds");
std::string cmp_name310("_vim64_dsubs");    std::string cmp_name311("_vim512_dsubs");
std::string cmp_name312("_vim64_dabss");    std::string cmp_name313("_vim512_dabss");
std::string cmp_name314("_vim64_dmaxs");    std::string cmp_name315("_vim512_dmaxs");
std::string cmp_name316("_vim64_dmins");    std::string cmp_name317("_vim512_dmins");
std::string cmp_name318("_vim64_dcpys");    std::string cmp_name319("_vim512_dcpys");
std::string cmp_name320("_vim64_dslts");    std::string cmp_name321("_vim512_dslts");
std::string cmp_name322("_vim64_dcmqs");    std::string cmp_name323("_vim512_dcmqs");
std::string cmp_name324("_vim64_ddivs");    std::string cmp_name325("_vim512_ddivs");
std::string cmp_name326("_vim64_dmuls");    std::string cmp_name327("_vim512_dmuls");
std::string cmp_name328("_vim64_dcums");    std::string cmp_name330("_vim512_dcums");
std::string cmp_name331("_vim64_dmovs");    std::string cmp_name332("_vim512_dmovs");
std::string cmp_name333("_vim128_ilmku");   std::string cmp_name334("_vim1K_ilmku");
std::string cmp_name335("_vim128_ilmks");   std::string cmp_name336("_vim1K_ilmks");
std::string cmp_name337("_vim128_flmks");   std::string cmp_name338("_vim1K_flmks");
std::string cmp_name339("_vim128_dlmks");   std::string cmp_name340("_vim1K_dlmks");
std::string cmp_name341("_vim128_ismku");   std::string cmp_name342("_vim1K_ismku");
std::string cmp_name343("_vim128_ismks");   std::string cmp_name344("_vim1K_ismks");
std::string cmp_name345("_vim128_fsmks");   std::string cmp_name346("_vim1K_fsmks");
std::string cmp_name347("_vim128_dsmks");   std::string cmp_name348("_vim1K_dsmks");
std::string cmp_name349("_vim128_irmku");   std::string cmp_name350("_vim1K_irmku");
std::string cmp_name351("_vim128_irmk");    std::string cmp_name352("_vim1K_irmk");
std::string cmp_name353("_vim128_frmks");   std::string cmp_name354("_vim1K_frmks");
std::string cmp_name355("_vim128_drmks");   std::string cmp_name356("_vim1K_drmks");
std::string cmp_name357("_vim128_ipmtu");   std::string cmp_name358("_vim1K_ipmtu");
std::string cmp_name359("_vim128_ipmts");   std::string cmp_name360("_vim1K_ipmts");
std::string cmp_name361("_vim128_fpmts");   std::string cmp_name362("_vim1K_fpmts");
std::string cmp_name363("_vim128_dpmts");   std::string cmp_name364("_vim1K_dpmts");
std::string cmp_name365("_vim128_imodu");   std::string cmp_name366("_vim1K_imodu");
std::string cmp_name367("_vim128_imods");   std::string cmp_name368("_vim1K_imods");
std::string cmp_name369("_vim128_igtru");   std::string cmp_name370("_vim1K_igtru");
std::string cmp_name371("_vim128_igtrs");   std::string cmp_name372("_vim1K_igtrs");
std::string cmp_name373("_vim128_fgtrs");   std::string cmp_name374("_vim1K_fgtrs");
std::string cmp_name376("_vim128_dgtrs");   std::string cmp_name377("_vim1K_dgtrs");
std::string cmp_name378("_vim128_isctu");   std::string cmp_name379("_vim1K_isctu");
std::string cmp_name380("_vim128_iscts");   std::string cmp_name381("_vim1K_iscts");
std::string cmp_name382("_vim128_fscts");   std::string cmp_name383("_vim1K_fscts");
std::string cmp_name384("_vim128_dscts");   std::string cmp_name385("_vim1K_dscts");
std::string cmp_name386("_vim128_idptu");   std::string cmp_name387("_vim1K_idptu");
std::string cmp_name388("_vim128_idpts");   std::string cmp_name389("_vim1K_idpts");
std::string cmp_name390("_vim128_iscou");   std::string cmp_name391("_vim1K_iscou");
std::string cmp_name392("_vim128_iscos");   std::string cmp_name393("_vim1K_iscos");
std::string cmp_name394("_vim256_iadds");   std::string cmp_name395("_vim512_iadds");
std::string cmp_name396("_vim256_iaddu");   std::string cmp_name397("_vim512_iaddu");
std::string cmp_name398("_vim256_isubs");   std::string cmp_name399("_vim512_isubs");
std::string cmp_name400("_vim256_isubu");   std::string cmp_name401("_vim512_isubu");
std::string cmp_name402("_vim256_iabss");   std::string cmp_name403("_vim512_iabss");
std::string cmp_name404("_vim256_imaxs");   std::string cmp_name405("_vim512_imaxs");
std::string cmp_name406("_vim256_imins");   std::string cmp_name407("_vim512_imins");
std::string cmp_name408("_vim256_icpys");   std::string cmp_name409("_vim512_icpys");
std::string cmp_name410("_vim256_icpyu");   std::string cmp_name411("_vim512_icpyu");
std::string cmp_name412("_vim256_iandu");   std::string cmp_name413("_vim512_iandu");
std::string cmp_name414("_vim256_iorun");   std::string cmp_name415("_vim512_iorun");
std::string cmp_name416("_vim256_ixoru");   std::string cmp_name417("_vim512_ixoru");
std::string cmp_name418("_vim256_inots");   std::string cmp_name419("_vim512_inots");
std::string cmp_name420("_vim256_islts");   std::string cmp_name421("_vim512_islts");
std::string cmp_name422("_vim256_isltu");   std::string cmp_name423("_vim512_isltu");
std::string cmp_name424("_vim256_icmqs");   std::string cmp_name425("_vim512_icmqs");
std::string cmp_name426("_vim256_icmqu");   std::string cmp_name427("_vim512_icmqu");
std::string cmp_name428("_vim256_isllu");   std::string cmp_name429("_vim512_isllu");
std::string cmp_name430("_vim256_isrlu");   std::string cmp_name431("_vim512_isrlu");
std::string cmp_name432("_vim256_isras");   std::string cmp_name433("_vim512_isras");
std::string cmp_name434("_vim256_idivs");   std::string cmp_name435("_vim512_idivs");
std::string cmp_name436("_vim256_idivu");   std::string cmp_name437("_vim512_idivu");
std::string cmp_name438("_vim128_idivs");   std::string cmp_name439("_vim256_idivs");
std::string cmp_name440("_vim128_idivu");   std::string cmp_name441("_vim256_idivu");
std::string cmp_name442("_vim256_imuls");   std::string cmp_name443("_vim512_imuls");
std::string cmp_name444("_vim256_imulu");   std::string cmp_name445("_vim512_imulu");
std::string cmp_name446("_vim128_imuls");   std::string cmp_name447("_vim256_imuls");
std::string cmp_name448("_vim128_imulu");   std::string cmp_name449("_vim256_imulu");
std::string cmp_name450("_vim256_icumu");   std::string cmp_name451("_vim512_icumu");
std::string cmp_name452("_vim256_icums");   std::string cmp_name453("_vim512_icums");
std::string cmp_name454("_vim256_imovs");   std::string cmp_name455("_vim512_imovs");
std::string cmp_name456("_vim256_imovu");   std::string cmp_name457("_vim512_imovu");
std::string cmp_name458("_vim256_fadds");   std::string cmp_name459("_vim512_fadds");
std::string cmp_name460("_vim256_fsubs");   std::string cmp_name461("_vim512_fsubs");
std::string cmp_name462("_vim256_fabss");   std::string cmp_name463("_vim512_fabss");
std::string cmp_name464("_vim256_fmaxs");   std::string cmp_name465("_vim512_fmaxs");
std::string cmp_name466("_vim256_fmins");   std::string cmp_name467("_vim512_fmins");
std::string cmp_name468("_vim256_fcpys");   std::string cmp_name469("_vim512_fcpys");
std::string cmp_name470("_vim256_fslts");   std::string cmp_name471("_vim512_fslts");
std::string cmp_name472("_vim256_fcmqs");   std::string cmp_name473("_vim512_fcmqs");
std::string cmp_name474("_vim256_fdivs");   std::string cmp_name475("_vim512_fdivs");
std::string cmp_name476("_vim256_fmuls");   std::string cmp_name477("_vim512_fmuls");
std::string cmp_name478("_vim256_fcums");   std::string cmp_name479("_vim512_fcums");
std::string cmp_name480("_vim256_fmovs");   std::string cmp_name481("_vim512_fmovs");
std::string cmp_name482("_vim128_dadds");   std::string cmp_name483("_vim256_dadds");
std::string cmp_name484("_vim128_dsubs");   std::string cmp_name485("_vim256_dsubs");
std::string cmp_name486("_vim128_dabss");   std::string cmp_name487("_vim256_dabss");
std::string cmp_name488("_vim128_dmaxs");   std::string cmp_name489("_vim256_dmaxs");
std::string cmp_name490("_vim128_dmins");   std::string cmp_name491("_vim256_dmins");
std::string cmp_name492("_vim128_dcpys");   std::string cmp_name493("_vim256_dcpys");
std::string cmp_name494("_vim128_dslts");   std::string cmp_name495("_vim256_dslts");
std::string cmp_name496("_vim128_dcmqs");   std::string cmp_name497("_vim256_dcmqs");
std::string cmp_name498("_vim128_ddivs");   std::string cmp_name499("_vim256_ddivs");
std::string cmp_name500("_vim128_dmuls");   std::string cmp_name501("_vim256_dmuls");
std::string cmp_name502("_vim128_dcums");   std::string cmp_name503("_vim256_dcums");
std::string cmp_name504("_vim128_dmovs");   std::string cmp_name505("_vim256_dmovs");
std::string cmp_name506("_vim256_ilmku");   std::string cmp_name507("_vim512_ilmku");
std::string cmp_name508("_vim256_ilmks");   std::string cmp_name509("_vim512_ilmks");
std::string cmp_name510("_vim256_flmks");   std::string cmp_name511("_vim512_flmks");
std::string cmp_name512("_vim256_dlmks");   std::string cmp_name513("_vim512_dlmks");
std::string cmp_name514("_vim256_ismku");   std::string cmp_name515("_vim512_ismku");
std::string cmp_name516("_vim256_ismks");   std::string cmp_name517("_vim512_ismks");
std::string cmp_name518("_vim256_fsmks");   std::string cmp_name519("_vim512_fsmks");
std::string cmp_name520("_vim256_dsmks");   std::string cmp_name521("_vim512_dsmks");
std::string cmp_name522("_vim256_irmku");   std::string cmp_name523("_vim512_irmku");
std::string cmp_name524("_vim256_irmks");   std::string cmp_name525("_vim512_irmks");
std::string cmp_name526("_vim256_frmks");   std::string cmp_name527("_vim512_frmks");
std::string cmp_name528("_vim256_drmks");   std::string cmp_name529("_vim512_drmks");
std::string cmp_name530("_vim256_ipmtu");   std::string cmp_name531("_vim512_ipmtu");
std::string cmp_name532("_vim256_ipmts");   std::string cmp_name533("_vim512_ipmts");
std::string cmp_name534("_vim256_fpmts");   std::string cmp_name535("_vim512_fpmts");
std::string cmp_name536("_vim256_dpmts");   std::string cmp_name537("_vim512_dpmts");
std::string cmp_name538("_vim256_imodu");   std::string cmp_name539("_vim512_imodu");
std::string cmp_name540("_vim256_imods");   std::string cmp_name541("_vim512_imods");
std::string cmp_name542("_vim256_igtru");   std::string cmp_name543("_vim512_igtru");
std::string cmp_name544("_vim256_igtrs");   std::string cmp_name545("_vim512_igtrs");
std::string cmp_name546("_vim256_fgtrs");   std::string cmp_name547("_vim512_fgtrs");
std::string cmp_name548("_vim256_dgtrs");   std::string cmp_name549("_vim512_dgtrs");
std::string cmp_name550("_vim256_isctu");   std::string cmp_name551("_vim512_isctu");
std::string cmp_name552("_vim256_iscts");   std::string cmp_name553("_vim512_iscts");
std::string cmp_name554("_vim256_fscts");   std::string cmp_name555("_vim512_fscts");
std::string cmp_name556("_vim256_dscts");   std::string cmp_name557("_vim512_dscts");
std::string cmp_name558("_vim256_idptu");   std::string cmp_name559("_vim512_idptu");
std::string cmp_name560("_vim256_idpts");   std::string cmp_name561("_vim512_idpts");
std::string cmp_name562("_vim256_iscou");   std::string cmp_name563("_vim512_iscou");
std::string cmp_name564("_vim256_iscos");   std::string cmp_name565("_vim512_iscos");

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
VOID initialize_intrinsics_vima(data_instr vim_x86_data[114]);
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
VOID synthetic_trace_generation(std::string rtn_name, data_instr hmc_x86_data[20], data_instr vim_x86_data[114], data_instr mps_x86_data[28], RTN rtn);
VOID specific_trace_generation(std::string rtn_name, const char *arch_name, int n_instr, data_instr *arch_x86_data, RTN rtn);