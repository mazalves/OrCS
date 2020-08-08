#include "./../simulator.hpp"

// =====================================================================
processor_t::processor_t()
{
	this->stall_full_FetchBuffer = 0;
    this->stall_wrong_branch = 0;
	//=============
	//Statistics Decode
	//=============
    this->stall_full_DecodeBuffer = 0;
	//=============
	//Statistics Rename
	//=============
	this->registerWrite = 0;
	this->stall_full_MOB_Read = 0;
	this->stall_full_MOB_Write = 0;
	this->stall_full_ROB = 0;
	//=============
	//Statistics Dispatch
	//=============
	this->stall_empty_RS = 0;
	//=============
	//Statistics Execute
	//=============
	this->stat_disambiguation_read_false_positive = 0;
	this->stat_disambiguation_write_false_positive = 0;
	this->stat_address_to_address = 0;
	this->times_reach_parallel_requests_read = 0;
	this->times_reach_parallel_requests_write = 0;
	this->instruction_per_cycle = 0;
	this->ended_cycle = 0;
	this->mem_req_wait_cycles = 0;
	this->core_ram_request_wait_cycles = 0;
	this->core_ram_requests = 0;
	//=============
	//Statistics Commit
	//=============
	this->stat_inst_int_alu_completed = 0;
	this->stat_inst_mul_alu_completed = 0;
	this->stat_inst_div_alu_completed = 0;
	this->stat_inst_int_fp_completed = 0;
	this->stat_inst_mul_fp_completed = 0;
	this->stat_inst_div_fp_completed = 0;
	this->stat_inst_hive_completed = 0;
	this->stat_inst_vima_completed = 0;
	this->stat_inst_nop_completed = 0;
	this->stat_inst_load_completed = 0;
	this->stat_inst_store_completed = 0;
	this->stat_inst_branch_completed = 0;
	this->stat_inst_other_completed = 0;

	this->HAS_HIVE = 0;
	this->HAS_VIMA = 0;

	this->FETCH_WIDTH = 0;
	this->DECODE_WIDTH = 0;
	this->RENAME_WIDTH = 0;
	this->DISPATCH_WIDTH = 0;
	this->EXECUTE_WIDTH = 0;
	this->COMMIT_WIDTH = 0;

	this->FETCH_LATENCY = 0;
	this->DECODE_LATENCY = 0;
	this->RENAME_LATENCY = 0;
	this->DISPATCH_LATENCY = 0;
	this->EXECUTE_LATENCY = 0;
	this->COMMIT_LATENCY = 0;

	this->LATENCY_INTEGER_ALU = 0;
	this->WAIT_NEXT_INT_ALU = 0;
	this->INTEGER_ALU = 0;
	// INTEGER MUL
	this->LATENCY_INTEGER_MUL = 0;
	this->WAIT_NEXT_INT_MUL = 0;
	this->INTEGER_MUL = 0;
	// INTEGER DIV
	this->LATENCY_INTEGER_DIV = 0;
	this->WAIT_NEXT_INT_DIV = 0;
	this->INTEGER_DIV = 0;

	this->QTDE_INTEGER_FU = 0;

	//FP ULAS LATENCY 
	// FLOATING POINT DIV
	this->LATENCY_FP_DIV = 0;
	this->WAIT_NEXT_FP_DIV = 0;
	this->FP_DIV = 0;
	// FLOATING POINT MUL
	this->LATENCY_FP_MUL = 0;
	this->WAIT_NEXT_FP_MUL = 0;
	this->FP_MUL = 0;
	// FLOATING POINT ALU
	this->LATENCY_FP_ALU = 0;
	this->WAIT_NEXT_FP_ALU = 0;
	this->FP_ALU = 0;

	this->QTDE_FP_FU = 0;

	this->PARALLEL_LOADS = 0;
	this->PARALLEL_STORES = 0;

	// ======================
	///UNIFIED FUS

	// PROCESSOR BUFFERS SIZE
	this->FETCH_BUFFER = 0;
	this->DECODE_BUFFER = 0;
	this->RAT_SIZE = 0;
	this->ROB_SIZE = 0;
	this->UNIFIED_RS = 0;
	//MOB
	this->MOB_READ = 0;
	this->MOB_WRITE = 0;
	this->MOB_HIVE = 0;
	this->MOB_VIMA = 0;
	// =====================

	// =====================
	// MEMORY FU
	// =====================
	// Load Units
	this->LOAD_UNIT = 0;
	this->WAIT_NEXT_MEM_LOAD = 0;
	this->LATENCY_MEM_LOAD = 0;
	// Store Units
	this->STORE_UNIT = 0;
	this->WAIT_NEXT_MEM_STORE = 0;
	this->LATENCY_MEM_STORE = 0;
	// HIVE Units
	this->HIVE_UNIT = 0;
	this->WAIT_NEXT_MEM_HIVE = 0;
	this->LATENCY_MEM_HIVE = 0;
	// VIMA Units
	this->VIMA_UNIT = 0;
	this->WAIT_NEXT_MEM_VIMA = 0;
	this->LATENCY_MEM_VIMA = 0;

	this->QTDE_MEMORY_FU = 0;

	//this->KILO = 1024 = 0;
	//this->MEGA = KILO*KILO = 0;

	this->LINE_SIZE = 0;
	this->DATA_CACHES = 0;
	this->DATA_SIZE = NULL;
	this->DATA_ASSOCIATIVITY = NULL;
	this->DATA_LATENCY = NULL;
	this->DATA_SETS = NULL;
	this->DATA_LEVEL = NULL;
	// I$
	this->INST_SIZE = NULL;
	this->INST_ASSOCIATIVITY = NULL;
	this->INST_LATENCY = NULL;
	this->INST_SETS = NULL;
	this->INST_LEVEL = NULL;
	// I$
	this->INSTRUCTION_CACHES = 0;
	this->RAM_LATENCY = 0;
	this->PARALLEL_LIM_ACTIVE = 0;
	this->MAX_PARALLEL_REQUESTS_CORE = 0;

	this->PREFETCHER_ACTIVE = 0;

	this->DISAMBIGUATION_ENABLED = 0;

	this->DEBUG = 0;
	this->PROCESSOR_DEBUG = 0;
	this->FETCH_DEBUG = 0;
	this->DECODE_DEBUG = 0;
	this->RENAME_DEBUG = 0;
	this->DISPATCH_DEBUG = 0;
	this->EXECUTE_DEBUG = 0;
	this->MOB_DEBUG = 0;
	this->PRINT_MOB = 0;
	this->PRINT_ROB = 0;
	this->HIVE_DEBUG = 0;
	this->VIMA_DEBUG = 0;
	this->COMMIT_DEBUG = 0;
	this->MSHR_DEBUG = 0;
	this->MULTICORE_DEBUG = 0;

	this->WAIT_CYCLE = 0;

	this->memory_read_executed = 0;
	this->memory_write_executed = 0;
	this->memory_vima_executed = 0;
	this->memory_hive_executed = 0;

	this->robUsed = 0;
	//Setting Pointers to NULL
	// ========OLDEST MEMORY OPERATIONS POINTER======
	this->oldest_read_to_send = NULL;
	this->oldest_write_to_send = NULL;
	this->oldest_hive_to_send = NULL;
	this->oldest_vima_to_send = NULL;
	// ========MOB======
	this->memory_order_buffer_read = NULL;
	this->memory_order_buffer_write = NULL;
	this->memory_order_buffer_hive = NULL;
	this->memory_order_buffer_vima = NULL;

	this->memory_order_buffer_read_start = 0;
	this->memory_order_buffer_read_used = 0;
	this->memory_order_buffer_read_end = 0;
	this->memory_order_buffer_write_start = 0;
	this->memory_order_buffer_write_used = 0;
	this->memory_order_buffer_write_end = 0;
	this->memory_order_buffer_hive_start = 0;
	this->memory_order_buffer_hive_used = 0;
	this->memory_order_buffer_hive_end = 0;
	this->memory_order_buffer_vima_start = 0;
	this->memory_order_buffer_vima_used = 0;
	this->memory_order_buffer_vima_end = 0;
	//=========DESAMBIGUATION ============
	this->disambiguator = NULL;
	// ==========RAT======
	this->register_alias_table = NULL;
	// ==========ROB========
	this->reorderBuffer = NULL;
	// ======FUs=========
	// Integer FUs
	this->fu_int_alu = NULL;
	this->fu_int_mul = NULL;
	this->fu_int_div = NULL;
	// Floating Points FUs
	this->fu_fp_alu = NULL;
	this->fu_fp_mul = NULL;
	this->fu_fp_div = NULL;
	// Memory FUs
	this->fu_mem_load = NULL;
	this->fu_mem_store = NULL;
	this->fu_mem_hive = NULL;
	this->fu_mem_vima = NULL;
}
processor_t::~processor_t()
{
	for (size_t i = 0; i < MOB_READ; i++) {
		utils_t::template_delete_array<memory_order_buffer_line_t *>(this->memory_order_buffer_read[i].mem_deps_ptr_array);
	}
	for (size_t i = 0; i < MOB_WRITE; i++) {
		utils_t::template_delete_array<memory_order_buffer_line_t *>(this->memory_order_buffer_write[i].mem_deps_ptr_array);
	}
	if (this->get_HAS_HIVE()) {
		for (size_t i = 0; i < MOB_HIVE; i++) {
			utils_t::template_delete_array<memory_order_buffer_line_t *>(this->memory_order_buffer_hive[i].mem_deps_ptr_array);
		}
	}
	if (this->get_HAS_VIMA()) {
		for (size_t i = 0; i < MOB_VIMA; i++) {
			utils_t::template_delete_array<memory_order_buffer_line_t *>(this->memory_order_buffer_vima[i].mem_deps_ptr_array);
		}
	}
	//Memory structures
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_read);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_write);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_hive);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_vima);
	utils_t::template_delete_variable<desambiguation_t>(this->disambiguator);
	//auxiliar var to maintain status oldest instruction
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_read_to_send);
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_write_to_send);
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_hive_to_send);
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_vima_to_send);

	//deleting deps array rob
	for (size_t i = 0; i < ROB_SIZE; i++)
	{
		utils_t::template_delete_array<reorder_buffer_line_t>(this->reorderBuffer[i].reg_deps_ptr_array[0]);
	}
	// deleting rob
	utils_t::template_delete_array<reorder_buffer_line_t>(this->reorderBuffer);
	//delete RAT
	utils_t::template_delete_array<reorder_buffer_line_t *>(this->register_alias_table);
	//deleting fus int
	utils_t::template_delete_array<uint64_t>(this->fu_int_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_int_mul);
	utils_t::template_delete_array<uint64_t>(this->fu_int_div);
	//deleting fus fp
	utils_t::template_delete_array<uint64_t>(this->fu_fp_alu);
	utils_t::template_delete_array<uint64_t>(this->fu_fp_mul);
	utils_t::template_delete_array<uint64_t>(this->fu_fp_div);
	//deleting fus memory
	utils_t::template_delete_array<uint64_t>(this->fu_mem_load);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_store);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_hive);
	utils_t::template_delete_array<uint64_t>(this->fu_mem_vima);
	// =====================================================================
}

uint32_t processor_t::get_cache_list(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs, uint32_t *ASSOCIATIVITY, uint32_t *LATENCY, uint32_t *SIZE, uint32_t *SETS, uint32_t *LEVEL) {
	const char *string_cache_type;
    if (cache_type == 0) {
        string_cache_type = "INSTRUCTION";
    } else {
        string_cache_type = "DATA";
    }

	// Get the list of caches
    libconfig::Setting &cfg_caches = cfg_cache_defs[string_cache_type];
    uint32_t N_CACHES = cfg_caches.getLength();

	ASSOCIATIVITY = new uint32_t[N_CACHES]();
	LATENCY = new uint32_t[N_CACHES]();
	SIZE = new uint32_t[N_CACHES]();
	SETS = new uint32_t[N_CACHES]();
	LEVEL = new uint32_t[N_CACHES]();

	// Get information of each instruction cache
	for (uint32_t i = 0; i < N_CACHES; i++) {
		libconfig::Setting &cfg_cache = cfg_caches[i];
		try {
			ASSOCIATIVITY[i] = cfg_cache["ASSOCIATIVITY"];
			LATENCY[i] = cfg_cache["LATENCY"];
			SIZE[i] = cfg_cache["SIZE"];
			SETS[i] = ((SIZE[i]/ LINE_SIZE) / ASSOCIATIVITY[i]);
			LEVEL[i] = cfg_cache["LEVEL"];
		} catch (libconfig::SettingNotFoundException const &nfex) {
            ERROR_PRINTF("MISSING CACHE PARAMETERS");
        } catch (libconfig::SettingTypeException const &tex) {
            ERROR_PRINTF("WRONG TYPE CACHE PARAMETERS");
        }
	}

	delete[] ASSOCIATIVITY;
	delete[] LATENCY;
	delete[] SIZE;
	delete[] SETS;
	delete[] LEVEL;
	return N_CACHES;
}

// =====================================================================
void processor_t::allocate() {
	libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
	
	// Processor defaults
	libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];

	if (cfg_root.exists("VIMA_CONTROLLER")) set_HAS_VIMA (1);
	else set_HAS_VIMA (0);

	set_HAS_HIVE (cfg_processor["HAS_HIVE"]);
	
	set_FETCH_WIDTH (cfg_processor["FETCH_WIDTH"]);
	set_DECODE_WIDTH (cfg_processor["DECODE_WIDTH"]);
	set_RENAME_WIDTH (cfg_processor["RENAME_WIDTH"]);
	set_DISPATCH_WIDTH (cfg_processor["DISPATCH_WIDTH"]);
	set_EXECUTE_WIDTH (cfg_processor["EXECUTE_WIDTH"]);
	set_COMMIT_WIDTH (cfg_processor["COMMIT_WIDTH"]);

	set_FETCH_LATENCY (cfg_processor["FETCH_LATENCY"]);
	set_DECODE_LATENCY (cfg_processor["DECODE_LATENCY"]);
	set_RENAME_LATENCY (cfg_processor["RENAME_LATENCY"]);
	set_DISPATCH_LATENCY (cfg_processor["DISPATCH_LATENCY"]);
	set_EXECUTE_LATENCY (cfg_processor["EXECUTE_LATENCY"]);
	set_COMMIT_LATENCY (cfg_processor["COMMIT_LATENCY"]);

	set_LATENCY_INTEGER_ALU (cfg_processor["LATENCY_INTEGER_ALU"]);
	set_WAIT_NEXT_INT_ALU (cfg_processor["WAIT_NEXT_INT_ALU"]);
	set_INTEGER_ALU (cfg_processor["INTEGER_ALU"]);

	set_LATENCY_INTEGER_MUL (cfg_processor["LATENCY_INTEGER_MUL"]);
	set_WAIT_NEXT_INT_MUL (cfg_processor["WAIT_NEXT_INT_MUL"]);
	set_INTEGER_MUL (cfg_processor["INTEGER_MUL"]);

	set_LATENCY_INTEGER_DIV (cfg_processor["LATENCY_INTEGER_DIV"]);
	set_WAIT_NEXT_INT_DIV (cfg_processor["WAIT_NEXT_INT_DIV"]);
	set_INTEGER_DIV (cfg_processor["INTEGER_DIV"]);

	set_QTDE_INTEGER_FU (INTEGER_ALU+INTEGER_MUL+INTEGER_DIV);

	set_LATENCY_FP_DIV (cfg_processor["LATENCY_FP_DIV"]);
	set_WAIT_NEXT_FP_DIV (cfg_processor["WAIT_NEXT_FP_DIV"]);
	set_FP_DIV (cfg_processor["FP_DIV"]);

	set_LATENCY_FP_MUL (cfg_processor["LATENCY_FP_MUL"]);
	set_WAIT_NEXT_FP_MUL (cfg_processor["WAIT_NEXT_FP_MUL"]);
	set_FP_MUL (cfg_processor["FP_MUL"]);

	set_LATENCY_FP_ALU (cfg_processor["LATENCY_FP_ALU"]);
	set_WAIT_NEXT_FP_ALU (cfg_processor["WAIT_NEXT_FP_ALU"]);
	set_FP_ALU (cfg_processor["FP_ALU"]);

	set_QTDE_FP_FU (FP_ALU+FP_MUL+FP_DIV);

	set_PARALLEL_LOADS (cfg_processor["PARALLEL_LOADS"]);
	set_PARALLEL_STORES (cfg_processor["PARALLEL_STORES"]);

	set_FETCH_BUFFER (cfg_processor["FETCH_BUFFER"]);
	set_DECODE_BUFFER (cfg_processor["DECODE_BUFFER"]);
	set_RAT_SIZE (cfg_processor["RAT_SIZE"]);
	set_ROB_SIZE (cfg_processor["ROB_SIZE"]);
	set_UNIFIED_RS (cfg_processor["UNIFIED_RS"]);

	set_MOB_READ (cfg_processor["MOB_READ"]);
	set_MOB_WRITE (cfg_processor["MOB_WRITE"]);
	
	set_DEBUG(cfg_processor["DEBUG"]);
	set_PROCESSOR_DEBUG(cfg_processor["PROCESSOR_DEBUG"]);
	set_FETCH_DEBUG(cfg_processor["FETCH_DEBUG"]);
	set_DECODE_DEBUG(cfg_processor["DECODE_DEBUG"]);
	set_RENAME_DEBUG(cfg_processor["RENAME_DEBUG"]);
	set_DISPATCH_DEBUG(cfg_processor["DISPATCH_DEBUG"]);
	set_EXECUTE_DEBUG(cfg_processor["EXECUTE_DEBUG"]);
	set_MOB_DEBUG(cfg_processor["MOB_DEBUG"]);
	set_PRINT_MOB(cfg_processor["PRINT_MOB"]);
	set_PRINT_ROB(cfg_processor["PRINT_ROB"]);
	set_COMMIT_DEBUG(cfg_processor["COMMIT_DEBUG"]);
	set_MSHR_DEBUG(cfg_processor["MSHR_DEBUG"]);
	set_MULTICORE_DEBUG(cfg_processor["MULTICORE_DEBUG"]);

	set_WAIT_CYCLE(cfg_processor["WAIT_CYCLE"]);
	// Load Units
	set_LOAD_UNIT (cfg_processor["LOAD_UNIT"]);
	set_WAIT_NEXT_MEM_LOAD (cfg_processor["WAIT_NEXT_MEM_LOAD"]);
	set_LATENCY_MEM_LOAD (cfg_processor["LATENCY_MEM_LOAD"]);
	// Store Units
	set_STORE_UNIT (cfg_processor["STORE_UNIT"]);
	set_WAIT_NEXT_MEM_STORE (cfg_processor["WAIT_NEXT_MEM_STORE"]);
	set_LATENCY_MEM_STORE (cfg_processor["LATENCY_MEM_STORE"]);
	
	if (get_HAS_HIVE()){
		set_MOB_HIVE (cfg_processor["MOB_HIVE"]);
		set_HIVE_DEBUG(cfg_processor["HIVE_DEBUG"]);
		set_HIVE_UNIT (cfg_processor["HIVE_UNIT"]);
		set_WAIT_NEXT_MEM_HIVE (cfg_processor["WAIT_NEXT_MEM_HIVE"]);
		set_LATENCY_MEM_HIVE (cfg_processor["LATENCY_MEM_HIVE"]);
	}

	if (get_HAS_VIMA()){
		set_MOB_VIMA (cfg_processor["MOB_VIMA"]);
		set_VIMA_DEBUG(cfg_processor["VIMA_DEBUG"]);
		set_VIMA_UNIT (cfg_processor["VIMA_UNIT"]);
		set_WAIT_NEXT_MEM_VIMA (cfg_processor["WAIT_NEXT_MEM_VIMA"]);
		set_LATENCY_MEM_VIMA (cfg_processor["LATENCY_MEM_VIMA"]);
	}
	

	set_QTDE_MEMORY_FU (LOAD_UNIT+STORE_UNIT);
	if (get_HAS_HIVE()) set_QTDE_MEMORY_FU (get_QTDE_MEMORY_FU() + HIVE_UNIT);
	if (get_HAS_VIMA()) set_QTDE_MEMORY_FU (get_QTDE_MEMORY_FU() + VIMA_UNIT);


	set_DISAMBIGUATION_ENABLED (cfg_processor["DISAMBIGUATION_ENABLED"]);
	if (!strcmp(cfg_processor["DISAMBIGUATION_METHOD"], "HASHED")) this->DISAMBIGUATION_METHOD = DISAMBIGUATION_METHOD_HASHED;
	else if (!strcmp(cfg_processor["DISAMBIGUATION_METHOD"], "PERFECT")) this->DISAMBIGUATION_METHOD = DISAMBIGUATION_METHOD_PERFECT;

	// Cache memory defaults
	libconfig::Setting &cfg_cache_mem = cfg_root["CACHE_MEMORY"];
	set_LINE_SIZE(cfg_cache_mem["CONFIG"]["LINE_SIZE"]);

	uint32_t n_caches = get_cache_list(INSTRUCTION, cfg_cache_mem, INST_ASSOCIATIVITY, INST_LATENCY, INST_SIZE, INST_SETS, INST_LEVEL);
	set_INSTRUCTION_CACHES(n_caches);

	n_caches = get_cache_list(DATA, cfg_cache_mem, DATA_ASSOCIATIVITY, DATA_LATENCY, DATA_SIZE, DATA_SETS, DATA_LEVEL);
	set_DATA_CACHES(n_caches);

	// Memory controller defaults
	libconfig::Setting &cfg_memory = cfg_root["MEMORY_CONTROLLER"];

	set_PARALLEL_LIM_ACTIVE(cfg_memory["PARALLEL_LIM_ACTIVE"]);
	set_MAX_PARALLEL_REQUESTS_CORE(cfg_memory["MAX_PARALLEL_REQUESTS_CORE"]);

	// Prefetcher defaults
	libconfig::Setting &cfg_prefetcher = cfg_root["PREFETCHER"];

	set_PREFETCHER_ACTIVE(cfg_prefetcher["PREFETCHER_ACTIVE"]);

	//======================================================================
	// Initializating variables
	//======================================================================
	this->processor_id = 0;
	this->traceIsOver = false;
	this->hasBranch = false;
	this->insertError = false;
	this->snapshoted = false;
	this->fetchCounter = 1;
	this->decodeCounter = 1;
	this->renameCounter = 1;
	this->uopCounter = 1;
	this->commit_uop_counter = 0;
	this->set_stall_wrong_branch(0);
	this->memory_read_executed = 0;
	this->memory_write_executed = 0;

	this->set_stall_full_FetchBuffer(0);
    this->set_stall_wrong_branch(0);

    this->set_stall_full_DecodeBuffer(0);

	this->set_registerWrite(0);
	this->set_stall_full_MOB_Read(0);
	this->set_stall_full_MOB_Write(0);
	this->set_stall_full_ROB(0);

	this->set_stall_empty_RS(0);

	this->set_stat_disambiguation_read_false_positive(0);
	this->set_stat_disambiguation_write_false_positive(0);
	this->set_stat_address_to_address(0);
	this->set_times_reach_parallel_requests_read(0);
	this->set_times_reach_parallel_requests_write(0);
	this->set_ended_cycle(0);
	this->set_mem_req_wait_cycles(0);
	this->set_core_ram_request_wait_cycles(0);
	this->set_core_ram_requests(0);

	this->set_stat_inst_int_alu_completed(0);
	this->set_stat_inst_mul_alu_completed(0);
	this->set_stat_inst_div_alu_completed(0);
	this->set_stat_inst_int_fp_completed(0);
	this->set_stat_inst_mul_fp_completed(0);
	this->set_stat_inst_div_fp_completed(0);
	this->set_stat_inst_nop_completed(0);
	this->set_stat_inst_load_completed(0);
	this->set_stat_inst_store_completed(0);
	this->set_stat_inst_branch_completed(0);
	this->set_stat_inst_other_completed(0);
	//======================================================================
	// Initializating structures
	//======================================================================
	//======================================================================
	// FetchBuffer
	this->fetchBuffer.allocate(FETCH_BUFFER);
	// DecodeBuffer
	this->decodeBuffer.allocate(DECODE_BUFFER);
	// Register Alias Table
	this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t *>(RAT_SIZE, NULL);
	// Reorder Buffer
	this->robStart = 0;
	this->robEnd = 0;
	this->robUsed = 0;
	this->reorderBuffer = utils_t::template_allocate_array<reorder_buffer_line_t>(ROB_SIZE);
	for (uint32_t i = 0; i < ROB_SIZE; i++)
	{
		this->reorderBuffer[i].reg_deps_ptr_array = utils_t::template_allocate_initialize_array<reorder_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// // Memory Order Buffer Read
	this->memory_order_buffer_read = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_READ);
	for (size_t i = 0; i < MOB_READ; i++)
	{
		this->memory_order_buffer_read[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// LOAD
	this->memory_order_buffer_read_start = 0;
	this->memory_order_buffer_read_end = 0;
	this->memory_order_buffer_read_used = 0;
	// =========================================================================================
	// // Memory Order Buffer Write
	this->memory_order_buffer_write = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_WRITE);
	for (size_t i = 0; i < MOB_WRITE; i++)
	{
		this->memory_order_buffer_write[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// STORE
	this->memory_order_buffer_write_start = 0;
	this->memory_order_buffer_write_end = 0;
	this->memory_order_buffer_write_used = 0;
	// =========================================================================================
	if (get_HAS_HIVE()) {// // Memory Order Buffer HIVE
		this->memory_order_buffer_hive = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_HIVE);
		for (size_t i = 0; i < MOB_HIVE; i++)
		{
			this->memory_order_buffer_hive[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
		}
		// =========================================================================================
		// HIVE
		this->memory_order_buffer_hive_start = 0;
		this->memory_order_buffer_hive_end = 0;
		this->memory_order_buffer_hive_used = 0;
	}
	// =========================================================================================
	if (get_HAS_VIMA()) {// // Memory Order Buffer VIMA
		this->memory_order_buffer_vima = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_VIMA);
		for (size_t i = 0; i < MOB_VIMA; i++)
		{
			this->memory_order_buffer_vima[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
		}
		// =========================================================================================
		// VIMA
		this->memory_order_buffer_vima_start = 0;
		this->memory_order_buffer_vima_end = 0;
		this->memory_order_buffer_vima_used = 0;
	}
	// =========================================================================================
	//disambiguator
	switch (this->DISAMBIGUATION_METHOD){
		case DISAMBIGUATION_METHOD_HASHED:{
			this->disambiguator = new disambiguation_hashed_t;
			this->disambiguator->allocate();
			break;
		}
		case DISAMBIGUATION_METHOD_PERFECT:{
			//this->disambiguator = new disambiguation_perfect_t;
			break;
		}
	}
	// parallel requests
	// =========================================================================================
	//DRAM
	// =========================================================================================
	this->request_DRAM=0;
	// =========================================================================================
	//allocating fus int
	this->fu_int_alu = utils_t::template_allocate_initialize_array<uint64_t>(INTEGER_ALU, 0);
	this->fu_int_mul = utils_t::template_allocate_initialize_array<uint64_t>(INTEGER_MUL, 0);
	this->fu_int_div = utils_t::template_allocate_initialize_array<uint64_t>(INTEGER_DIV, 0);
	//allocating fus fp
	this->fu_fp_alu = utils_t::template_allocate_initialize_array<uint64_t>(FP_ALU, 0);
	this->fu_fp_mul = utils_t::template_allocate_initialize_array<uint64_t>(FP_MUL, 0);
	this->fu_fp_div = utils_t::template_allocate_initialize_array<uint64_t>(FP_DIV, 0);
	//allocating fus memory
	this->fu_mem_load = utils_t::template_allocate_initialize_array<uint64_t>(LOAD_UNIT, 0);
	this->fu_mem_store = utils_t::template_allocate_initialize_array<uint64_t>(STORE_UNIT, 0);
	if (get_HAS_HIVE()){
		this->fu_mem_hive = utils_t::template_allocate_initialize_array<uint64_t>(HIVE_UNIT, 0);
	} else {
		this->fu_mem_hive = NULL;
	}
	if (get_HAS_VIMA()) this->fu_mem_vima = utils_t::template_allocate_initialize_array<uint64_t>(VIMA_UNIT, 0);
	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_reservation_station.reserve(ROB_SIZE);
	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_functional_units.reserve(ROB_SIZE);
}
// =====================================================================
bool processor_t::isBusy(){
	return (this->traceIsOver == false ||
			!this->fetchBuffer.is_empty() ||
			!this->decodeBuffer.is_empty() ||
			this->robUsed != 0);
}

// ======================================
// Require a position to insert on ROB
// The Reorder Buffer behavior is a Circular FIFO
// @return position to insert
// ======================================
int32_t processor_t::searchPositionROB(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->robUsed < ROB_SIZE)
	{
		position = this->robEnd;
		this->robUsed++;
		this->robEnd++;
		if (this->robEnd >= ROB_SIZE)
		{
			this->robEnd = 0;
		}
	}
	return position;
}
// ======================================
// Remove the Head of the reorder buffer
// The Reorder Buffer behavior is a Circular FIFO
// ======================================
void processor_t::removeFrontROB(){
	ERROR_ASSERT_PRINTF(this->robUsed > 0, "Removendo do ROB sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->reorderBuffer[this->robStart].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
	this->reorderBuffer[this->robStart].package_clean();
	this->robUsed--;
	this->robStart++;
	if (this->robStart >= ROB_SIZE)
	{
		this->robStart = 0;
	}
}
// ============================================================================
// get position on MOB read.
// MOB read is a circular buffer
// ============================================================================
int32_t processor_t::search_position_mob_read(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_read_used < MOB_READ)
	{
		position = this->memory_order_buffer_read_end;
		this->memory_order_buffer_read_used++;
		this->memory_order_buffer_read_end++;
		if (this->memory_order_buffer_read_end >= MOB_READ)
		{
			this->memory_order_buffer_read_end = 0;
		}
	}
	return position;
}
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_read(){
	if (COMMIT_DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Read Entry \n%s\n", this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	}
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read_used > 0, "Removendo do MOB_READ sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[this->memory_order_buffer_read_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
	this->memory_order_buffer_read_used--;
	this->memory_order_buffer_read[this->memory_order_buffer_read_start].package_clean();
	this->memory_order_buffer_read_start++;
	if (this->memory_order_buffer_read_start >= MOB_READ)
	{
		this->memory_order_buffer_read_start = 0;
	}
}
// ============================================================================
// get position on MOB hive.
// MOB read is a circular buffer
// ============================================================================
int32_t processor_t::search_position_mob_hive(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_hive_used < MOB_HIVE)
	{
		position = this->memory_order_buffer_hive_end;
		this->memory_order_buffer_hive_used++;
		this->memory_order_buffer_hive_end++;
		if (this->memory_order_buffer_hive_end >= MOB_HIVE)
		{
			this->memory_order_buffer_hive_end = 0;
		}
	}
	return position;
}
// ============================================================================
// get position on MOB vima.
// ============================================================================
int32_t processor_t::search_position_mob_vima(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_vima_used < MOB_VIMA)
	{
		position = this->memory_order_buffer_vima_end;
		this->memory_order_buffer_vima_used++;
		this->memory_order_buffer_vima_end++;
		if (this->memory_order_buffer_vima_end >= MOB_VIMA)
		{
			this->memory_order_buffer_vima_end = 0;
		}
	}
	return position;
}
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_hive(){
	if (COMMIT_DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB HIVE Entry \n%s\n", this->memory_order_buffer_hive[this->memory_order_buffer_hive_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	}
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_hive_used > 0, "Removendo do MOB_HIVE sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_hive[this->memory_order_buffer_hive_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
	this->memory_order_buffer_hive_used--;
	this->memory_order_buffer_hive[this->memory_order_buffer_hive_start].package_clean();
	this->memory_order_buffer_hive_start++;
	if (this->memory_order_buffer_hive_start >= MOB_HIVE)
	{
		this->memory_order_buffer_hive_start = 0;
	}
}
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_vima(){
	if (COMMIT_DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB VIMA Entry \n%s\n", this->memory_order_buffer_vima[this->memory_order_buffer_vima_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	}
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_vima_used > 0, "Removendo do MOB_VIMA sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_vima[this->memory_order_buffer_vima_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
	this->memory_order_buffer_vima_used--;
	this->memory_order_buffer_vima[this->memory_order_buffer_vima_start].package_clean();
	this->memory_order_buffer_vima_start++;
	if (this->memory_order_buffer_vima_start >= MOB_VIMA)
	{
		this->memory_order_buffer_vima_start = 0;
	}
}
// ============================================================================
// get position on MOB write.
// MOB read is a circular buffer
// ============================================================================
int32_t processor_t::search_position_mob_write(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_write_used < MOB_WRITE)
	{
		position = this->memory_order_buffer_write_end;
		this->memory_order_buffer_write_used++;
		this->memory_order_buffer_write_end++;
		if (this->memory_order_buffer_write_end >= MOB_WRITE)
		{
			this->memory_order_buffer_write_end = 0;
		}
	}
	return position;
}
// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_write(){
	if (COMMIT_DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Write Entry \n%s\n", this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	}
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write_used > 0, "Removendo do MOB_WRITE sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[this->memory_order_buffer_write_start].sent == true,"Removendo sem ter sido enviado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write[this->memory_order_buffer_write_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n%s\n",this->memory_order_buffer_write[this->memory_order_buffer_write_start].rob_ptr->content_to_string().c_str(),this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
	this->memory_order_buffer_write_used--;
	this->memory_order_buffer_write[this->memory_order_buffer_write_start].package_clean();
	this->memory_order_buffer_write_start++;
	if (this->memory_order_buffer_write_start >= MOB_WRITE)
	{
		this->memory_order_buffer_write_start = 0;
	}
}
// ============================================================================


void processor_t::fetch(){
	if (FETCH_DEBUG){
		ORCS_PRINTF("Fetch Stage\n")
	}
	opcode_package_t operation;
	// uint32_t position;
	// Trace ->fetchBuffer
	for (uint32_t i = 0; i < FETCH_WIDTH; i++) {
		operation.package_clean();
		//bool updated = false;
		//=============================
		//Stall full fetch buffer
		//=============================
		if (this->fetchBuffer.is_full()) {
			this->add_stall_full_FetchBuffer();
			break;
		}
		//=============================
		//Stall branch wrong predict
		//=============================
		if (this->get_stall_wrong_branch() > orcs_engine.get_global_cycle()){
			break;
		}
		//=============================
		//Get new Opcode
		//=============================
		if (!orcs_engine.trace_reader[this->processor_id].trace_fetch(&operation)){
			this->traceIsOver = true;
			break;
		}
		if (FETCH_DEBUG){			
			ORCS_PRINTF("Opcode Fetched %s\n", operation.content_to_string2().c_str())
		}
		//============================
		//add control variables
		//============================
		operation.opcode_number = this->fetchCounter;
		operation.readyAt = orcs_engine.get_global_cycle() + FETCH_LATENCY;
		this->fetchCounter++;

		//============================
		///Solve Branch
		//============================

		if (this->hasBranch){
			//solve
			uint32_t stallWrongBranch = orcs_engine.branchPredictor[this->processor_id].solveBranch(this->previousBranch, operation);
			this->set_stall_wrong_branch (orcs_engine.get_global_cycle() + stallWrongBranch);
			this->hasBranch = false;
			//uint32_t ttc = orcs_engine.cacheManager->searchInstruction (this->processor_id, operation.opcode_address);
			// ORCS_PRINTF("ready after wrong branch %lu\n",this->get_stall_wrong_branch()+ttc)
			operation.updatePackageWait (stallWrongBranch);
			//updated = true;
			this->previousBranch.package_clean();
			// ORCS_PRINTF("Stall Wrong Branch %u\n",stallWrongBranch)
		}
		//============================
		// Operation Branch, set flag
		//============================
		if (operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH){
			orcs_engine.branchPredictor[this->processor_id].branches++;
			this->previousBranch = operation;
			this->hasBranch = true;
		}
		//============================
		//Insert into fetch buffer
		//============================
		if (POSITION_FAIL == this->fetchBuffer.push_back(operation)){
			break;
		}

		if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu fetch(): opcode %lu %s, readyAt %u, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", orcs_engine.get_global_cycle(), this->processor_id, operation.opcode_number, get_enum_instruction_operation_char (operation.opcode_operation), operation.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)

		//if (!updated){
			memory_package_t* request = new memory_package_t();
			
			request->clients.push_back (fetchBuffer.back());
			request->processor_id = this->processor_id;
			request->uop_number = fetchBuffer.back()->opcode_number;
			request->opcode_address = fetchBuffer.back()->opcode_address;
			request->opcode_number = fetchBuffer.back()->opcode_number;
			request->memory_address = fetchBuffer.back()->opcode_address |= (this->get_processor_id() << 56);
			request->memory_size = fetchBuffer.back()->opcode_size;
			request->memory_operation = MEMORY_OPERATION_INST;
			request->is_hive = false;
			request->is_vima = false;
			request->status = PACKAGE_STATE_UNTREATED;
			request->readyAt = orcs_engine.get_global_cycle();
			request->born_cycle = orcs_engine.get_global_cycle();
			request->sent_to_cache = false;
			request->sent_to_ram = false;
			request->type = INSTRUCTION;
			request->op_count[request->memory_operation]++;

			if (!orcs_engine.cacheManager->searchData(request)) delete request;
		//}
	}
}
// ============================================================================
/*
	===========================
	Elimina os elementos do fetch buffer
	============================================================================
	Divide the opcode into
	1st. uop READ MEM. + unaligned
	2st. uop READ 2 MEM. + unaligned
	3rd. uop BRANCH
	4th. uop ALU
	5th. uop WRITE MEM. + unaligned
	============================================================================
	To maintain the right dependencies between the uops and opcodes
	If the opcode generates multiple uops, they must be in this format:

	READ    ReadRegs    = BaseRegs + IndexRegs
			WriteRegs   = 258 (Aux Register)

	ALU     ReadRegs    = * + 258 (Aux Register) (if is_read)
			WriteRegs   = * + 258 (Aux Register) (if is_write)

	WRITE   ReadRegs    = * + 258 (Aux Register)
			WriteRegs   = NULL
	============================================================================
*/
void processor_t::decode(){
	if (DECODE_DEBUG){
		ORCS_PRINTF("Decode Stage\n")
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("Opcode to decode %s\n", this->fetchBuffer.front()->content_to_string2().c_str())
		}
	}
	uop_package_t new_uop;
	int32_t statusInsert = POSITION_FAIL;
	for (size_t i = 0; i < DECODE_WIDTH; i++)
	{
		if (this->fetchBuffer.is_empty() ||
			this->fetchBuffer.front()->status != PACKAGE_STATE_READY ||
			this->fetchBuffer.front()->readyAt > orcs_engine.get_global_cycle())
		{
			//if (!this->fetchBuffer.is_empty()) ORCS_PRINTF ("NOT READY %s %u %lu\n", get_enum_package_state_char (this->fetchBuffer.front()->status), this->fetchBuffer.front()->readyAt, orcs_engine.get_global_cycle())
			break;
		}
		if (this->decodeBuffer.get_capacity() - this->decodeBuffer.get_size() < MAX_UOP_DECODED)
		{
			this->add_stall_full_DecodeBuffer();
			//ORCS_PRINTF ("DECODE FULL %s %s %u %lu\n", get_enum_package_state_char (this->fetchBuffer.front()->status), get_enum_instruction_operation_char (this->fetchBuffer.front()->opcode_operation), this->fetchBuffer.front()->readyAt, orcs_engine.get_global_cycle())
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeCounter == this->fetchBuffer.front()->opcode_number, "Trying decode out-of-order");
		this->decodeCounter++;

		//HIVE
		if (get_HAS_HIVE()){
			if (this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK){
				new_uop.package_clean();
				new_uop.opcode_to_uop (this->uopCounter++,
										this->fetchBuffer.front()->opcode_operation,
										0,
										1,
										*this->fetchBuffer.front());
				
				new_uop.is_hive = true;
				new_uop.is_vima = false;
				new_uop.hive_read1 = this->fetchBuffer.front()->hive_read1;
				new_uop.hive_read2 = this->fetchBuffer.front()->hive_read2;
				new_uop.hive_write = this->fetchBuffer.front()->hive_write;

				new_uop.updatePackageWait (DECODE_LATENCY);
				statusInsert = this->decodeBuffer.push_back(new_uop);
				if (DECODE_DEBUG){
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				}
				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
				this->fetchBuffer.pop_front();
				return;
			} else if (this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_LOAD){
				new_uop.package_clean();
				new_uop.opcode_to_uop (this->uopCounter++,
										this->fetchBuffer.front()->opcode_operation,
										this->fetchBuffer.front()->read_address,
										this->fetchBuffer.front()->read_size,
										*this->fetchBuffer.front());
				
				new_uop.is_hive = true;
				new_uop.is_vima = false;
				new_uop.hive_read1 = this->fetchBuffer.front()->hive_read1;
				new_uop.read_address = this->fetchBuffer.front()->read_address;
				new_uop.hive_read2 = this->fetchBuffer.front()->hive_read2;
				new_uop.read2_address = this->fetchBuffer.front()->read2_address;
				new_uop.hive_write = this->fetchBuffer.front()->hive_write;
				new_uop.write_address = this->fetchBuffer.front()->write_address;

				new_uop.updatePackageWait (DECODE_LATENCY);
				statusInsert = this->decodeBuffer.push_back(new_uop);
				if (DECODE_DEBUG){
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				}
				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
				this->fetchBuffer.pop_front();
				return;
			} else if (this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_HIVE_STORE){
				new_uop.package_clean();
				new_uop.opcode_to_uop (this->uopCounter++,
										this->fetchBuffer.front()->opcode_operation,
										this->fetchBuffer.front()->write_address,
										this->fetchBuffer.front()->write_size,
										*this->fetchBuffer.front());
				
				new_uop.is_hive = true;
				new_uop.is_vima = false;
				new_uop.hive_read1 = this->fetchBuffer.front()->hive_read1;
				new_uop.read_address = this->fetchBuffer.front()->read_address;
				new_uop.hive_read2 = this->fetchBuffer.front()->hive_read2;
				new_uop.read2_address = this->fetchBuffer.front()->read2_address;
				new_uop.hive_write = this->fetchBuffer.front()->hive_write;
				new_uop.write_address = this->fetchBuffer.front()->write_address;

				new_uop.updatePackageWait (DECODE_LATENCY);
				statusInsert = this->decodeBuffer.push_back(new_uop);
				if (DECODE_DEBUG){
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				}
				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
				this->fetchBuffer.pop_front();
				return;
			}
		}


		//VIMA
		if (get_HAS_VIMA()){
			if (this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
			this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA){
				new_uop.package_clean();
				new_uop.opcode_to_uop (this->uopCounter++,
										this->fetchBuffer.front()->opcode_operation,
										0,
										1,
										*this->fetchBuffer.front());
				
				new_uop.is_hive = false;
				new_uop.hive_read1 = -1;
				new_uop.hive_read2 = -1;
				new_uop.hive_write = -1;

				new_uop.is_vima = true;
				new_uop.read_address = fetchBuffer.front()->read_address;
				new_uop.read2_address = fetchBuffer.front()->read2_address;
				new_uop.write_address = fetchBuffer.front()->write_address;

				new_uop.updatePackageWait (DECODE_LATENCY);
				statusInsert = this->decodeBuffer.push_back(new_uop);
				if (DECODE_DEBUG){
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				}
				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
				this->fetchBuffer.pop_front();
				return;
			}
		}

		// =====================
		//Decode Read 1
		// =====================
		if (this->fetchBuffer.front()->is_read)
		{
			new_uop.package_clean();
			//creating uop
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_MEM_LOAD,
								  this->fetchBuffer.front()->read_address,
								  this->fetchBuffer.front()->read_size,
								  *this->fetchBuffer.front());
			
			//SE OP DIFERE DE LOAD, ZERA REGISTERS
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD)
			{
				// ===== Read Regs =============================================
				/// Clear RRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.read_regs[i] = POSITION_FAIL;
				}
				/// Insert BASE and INDEX into RReg
				new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
				new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;

				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
				/// Insert 258 into WRegs
				new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageWait(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			if (DECODE_DEBUG){
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			}
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Read 2
		// =====================
		if (this->fetchBuffer.front()->is_read2)
		{
			new_uop.package_clean();
			//creating uop
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_MEM_LOAD,
								  this->fetchBuffer.front()->read2_address,
								  this->fetchBuffer.front()->read2_size,
								  *this->fetchBuffer.front());
			//SE OP DIFERE DE LOAD, ZERA REGISTERS
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD)
			{
				// ===== Read Regs =============================================
				/// Clear RRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.read_regs[i] = POSITION_FAIL;
				}
				/// Insert BASE and INDEX into RReg
				new_uop.read_regs[0] = this->fetchBuffer.front()->base_reg;
				new_uop.read_regs[1] = this->fetchBuffer.front()->index_reg;

				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
				/// Insert 258 into WRegs
				new_uop.write_regs[0] = 258;
			}
			new_uop.updatePackageWait(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			if (DECODE_DEBUG){
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			}
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode ALU Operation
		// =====================
		if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_BRANCH &&
			this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD &&
			this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE)
		{
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
								  this->fetchBuffer.front()->opcode_operation,
								  0,
								  0,
								  *this->fetchBuffer.front());

			if (this->fetchBuffer.front()->is_read || this->fetchBuffer.front()->is_read2)
			{
				// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
				// ===== Read Regs =============================================
				//registers /258 aux onde pos[i] = fail
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					// ORCS_PRINTF("read reg %d\n",new_uop.read_regs[i])
					if (new_uop.read_regs[i] == POSITION_FAIL)
					{
						// ORCS_PRINTF("read reg2 %d\n",new_uop.read_regs[i])
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.\n", MAX_REGISTERS)
			}
			if (this->fetchBuffer.front()->is_write)
			{
				// ===== Write Regs =============================================
				//registers /258 aux onde pos[i] = fail
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.write_regs[i] == POSITION_FAIL)
					{
						new_uop.write_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.\n", MAX_REGISTERS)
				// assert(!inserted_258 && "Max registers used");
			}
			new_uop.updatePackageWait(DECODE_LATENCY);
			statusInsert = this->decodeBuffer.push_back(new_uop);
			if (DECODE_DEBUG){
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			}
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Branch
		// =====================https://old.reddit.com/r/rupaulsdragrace/
		if (this->fetchBuffer.front()->opcode_operation == INSTRUCTION_OPERATION_BRANCH)
		{
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_BRANCH,
								  0,
								  0,
								  *this->fetchBuffer.front());
			if (this->fetchBuffer.front()->is_read || this->fetchBuffer.front()->is_read2)
			{
				// ===== Read Regs =============================================
				/// Insert Reg258 into RReg
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.read_regs[i] == POSITION_FAIL)
					{
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
			}
			if (this->fetchBuffer.front()->is_write)
			{
				// ===== Write Regs =============================================
				//registers /258 aux onde pos[i] = fail
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					if (new_uop.write_regs[i] == POSITION_FAIL)
					{
						new_uop.write_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Todos Max regs usados. %u \n", MAX_REGISTERS)
			}
			new_uop.updatePackageWait(DECODE_LATENCY);
			statusInsert = this->decodeBuffer.push_back(new_uop);
			if (DECODE_DEBUG){
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			}
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Write
		// =====================
		if (this->fetchBuffer.front()->is_write)
		{
			new_uop.package_clean();
			// make package
			new_uop.opcode_to_uop(this->uopCounter++,
								  INSTRUCTION_OPERATION_MEM_STORE,
								  this->fetchBuffer.front()->write_address,
								  this->fetchBuffer.front()->write_size,
								  *this->fetchBuffer.front());
			//
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE){
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++){
					if (new_uop.read_regs[i] == POSITION_FAIL){
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}
				ERROR_ASSERT_PRINTF(inserted_258, "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)
				// assert(!inserted_258 && "Max registers used");
				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
			}
			new_uop.updatePackageWait(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
			if (DECODE_DEBUG){
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			}
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}

		if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu decode(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", orcs_engine.get_global_cycle(), this->processor_id, new_uop.uop_number, get_enum_instruction_operation_char (new_uop.uop_operation), new_uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)
		this->fetchBuffer.pop_front();
	}
}

// ============================================================================
void processor_t::update_registers(reorder_buffer_line_t *new_rob_line){
	/// Control the Register Dependency - Register READ
	for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
		if (new_rob_line->uop.read_regs[k] < 0) break;
		uint32_t read_register = new_rob_line->uop.read_regs[k];
		ERROR_ASSERT_PRINTF(read_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, RAT_SIZE);
		/// If there is a dependency
		if (this->register_alias_table[read_register] != NULL){
			for (uint32_t j = 0; j < ROB_SIZE; j++){
				if (this->register_alias_table[read_register]->reg_deps_ptr_array[j] == NULL){
					this->register_alias_table[read_register]->wake_up_elements_counter++;
					this->register_alias_table[read_register]->reg_deps_ptr_array[j] = new_rob_line;
					new_rob_line->wait_reg_deps_number++;
					break;
				}
			}
		}
	}

	/// Control the Register Dependency - Register WRITE
	for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
		this->add_registerWrite();
		if (new_rob_line->uop.write_regs[k] < 0) break;

		uint32_t write_register = new_rob_line->uop.write_regs[k];
		ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);

		this->register_alias_table[write_register] = new_rob_line;
	}
}
// ============================================================================
void processor_t::rename(){
	if (RENAME_DEBUG){
		ORCS_PRINTF("Rename Stage\n")
	}
	size_t i;
	int32_t pos_rob, pos_mob;

	for (i = 0; i < RENAME_WIDTH; i++)
	{
		memory_order_buffer_line_t *mob_line = NULL;
		// Checando se há uop decodificado, se está pronto, e se o ciclo de pronto
		// é maior ou igual ao atual
		if (this->decodeBuffer.is_empty() || this->decodeBuffer.front()->status != PACKAGE_STATE_WAIT || this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle()) {
			break;
		}

		ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter, "Erro, renomeio incorreto\n")
		//=======================
		// Memory Operation Read
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
			if(	this->memory_order_buffer_read_used>=MOB_READ ||
				this->robUsed>=ROB_SIZE )break;

			pos_mob = this->search_position_mob_read();
			if (pos_mob == POSITION_FAIL)
			{
				if (RENAME_DEBUG){
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				}
				this->add_stall_full_MOB_Read();
				break;
			}
			if (RENAME_DEBUG){
				ORCS_PRINTF("Get_Position_MOB_READ %d\n",pos_mob)
			}
			mob_line = &this->memory_order_buffer_read[pos_mob];
		}
		//=======================
		// Memory Operation Write
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
			if (this->memory_order_buffer_write_used >= MOB_WRITE || this->robUsed >= ROB_SIZE) break;
			pos_mob = this->search_position_mob_write();
			if (pos_mob == POSITION_FAIL) {
				if (RENAME_DEBUG) ORCS_PRINTF("Stall_MOB_Read_Full\n")
				this->add_stall_full_MOB_Write();
				break;
			}
			if (RENAME_DEBUG){
				ORCS_PRINTF("Get_Position_MOB_WRITE %d\n",pos_mob)
			}
			mob_line = &this->memory_order_buffer_write[pos_mob];
		}
		//=======================
		// Memory Operation HIVE
		//=======================
		if (get_HAS_HIVE()){
			if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_LOAD ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_HIVE_STORE){
				if (this->memory_order_buffer_hive_used>=MOB_HIVE || this->robUsed>=ROB_SIZE) break;
				pos_mob = this->search_position_mob_hive();
				if (pos_mob == POSITION_FAIL) {
					this->add_stall_full_MOB_Read();
					break;
				}
				mob_line = &this->memory_order_buffer_hive[pos_mob];
				//ORCS_PRINTF ("reservando espaço no MOB para instrução %s %lu\n", get_enum_instruction_operation_char(this->decodeBuffer.front()->opcode_operation), this->decodeBuffer.front()->opcode_number)
			}
		}

		//=======================
		// Memory Operation VIMA
		//=======================
		if (get_HAS_VIMA()){
			if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
			this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA){
				if (this->memory_order_buffer_hive_used>=MOB_VIMA || this->robUsed>=ROB_SIZE) break;
				pos_mob = this->search_position_mob_vima();
				if (pos_mob == POSITION_FAIL) {
					this->add_stall_full_MOB_Read();
					break;
				}
				mob_line = &this->memory_order_buffer_vima[pos_mob];
				//ORCS_PRINTF ("reservando espaço no MOB para instrução %s %lu\n", get_enum_instruction_operation_char(this->decodeBuffer.front()->opcode_operation), this->decodeBuffer.front()->opcode_number)
			}
		}
		
		//=======================
		// Verificando se tem espaco no ROB se sim bamos inserir
		//=======================
		pos_rob = this->searchPositionROB();
		if (pos_rob == POSITION_FAIL)
		{
			if (RENAME_DEBUG){
				ORCS_PRINTF("Stall_MOB_Read_Full\n")
			}
			this->add_stall_full_ROB();
			break;
		}
		// ===============================================
		// Inserting on ROB
		// ===============================================
		this->reorderBuffer[pos_rob].uop = *this->decodeBuffer.front();
		//remove uop from decodebuffer
		this->decodeBuffer.front()->package_clean();
		this->decodeBuffer.pop_front();
		this->renameCounter++;

		// =======================
		// Setting controls to ROB.
		// =======================
		this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_RENAME;
		this->reorderBuffer[pos_rob].uop.updatePackageWait(RENAME_LATENCY + DISPATCH_LATENCY);
		this->reorderBuffer[pos_rob].mob_ptr = mob_line;
		this->reorderBuffer[pos_rob].processor_id = this->processor_id;
		// =======================
		// Making registers dependences
		// =======================
		this->update_registers(&this->reorderBuffer[pos_rob]);
		if (RENAME_DEBUG){
			ORCS_PRINTF("Rename %s\n", this->reorderBuffer[pos_rob].content_to_string().c_str())
		}
		// =======================
		// Insert into Reservation Station
		// =======================
		this->unified_reservation_station.push_back(&this->reorderBuffer[pos_rob]);
		// =======================
		// Insert into MOB.
		// =======================
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
			if (RENAME_DEBUG) ORCS_PRINTF("Mem Load\n")
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_READ;
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		else if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE){
			if (RENAME_DEBUG) ORCS_PRINTF("Mem Store\n")
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_WRITE;
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		else if (this->get_HAS_HIVE() && (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOAD ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_STORE ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL)){
			this->reorderBuffer[pos_rob].mob_ptr->is_hive = true;
			this->reorderBuffer[pos_rob].mob_ptr->is_hive = false;
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->hive_read1 = this->reorderBuffer[pos_rob].uop.hive_read1;
			this->reorderBuffer[pos_rob].mob_ptr->hive_read2 = this->reorderBuffer[pos_rob].uop.hive_read2;
			this->reorderBuffer[pos_rob].mob_ptr->hive_write = this->reorderBuffer[pos_rob].uop.hive_write;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			switch (this->reorderBuffer[pos_rob].uop.uop_operation){
				case INSTRUCTION_OPERATION_HIVE_LOCK:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_LOCK;
					break;
				case INSTRUCTION_OPERATION_HIVE_UNLOCK:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_UNLOCK;
					break;
				case INSTRUCTION_OPERATION_HIVE_LOAD:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_LOAD;
					this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.read_address;
					break;
				case INSTRUCTION_OPERATION_HIVE_STORE:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_STORE;
					this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.write_address;
					break;
				case INSTRUCTION_OPERATION_HIVE_INT_ALU:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_INT_ALU;
					break;
            	case INSTRUCTION_OPERATION_HIVE_INT_MUL:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_INT_MUL;
					break;
            	case INSTRUCTION_OPERATION_HIVE_INT_DIV:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_INT_DIV;
					break;
            	case INSTRUCTION_OPERATION_HIVE_FP_ALU:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_FP_ALU;
					break;
            	case INSTRUCTION_OPERATION_HIVE_FP_MUL:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_FP_MUL;
					break;
            	case INSTRUCTION_OPERATION_HIVE_FP_DIV:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_FP_DIV;
					break;
				default:
					break;
			}
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		else if (this->get_HAS_VIMA() && (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
		this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA)){
			this->reorderBuffer[pos_rob].mob_ptr->is_hive = false;
			this->reorderBuffer[pos_rob].mob_ptr->is_vima = true;
			this->reorderBuffer[pos_rob].mob_ptr->vima_read1 = this->reorderBuffer[pos_rob].uop.read_address;
			this->reorderBuffer[pos_rob].mob_ptr->vima_read2 = this->reorderBuffer[pos_rob].uop.read2_address;
			this->reorderBuffer[pos_rob].mob_ptr->vima_write = this->reorderBuffer[pos_rob].uop.write_address;
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			switch (this->reorderBuffer[pos_rob].uop.uop_operation){
				case INSTRUCTION_OPERATION_VIMA_INT_ALU:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_ALU;
					break;
            	case INSTRUCTION_OPERATION_VIMA_INT_MUL:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_MUL;
					break;
            	case INSTRUCTION_OPERATION_VIMA_INT_DIV:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_DIV;
					break;
            	case INSTRUCTION_OPERATION_VIMA_FP_ALU:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_ALU;
					break;
            	case INSTRUCTION_OPERATION_VIMA_FP_MUL:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_MUL;
					break;
            	case INSTRUCTION_OPERATION_VIMA_FP_DIV:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_DIV;
					break;
				case INSTRUCTION_OPERATION_VIMA_INT_MLA:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_MLA;
					break;
				case INSTRUCTION_OPERATION_VIMA_FP_MLA:
                	this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_MLA;
					break;
				default:
					break;
			}
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		//linking rob and mob
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->make_memory_dependences(this->reorderBuffer[pos_rob].mob_ptr);
			}
		} else if (this->get_HAS_HIVE() && (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOAD ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_STORE ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL))
		{
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->make_memory_dependences(this->reorderBuffer[pos_rob].mob_ptr);
			}
		} else if (this->get_HAS_VIMA() && (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
			this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA))
		{
			mob_line->rob_ptr = &this->reorderBuffer[pos_rob];
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->make_memory_dependences(this->reorderBuffer[pos_rob].mob_ptr);
			}
		}

		if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu rename(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", orcs_engine.get_global_cycle(), this->processor_id, this->reorderBuffer[pos_rob].uop.uop_number, get_enum_instruction_operation_char (this->reorderBuffer[pos_rob].uop.uop_operation), this->reorderBuffer[pos_rob].uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)
	} //end for
}
// ============================================================================
void processor_t::dispatch(){
	if (DISPATCH_DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("====================================================================\n")
			ORCS_PRINTF("Dispatch Stage\n")
			ORCS_PRINTF("====================================================================\n")
		}
	}
		//control variables
		uint32_t total_dispatched = 0;
		/// Control the total dispatched per FU
		uint32_t fu_int_alu = 0;
		uint32_t fu_int_mul = 0;
		uint32_t fu_int_div = 0;

		uint32_t fu_fp_alu = 0;
		uint32_t fu_fp_mul = 0;
		uint32_t fu_fp_div = 0;

		uint32_t fu_mem_load = 0;
		uint32_t fu_mem_store = 0;
		uint32_t fu_mem_hive = 0;
		uint32_t fu_mem_vima = 0;

		for (uint32_t i = 0; i < this->unified_reservation_station.size() && i < UNIFIED_RS; i++)
		{
			//pointer to entry
			reorder_buffer_line_t *rob_line = this->unified_reservation_station[i];
			if (DISPATCH_DEBUG){
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("cycle %lu\n", orcs_engine.get_global_cycle())
					ORCS_PRINTF("=================\n")
					ORCS_PRINTF("Unified Reservations Station on use: %lu\n",this->unified_reservation_station.size())
					ORCS_PRINTF("Trying Dispatch %s\n", rob_line->content_to_string().c_str())
					ORCS_PRINTF("=================\n")
				}
			}
		
			if (total_dispatched >= DISPATCH_WIDTH){
				break;
			}

			if ((rob_line->uop.readyAt <= orcs_engine.get_global_cycle()) &&
				(rob_line->wait_reg_deps_number == 0)){
				ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_WAIT, "Error, uop not ready being dispatched\n %s\n", rob_line->content_to_string().c_str())
				ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_RENAME, "Error, uop not in Rename to rename stage\n %s\n",rob_line->content_to_string().c_str())

				//if dispatched
				bool dispatched = false;
				switch (rob_line->uop.uop_operation)
				{
				// NOP operation
				case INSTRUCTION_OPERATION_NOP:
				// integer alu// add/sub/logical
				case INSTRUCTION_OPERATION_INT_ALU:
				// branch op. como fazer, branch solved on fetch
				case INSTRUCTION_OPERATION_BRANCH:
				// op not defined
				case INSTRUCTION_OPERATION_OTHER:
					if (fu_int_alu < INTEGER_ALU)
					{
						for (uint8_t k = 0; k < INTEGER_ALU; k++)
						{
							if (this->fu_int_alu[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_alu[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_ALU;
								fu_int_alu++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_INTEGER_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Integer Multiplication
				case INSTRUCTION_OPERATION_INT_MUL:
					if (fu_int_mul < INTEGER_MUL)
					{
						for (uint8_t k = 0; k < INTEGER_MUL; k++)
						{
							if (this->fu_int_mul[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_mul[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_MUL;
								fu_int_mul++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_INTEGER_MUL);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Integer division
				case INSTRUCTION_OPERATION_INT_DIV:
					if (fu_int_div < INTEGER_DIV)
					{
						for (uint8_t k = 0; k < INTEGER_DIV; k++)
						{
							if (this->fu_int_div[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_int_div[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_INT_DIV;
								fu_int_div++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_INTEGER_DIV);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Floating point ALU operation
				case INSTRUCTION_OPERATION_FP_ALU:
					if (fu_fp_alu < FP_ALU)
					{
						for (uint8_t k = 0; k < FP_ALU; k++)
						{
							if (this->fu_fp_alu[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_alu[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_FP_ALU;
								fu_fp_alu++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_FP_ALU);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Floating Point Multiplication
				case INSTRUCTION_OPERATION_FP_MUL:
					if (fu_fp_mul < FP_MUL)
					{
						for (uint8_t k = 0; k < FP_MUL; k++)
						{
							if (this->fu_fp_mul[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_mul[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_FP_MUL;
								fu_fp_mul++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_FP_MUL);
								break;
							}
						}
					}
					break;

				// ====================================================
				// Floating Point Division
				case INSTRUCTION_OPERATION_FP_DIV:
					if (fu_fp_div < FP_DIV)
					{
						for (uint8_t k = 0; k < FP_DIV; k++)
						{
							if (this->fu_fp_div[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_fp_div[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_FP_DIV;
								fu_fp_div++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_FP_DIV);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Operation HIVE
				case INSTRUCTION_OPERATION_HIVE_LOCK:
                case INSTRUCTION_OPERATION_HIVE_UNLOCK:
                case INSTRUCTION_OPERATION_HIVE_LOAD:
                case INSTRUCTION_OPERATION_HIVE_STORE:
                case INSTRUCTION_OPERATION_HIVE_INT_ALU:
                case INSTRUCTION_OPERATION_HIVE_INT_MUL:
                case INSTRUCTION_OPERATION_HIVE_INT_DIV:
                case INSTRUCTION_OPERATION_HIVE_FP_ALU :
                case INSTRUCTION_OPERATION_HIVE_FP_MUL :
                case INSTRUCTION_OPERATION_HIVE_FP_DIV :
					if (fu_mem_hive < HIVE_UNIT)
					{
						for (uint8_t k = 0; k < HIVE_UNIT; k++)
						{
							if (this->fu_mem_hive[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_mem_hive[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_MEM_HIVE;
								fu_mem_hive++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_MEM_HIVE);
								if (DEBUG) ORCS_PRINTF ("Processor dispatch(): HIVE instruction %lu dispatched!\n", rob_line->uop.uop_number)
								break;
							}
						}
					}
					break;
				case INSTRUCTION_OPERATION_VIMA_INT_ALU:
                case INSTRUCTION_OPERATION_VIMA_INT_MUL:
                case INSTRUCTION_OPERATION_VIMA_INT_DIV:
                case INSTRUCTION_OPERATION_VIMA_FP_ALU :
                case INSTRUCTION_OPERATION_VIMA_FP_MUL :
                case INSTRUCTION_OPERATION_VIMA_FP_DIV :
				case INSTRUCTION_OPERATION_VIMA_INT_MLA:
				case INSTRUCTION_OPERATION_VIMA_FP_MLA:
					if (fu_mem_vima < VIMA_UNIT)
					{
						for (uint8_t k = 0; k < VIMA_UNIT; k++)
						{
							if (this->fu_mem_vima[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_mem_vima[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_MEM_VIMA;
								fu_mem_vima++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_MEM_VIMA);
								if (VIMA_DEBUG) ORCS_PRINTF ("Processor dispatch(): VIMA instruction %lu dispatched!\n", rob_line->uop.uop_number)
								break;
							}
						}
					}
					break;
				case INSTRUCTION_OPERATION_MEM_LOAD:
					if (fu_mem_load < LOAD_UNIT)
					{
						for (uint8_t k = 0; k < LOAD_UNIT; k++)
						{
							if (this->fu_mem_load[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_mem_load[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_MEM_LOAD;
								fu_mem_load++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_MEM_LOAD);
								break;
							}
						}
					}
					break;

				// ====================================================
				// Operation STORE
				case INSTRUCTION_OPERATION_MEM_STORE:
					if (fu_mem_store < STORE_UNIT)
					{
						for (uint8_t k = 0; k < STORE_UNIT; k++)
						{
							if (this->fu_mem_store[k] <= orcs_engine.get_global_cycle())
							{
								this->fu_mem_store[k] = orcs_engine.get_global_cycle() + WAIT_NEXT_MEM_STORE;
								fu_mem_store++;
								dispatched = true;
								rob_line->stage = PROCESSOR_STAGE_EXECUTION;
								rob_line->uop.updatePackageWait(LATENCY_MEM_STORE);
								break;
							}
						}
					}
					break;
				// ====================================================
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
				case INSTRUCTION_OPERATION_LAST:
					ERROR_PRINTF("Invalid instruction LAST||BARRIER||HMC_ROA||HMC_ROWA being dispatched.\n");
					break;
				} //end switch
				//remover os postos em execucao aqui
				if (dispatched == true)
				{
				if (DISPATCH_DEBUG){
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("Dispatched %s\n", rob_line->content_to_string().c_str())
						ORCS_PRINTF("===================================================================\n")
					}
				}
					// update Dispatched
					total_dispatched++;
					// insert on FUs waiting structure
					this->unified_functional_units.push_back(rob_line);
					// remove from reservation station
					this->unified_reservation_station.erase(this->unified_reservation_station.begin() + i);
					i--;
				} //end if dispatched

				if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu dispatch(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", orcs_engine.get_global_cycle(), this->processor_id, rob_line->uop.uop_number, get_enum_instruction_operation_char (rob_line->uop.uop_operation), rob_line->uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)
			}	 //end if robline is ready
		}		  //end for
		// sleep(1);
} //end method

//clean_mob_write() copy!
void processor_t::clean_mob_hive(){
	uint32_t pos = this->memory_order_buffer_hive_start;
	for (uint8_t i = 0; i < this->memory_order_buffer_hive_used; i++){
		if (this->memory_order_buffer_hive[pos].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_hive[pos].readyAt <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_hive[pos].processed == false){
			this->memory_order_buffer_hive[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_hive[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_hive[pos].processed=true;
			this->memory_hive_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_hive[pos].rob_ptr);
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->solve_memory_dependences(&this->memory_order_buffer_hive[pos]);
			}
			if (DEBUG) ORCS_PRINTF ("Processor clean_mob_hive(): HIVE instruction %lu %s, %u!\n", this->memory_order_buffer_hive[pos].uop_number, get_enum_processor_stage_char (this->memory_order_buffer_hive[pos].rob_ptr->stage), this->memory_order_buffer_hive[pos].readyAt)
		}
		pos++;
		if(pos >= MOB_HIVE) pos = 0;
	}
}

void processor_t::clean_mob_vima(){
	uint32_t pos = this->memory_order_buffer_vima_start;
	for (uint8_t i = 0; i < this->memory_order_buffer_vima_used; i++){
		if (this->memory_order_buffer_vima[pos].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_vima[pos].readyAt <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_vima[pos].processed == false){
			this->memory_order_buffer_vima[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_vima[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_vima[pos].processed=true;
			this->memory_vima_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_vima[pos].rob_ptr);
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->solve_memory_dependences(&this->memory_order_buffer_vima[pos]);
			}
			if (DEBUG) ORCS_PRINTF ("Processor clean_mob_vima(): HIVE instruction %lu %s, %u!\n", this->memory_order_buffer_vima[pos].uop_number, get_enum_processor_stage_char (this->memory_order_buffer_vima[pos].rob_ptr->stage), this->memory_order_buffer_vima[pos].readyAt)
		}
		pos++;
		if(pos >= MOB_VIMA) pos = 0;
	}
}

void processor_t::clean_mob_read(){
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	uint32_t pos = this->memory_order_buffer_read_start;
	for (uint8_t i = 0; i < this->memory_order_buffer_read_used; i++){
		if (this->memory_order_buffer_read[pos].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_read[pos].readyAt <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_read[pos].processed == false){
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[pos].uop_executed == true, "Removing memory read before being executed.\n")
			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[pos].wait_mem_deps_number == 0, "Number of memory dependencies should be zero.\n %s\n",this->memory_order_buffer_read[i].rob_ptr->content_to_string().c_str())
			if (EXECUTE_DEBUG){
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("\nSolving %s\n\n", this->memory_order_buffer_read[pos].rob_ptr->content_to_string().c_str())
				}
			}
			this->memory_order_buffer_read[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_read[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_read[pos].processed=true;
			this->memory_read_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_read[pos].rob_ptr);
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->solve_memory_dependences(&this->memory_order_buffer_read[pos]);
			}
			if(this->memory_order_buffer_read[pos].waiting_DRAM){
				ERROR_ASSERT_PRINTF(this->request_DRAM > 0,"ERRO, Contador negativo Waiting DRAM\n")
				if (EXECUTE_DEBUG){
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("\nReducing DRAM COUNTER\n\n")
					}
				}
				this->request_DRAM--;
			}
		}
		pos++;
		if( pos >= MOB_READ) pos=0;
	}
}
// ============================================================================
void processor_t::execute()
{
	if (EXECUTE_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Execute Stage ==========\n")
		}
	}
	if (this->get_HAS_VIMA()) this->clean_mob_vima();
	if (this->get_HAS_HIVE()) this->clean_mob_hive();
	this->clean_mob_read();
	uint32_t uop_total_executed = 0;
	for (uint32_t i = 0; i < this->unified_functional_units.size(); i++){

		reorder_buffer_line_t *rob_line = this->unified_functional_units[i];
		if (uop_total_executed == EXECUTE_WIDTH){
			break;
		}
		if (rob_line == NULL){
			break;
		}

		if (rob_line->uop.readyAt <= orcs_engine.get_global_cycle()){
			ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_EXECUTION, "ROB not on execution state")
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_WAIT, "FU with Package not in ready state")
			switch (rob_line->uop.uop_operation){
				// =============================================================
				// BRANCHES
				case INSTRUCTION_OPERATION_BRANCH:
				// INTEGERS ===============================================
				case INSTRUCTION_OPERATION_INT_ALU:
				case INSTRUCTION_OPERATION_NOP:
				case INSTRUCTION_OPERATION_OTHER:
				case INSTRUCTION_OPERATION_INT_MUL:
				case INSTRUCTION_OPERATION_INT_DIV:
				// FLOAT POINT ===============================================
				case INSTRUCTION_OPERATION_FP_ALU:
				case INSTRUCTION_OPERATION_FP_MUL:
				case INSTRUCTION_OPERATION_FP_DIV:
				{
					rob_line->stage = PROCESSOR_STAGE_COMMIT;
					rob_line->uop.updatePackageReady(EXECUTE_LATENCY + COMMIT_LATENCY);
					this->solve_registers_dependency(rob_line);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				// HIVE ==========================================
				case INSTRUCTION_OPERATION_HIVE_LOCK:
                case INSTRUCTION_OPERATION_HIVE_UNLOCK:
                case INSTRUCTION_OPERATION_HIVE_LOAD:
                case INSTRUCTION_OPERATION_HIVE_STORE:
                case INSTRUCTION_OPERATION_HIVE_INT_ALU:
                case INSTRUCTION_OPERATION_HIVE_INT_MUL:
                case INSTRUCTION_OPERATION_HIVE_INT_DIV:
                case INSTRUCTION_OPERATION_HIVE_FP_ALU :
                case INSTRUCTION_OPERATION_HIVE_FP_MUL :
                case INSTRUCTION_OPERATION_HIVE_FP_DIV :
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_hive_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageWait(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
					
					if (DEBUG) ORCS_PRINTF ("Processor execute(): HIVE instruction %lu executed!\n", rob_line->uop.uop_number)
				}
				break;
				// VIMA ==========================================
				case INSTRUCTION_OPERATION_VIMA_INT_ALU:
                case INSTRUCTION_OPERATION_VIMA_INT_MUL:
                case INSTRUCTION_OPERATION_VIMA_INT_DIV:
                case INSTRUCTION_OPERATION_VIMA_FP_ALU :
                case INSTRUCTION_OPERATION_VIMA_FP_MUL :
                case INSTRUCTION_OPERATION_VIMA_FP_DIV :
				case INSTRUCTION_OPERATION_VIMA_INT_MLA:
				case INSTRUCTION_OPERATION_VIMA_FP_MLA:
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_vima_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageWait(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
					
					if (VIMA_DEBUG) ORCS_PRINTF ("Processor execute(): VIMA instruction %lu executed!\n", rob_line->uop.uop_number)
				}
				break;
				case INSTRUCTION_OPERATION_MEM_LOAD:
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_read_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageWait(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				case INSTRUCTION_OPERATION_MEM_STORE:
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Write with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_write_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageWait(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
				case INSTRUCTION_OPERATION_LAST:
					ERROR_PRINTF("Invalid BARRIER | HMC ROA | HMC ROWA | ÇAST.\n");
					break;
			} //end switch
			if (EXECUTE_DEBUG){
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("Executed %s\n", rob_line->content_to_string().c_str())
				}
			}

			if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu execute(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", orcs_engine.get_global_cycle(), this->processor_id, rob_line->uop.uop_number, get_enum_instruction_operation_char (rob_line->uop.uop_operation), rob_line->uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)
		} //end if ready package
	}	 //end for
	if (EXECUTE_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("Memory Operations Read Executed %u\n",this->memory_read_executed)
			ORCS_PRINTF("Memory Operations Write Executed %u\n",this->memory_write_executed)
			ORCS_PRINTF("Requests to DRAM on the Fly %d \n",this->request_DRAM)
		}
	}
	// =========================================================================
	// Verificar se foi executado alguma operação de leitura,
	//  e executar a mais antiga no MOB
	// =========================================================================
	for (size_t i = 0; i < PARALLEL_LOADS; i++){
		if(this->memory_read_executed!=0){
			this->mob_read();
		}
	}

	if(this->memory_hive_executed!=0){
		this->mob_hive();
	}

	if(this->memory_vima_executed!=0){
		this->mob_vima();
	}

	// ==================================
	// Executar o MOB Write, com a escrita mais antiga.
	// depois liberar e tratar as escrita prontas;
	// ==================================
	for (size_t i = 0; i < PARALLEL_STORES; i++){
		if(this->memory_write_executed!=0){
			this->mob_write();
		}
	}
		// =====================================
	if (EXECUTE_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	}
} //end method
// ============================================================================
memory_order_buffer_line_t* processor_t::get_next_op_load(){

	uint32_t pos = this->memory_order_buffer_read_start;
	for(uint32_t i = 0 ; i < this->memory_order_buffer_read_used; i++){
		if(this->memory_order_buffer_read[pos].uop_executed &&
			this->memory_order_buffer_read[pos].status == PACKAGE_STATE_WAIT &&
			this->memory_order_buffer_read[pos].sent==false &&
        	this->memory_order_buffer_read[pos].wait_mem_deps_number == 0 &&
			this->memory_order_buffer_read[pos].readyToGo <= orcs_engine.get_global_cycle()){
				return &this->memory_order_buffer_read[pos];
			}
		pos++;
		if( pos >= MOB_READ) pos=0;
	}
	return NULL;
}
// ============================================================================
uint32_t processor_t::mob_read(){
	if (MOB_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
			ORCS_PRINTF("=========== MOB Read ===========\n")
			ORCS_PRINTF("MOB Read Start %u\n",this->memory_order_buffer_read_start)
			ORCS_PRINTF("MOB Read End %u\n",this->memory_order_buffer_read_end)
			ORCS_PRINTF("MOB Read Used %u\n",this->memory_order_buffer_read_used)
			if (PRINT_MOB){
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					memory_order_buffer_line_t::printAllOrder(this->memory_order_buffer_read,MOB_READ,this->memory_order_buffer_read_start,this->memory_order_buffer_read_used);
				}
			}
			if(oldest_read_to_send!=NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("MOB Read Atual %s\n",this->oldest_read_to_send->content_to_string().c_str())
				}
			}
		}
	}
	if(this->oldest_read_to_send == NULL){
			this->oldest_read_to_send = this->get_next_op_load();
			if (MOB_DEBUG){
				if(oldest_read_to_send==NULL){
					if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
						ORCS_PRINTF("Oldest Read NULL\n")
					}
				}
			}
	}
	if (this->oldest_read_to_send != NULL && !this->oldest_read_to_send->sent && orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)){
		if (MOB_DEBUG){
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("=================================\n")
				ORCS_PRINTF("Sending to memory request to data\n")
				ORCS_PRINTF("%s\n",this->oldest_read_to_send->content_to_string().c_str())
				ORCS_PRINTF("=================================\n")
			}
		}
		
		if (!oldest_read_to_send->sent){
			memory_package_t* request = new memory_package_t();
			
			request->clients.push_back (oldest_read_to_send);
			request->opcode_address = oldest_read_to_send->opcode_address;
			request->memory_address = oldest_read_to_send->memory_address;
			request->memory_size = oldest_read_to_send->memory_size;
			request->memory_operation = oldest_read_to_send->memory_operation;
			request->status = PACKAGE_STATE_UNTREATED;
			request->is_hive = false;
			request->is_vima = false;
			request->hive_read1 = oldest_read_to_send->hive_read1;
			request->hive_read2 = oldest_read_to_send->hive_read2;
			request->hive_write = oldest_read_to_send->hive_write;
			request->readyAt = orcs_engine.get_global_cycle();
			request->born_cycle = orcs_engine.get_global_cycle();
			request->sent_to_ram = false;
			request->sent_to_cache = false;
			request->type = DATA;
			request->uop_number = oldest_read_to_send->uop_number;
			request->processor_id = this->processor_id;
			request->op_count[request->memory_operation]++;

			if (orcs_engine.cacheManager->searchData(request)){
				this->oldest_read_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
				this->oldest_read_to_send->sent=true;
				this->oldest_read_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
				if (DEBUG) ORCS_PRINTF ("Processor mob_read(): sending memory request %lu, %s.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
			} else {
				this->add_times_reach_parallel_requests_read();
				delete request;
			}
		}
		this->oldest_read_to_send = NULL;
	} //end if request null
	if (MOB_DEBUG){
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
		}
	}
	return OK;
} //end method

void processor_t::print_mob_hive(){
	ORCS_PRINTF ("Cycle: %lu\n", orcs_engine.get_global_cycle())
	for(uint32_t j = 0; j < this->memory_order_buffer_hive_used; j++){
		ORCS_PRINTF ("Processor print_mob_hive(): %s %s %lu %u %lu.\n", get_enum_package_state_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].status), get_enum_memory_operation_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].memory_operation), this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].uop_number, this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].wait_mem_deps_number, this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].readyToGo)
	}
}

void processor_t::print_mob_vima(){
	ORCS_PRINTF ("Cycle: %lu\n", orcs_engine.get_global_cycle())
	for(uint32_t j = 0; j < this->memory_order_buffer_vima_used; j++){
		ORCS_PRINTF ("Processor print_mob_vima(): %s %s %lu %u %lu.\n", get_enum_package_state_char (this->memory_order_buffer_vima[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].status), get_enum_memory_operation_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].memory_operation), this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].uop_number, this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].wait_mem_deps_number, this->memory_order_buffer_hive[(this->memory_order_buffer_vima_start+j) % MOB_VIMA].readyToGo)
	}
}

memory_order_buffer_line_t* processor_t::get_next_op_hive(){
	uint32_t pos = this->memory_order_buffer_hive_start;
	for(uint32_t i = 0 ; i < this->memory_order_buffer_hive_used; i++){
		if(this->memory_order_buffer_hive[pos].uop_executed &&
			this->memory_order_buffer_hive[pos].status == PACKAGE_STATE_WAIT &&
			this->memory_order_buffer_hive[pos].sent==false &&
        	this->memory_order_buffer_hive[pos].wait_mem_deps_number == 0 &&
			this->memory_order_buffer_hive[pos].readyToGo <= orcs_engine.get_global_cycle()){
				if (DEBUG) {
					//ORCS_PRINTF ("Processor get_next_op_hive(): fetching next HIVE instruction from MOB.\n")
					for(uint32_t j = 0; j < this->memory_order_buffer_hive_used; j++){
						//ORCS_PRINTF ("Processor get_next_op_hive(): %s %s %lu %u %lu.\n", get_enum_package_state_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].status), get_enum_memory_operation_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].memory_operation), this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].uop_number, this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].wait_mem_deps_number, this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].readyToGo)
					}
				}
				return &this->memory_order_buffer_hive[pos];
		} 
		pos++;
		if( pos >= MOB_HIVE) pos=0;
	}
	return NULL;
}

memory_order_buffer_line_t* processor_t::get_next_op_vima(){
	uint32_t pos = this->memory_order_buffer_vima_start;
	for(uint32_t i = 0 ; i < this->memory_order_buffer_vima_used; i++){
		if(this->memory_order_buffer_vima[pos].uop_executed &&
			this->memory_order_buffer_vima[pos].status == PACKAGE_STATE_WAIT &&
			this->memory_order_buffer_vima[pos].sent==false &&
        	this->memory_order_buffer_vima[pos].wait_mem_deps_number == 0 &&
			this->memory_order_buffer_vima[pos].readyToGo <= orcs_engine.get_global_cycle()){
				return &this->memory_order_buffer_vima[pos];
		} 
		pos++;
		if( pos >= MOB_VIMA) pos=0;
	}
	return NULL;
}

// ============================================================================
uint32_t processor_t::mob_hive(){
	if(this->oldest_hive_to_send==NULL)
		this->oldest_hive_to_send = this->get_next_op_hive();
	
	if (this->oldest_hive_to_send != NULL && orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)){
		if (!this->oldest_hive_to_send->sent){
			memory_package_t* request = new memory_package_t();
			
			request->clients.push_back (oldest_hive_to_send);
			request->opcode_address = oldest_hive_to_send->opcode_address;
			request->memory_address = oldest_hive_to_send->memory_address;
			request->memory_size = oldest_hive_to_send->memory_size;
			request->memory_operation = oldest_hive_to_send->memory_operation;
			request->status = PACKAGE_STATE_UNTREATED;
			request->is_hive = true;
			request->is_vima = false;
			request->hive_read1 = oldest_hive_to_send->hive_read1;
			request->hive_read2 = oldest_hive_to_send->hive_read2;
			request->hive_write = oldest_hive_to_send->hive_write;
			request->readyAt = orcs_engine.get_global_cycle();
			request->born_cycle = orcs_engine.get_global_cycle();
			request->type = DATA;
			request->sent_to_ram = false;
			request->sent_to_cache = false;
			request->uop_number = oldest_hive_to_send->uop_number;
			request->processor_id = this->processor_id;
			request->op_count[request->memory_operation]++;

			if (orcs_engine.cacheManager->searchData(request)){
				this->oldest_hive_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
				this->oldest_hive_to_send->sent=true;
				this->oldest_hive_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
				if (DEBUG) ORCS_PRINTF ("Processor mob_hive(): sending memory request %lu, %s to cache manager.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
			} else delete request;
		}
		this->oldest_hive_to_send = NULL;
		// =============================================================
	}
	return OK;
}

// ============================================================================
uint32_t processor_t::mob_vima(){
	if(this->oldest_vima_to_send==NULL){
		this->oldest_vima_to_send = this->get_next_op_vima();
	}
	if (this->oldest_vima_to_send != NULL && orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)){
		if (!this->oldest_vima_to_send->sent){
			memory_package_t* request = new memory_package_t();
			
			request->clients.push_back (oldest_vima_to_send);
			request->opcode_address = oldest_vima_to_send->opcode_address;
			request->memory_address = oldest_vima_to_send->memory_address;
			request->memory_size = oldest_vima_to_send->memory_size;
			request->memory_operation = oldest_vima_to_send->memory_operation;
			request->status = PACKAGE_STATE_UNTREATED;
			request->is_hive = false;
			request->is_vima = true;
			request->vima_read1 = oldest_vima_to_send->vima_read1;
			request->vima_read2 = oldest_vima_to_send->vima_read2;
			request->vima_write = oldest_vima_to_send->vima_write;
			request->readyAt = orcs_engine.get_global_cycle();
			request->born_cycle = orcs_engine.get_global_cycle();
			request->type = DATA;
			request->sent_to_ram = false;
			request->sent_to_cache = false;
			request->uop_number = oldest_vima_to_send->uop_number;
			request->processor_id = this->processor_id;
			request->op_count[request->memory_operation]++;

			if (orcs_engine.cacheManager->searchData(request)){
				this->oldest_vima_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
				this->oldest_vima_to_send->sent=true;
				this->oldest_vima_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
				if (VIMA_DEBUG) ORCS_PRINTF ("Processor mob_vima(): sending memory request %lu, %s to cache manager.\n", request->uop_number, get_enum_memory_operation_char (request->memory_operation))
			} else delete request;
		}
		this->oldest_vima_to_send = NULL;
		// =============================================================
	}
	return OK;
}

// ============================================================================
memory_order_buffer_line_t* processor_t::get_next_op_store(){
	uint32_t i = this->memory_order_buffer_write_start;
	if(this->memory_order_buffer_write[i].uop_executed &&
		this->memory_order_buffer_write[i].status == PACKAGE_STATE_WAIT &&
		this->memory_order_buffer_write[i].sent ==false  &&
       	this->memory_order_buffer_write[i].wait_mem_deps_number <= 0 &&
		this->memory_order_buffer_write[i].readyToGo <= orcs_engine.get_global_cycle())
	{
		return &this->memory_order_buffer_write[i];
	} 
	return NULL;
}
// ============================================================================
uint32_t processor_t::mob_write(){
	if (MOB_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
			ORCS_PRINTF("=========== MOB Write ===========\n")
			ORCS_PRINTF("MOB Write Start %u\n",this->memory_order_buffer_write_start)
			ORCS_PRINTF("MOB Write End %u\n",this->memory_order_buffer_write_end)
			ORCS_PRINTF("MOB Write Used %u\n",this->memory_order_buffer_write_used)
			if (PRINT_MOB){
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					memory_order_buffer_line_t::printAllOrder(this->memory_order_buffer_write,MOB_WRITE,this->memory_order_buffer_write_start,this->memory_order_buffer_write_used);
				}
			}
			if(this->oldest_write_to_send!=NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("MOB write Atual %s\n",this->oldest_write_to_send->content_to_string().c_str())
				}
			}
		}
	}
	if(this->oldest_write_to_send==NULL){
		this->oldest_write_to_send = this->get_next_op_store();
		if (MOB_DEBUG){
			if(this->oldest_write_to_send==NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("Oldest Write NULL\n")
				}
			}
		}
	}
	if (this->oldest_write_to_send != NULL && orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_WRITE)){
		if (MOB_DEBUG){
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("=================================\n")
				ORCS_PRINTF("Sending to memory WRITE to data\n")
				ORCS_PRINTF("%s\n",this->oldest_write_to_send->content_to_string().c_str())
				ORCS_PRINTF("=================================\n")
			}
		}

		//sendind to write data
		if (!this->oldest_write_to_send->sent){
			memory_package_t* request = new memory_package_t();
			
			//request->clients.push_back (oldest_write_to_send);
			request->opcode_address = oldest_write_to_send->opcode_address;
			request->memory_address = oldest_write_to_send->memory_address;
			request->memory_size = oldest_write_to_send->memory_size;
			request->memory_operation = oldest_write_to_send->memory_operation;
			request->status = PACKAGE_STATE_UNTREATED;
			request->is_hive = false;
			request->is_vima = false;
			request->hive_read1 = oldest_write_to_send->hive_read1;
			request->hive_read2 = oldest_write_to_send->hive_read2;
			request->hive_write = oldest_write_to_send->hive_write;
			request->readyAt = orcs_engine.get_global_cycle();
			request->born_cycle = orcs_engine.get_global_cycle();
			request->type = DATA;
			request->sent_to_ram = false;
			request->sent_to_cache = false;
			request->uop_number = oldest_write_to_send->uop_number;
			request->processor_id = this->processor_id;
			request->op_count[request->memory_operation]++;

			if (orcs_engine.cacheManager->searchData(request)){
				this->oldest_write_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
				this->oldest_write_to_send->sent=true;
				this->oldest_write_to_send->rob_ptr->sent=true;	///Setting flag which marks sent request. set to remove entry on mob at commit
				this->oldest_write_to_send->rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
				this->oldest_write_to_send->rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
				this->oldest_write_to_send->processed=true;

				//ORCS_PRINTF ("%lu uop: %lu | %s | %u | %u.\n", orcs_engine.get_global_cycle(), this->oldest_write_to_send->rob_ptr->uop.uop_number, get_enum_processor_stage_char (this->oldest_write_to_send->rob_ptr->stage), this->counter_mshr_write, this->MAX_PARALLEL_REQUESTS_CORE)

				this->memory_write_executed--;
				this->solve_registers_dependency(this->oldest_write_to_send->rob_ptr);
				if (DISAMBIGUATION_ENABLED){
					this->disambiguator->solve_memory_dependences(this->oldest_write_to_send);
				}
				this->remove_front_mob_write();
			} else {
				this->add_times_reach_parallel_requests_write();
				delete request;
			}
		}
		this->oldest_write_to_send = NULL;
		// =============================================================
	} //end if request null
	return OK;
}
// ============================================================================
void processor_t::commit(){
	if (COMMIT_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
		{
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Commit Stage ==========\n")
			ORCS_PRINTF("Cycle %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("ROB Head %s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
			ORCS_PRINTF("==================================\n")
		}
	}
	int32_t pos_buffer;
	/// Commit the packages
	for (uint32_t i = 0; i < COMMIT_WIDTH; i++){
		pos_buffer = this->robStart;
		if (this->robUsed != 0 &&
			this->reorderBuffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
			this->reorderBuffer[pos_buffer].uop.status == PACKAGE_STATE_READY &&
			this->reorderBuffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle())
		{
			this->commit_uop_counter++;
			switch (this->reorderBuffer[pos_buffer].uop.uop_operation){
				// INTEGERS ALU
				case INSTRUCTION_OPERATION_INT_ALU:
					this->add_stat_inst_int_alu_completed();
					break;

				// INTEGERS MUL
				case INSTRUCTION_OPERATION_INT_MUL:
					this->add_stat_inst_mul_alu_completed();
					break;

				// INTEGERS DIV
				case INSTRUCTION_OPERATION_INT_DIV:
					this->add_stat_inst_div_alu_completed();
					break;

				// FLOAT POINT ALU
				case INSTRUCTION_OPERATION_FP_ALU:
					this->add_stat_inst_int_fp_completed();
					break;

				// FLOAT POINT MUL
				case INSTRUCTION_OPERATION_FP_MUL:
					this->add_stat_inst_mul_fp_completed();
					break;

				// FLOAT POINT DIV
				case INSTRUCTION_OPERATION_FP_DIV:
					this->add_stat_inst_div_fp_completed();
					break;

				case INSTRUCTION_OPERATION_HIVE_LOCK:
                case INSTRUCTION_OPERATION_HIVE_UNLOCK:
                case INSTRUCTION_OPERATION_HIVE_LOAD:
                case INSTRUCTION_OPERATION_HIVE_STORE:
                case INSTRUCTION_OPERATION_HIVE_INT_ALU:
                case INSTRUCTION_OPERATION_HIVE_INT_MUL:
                case INSTRUCTION_OPERATION_HIVE_INT_DIV:
                case INSTRUCTION_OPERATION_HIVE_FP_ALU :
                case INSTRUCTION_OPERATION_HIVE_FP_MUL :
                case INSTRUCTION_OPERATION_HIVE_FP_DIV :
					this->add_stat_inst_hive_completed();
					if (DEBUG) ORCS_PRINTF ("Processor commit(): instruction HIVE %lu, %s committed, readyAt %lu.\n", this->reorderBuffer[pos_buffer].uop.uop_number, get_enum_instruction_operation_char (this->reorderBuffer[pos_buffer].uop.uop_operation), this->reorderBuffer[pos_buffer].uop.readyAt)
					break;
				case INSTRUCTION_OPERATION_VIMA_INT_ALU:
                case INSTRUCTION_OPERATION_VIMA_INT_MUL:
                case INSTRUCTION_OPERATION_VIMA_INT_DIV:
                case INSTRUCTION_OPERATION_VIMA_FP_ALU :
                case INSTRUCTION_OPERATION_VIMA_FP_MUL :
                case INSTRUCTION_OPERATION_VIMA_FP_DIV :
				case INSTRUCTION_OPERATION_VIMA_INT_MLA:
				case INSTRUCTION_OPERATION_VIMA_FP_MLA:
					this->add_stat_inst_vima_completed();
					if (DEBUG || VIMA_DEBUG) ORCS_PRINTF ("Processor commit(): instruction VIMA %lu, %s committed, readyAt %lu.\n", this->reorderBuffer[pos_buffer].uop.uop_number, get_enum_instruction_operation_char (this->reorderBuffer[pos_buffer].uop.uop_operation), this->reorderBuffer[pos_buffer].uop.readyAt)
					break;
				// MEMORY OPERATIONS - READ
				case INSTRUCTION_OPERATION_MEM_LOAD:{
					if(this->reorderBuffer[pos_buffer].mob_ptr->waiting_DRAM){
						this->core_ram_request_wait_cycles+=(this->reorderBuffer[pos_buffer].mob_ptr->readyAt - this->reorderBuffer[pos_buffer].mob_ptr->cycle_send_request);
						this->add_core_ram_requests();
					}
					this->mem_req_wait_cycles+=(this->reorderBuffer[pos_buffer].mob_ptr->readyAt - this->reorderBuffer[pos_buffer].mob_ptr->readyToGo);
					//if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu commit(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", this->processor_id, orcs_engine.get_global_cycle(), this->reorderBuffer[pos_buffer].uop.uop_number, get_enum_instruction_operation_char (this->reorderBuffer[pos_buffer].uop.uop_operation), this->reorderBuffer[pos_buffer].uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)
					this->add_stat_inst_load_completed();
					break;
				}
				// MEMORY OPERATIONS - WRITE
				case INSTRUCTION_OPERATION_MEM_STORE:
					this->add_stat_inst_store_completed();
					break;
					// BRANCHES

				case INSTRUCTION_OPERATION_BRANCH:
					this->add_stat_inst_branch_completed();
					break;

				// NOP
				case INSTRUCTION_OPERATION_NOP:
					this->add_stat_inst_nop_completed();
					break;

				// NOT IDENTIFIED
				case INSTRUCTION_OPERATION_OTHER:
					this->add_stat_inst_other_completed();
					break;

				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROWA:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_LAST:
					ERROR_PRINTF("Invalid instruction BARRIER | HMC ROA | HMC ROWA | LAST.\n");
					break;
			}

			ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->robStart, "Commiting different from the position start\n");
			if (PROCESSOR_DEBUG) ORCS_PRINTF ("%lu processor %lu commit(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n", orcs_engine.get_global_cycle(), this->processor_id, this->reorderBuffer[pos_buffer].uop.uop_number, get_enum_instruction_operation_char (this->reorderBuffer[pos_buffer].uop.uop_operation), this->reorderBuffer[pos_buffer].uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), this->robUsed)

			if (COMMIT_DEBUG){
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
				{
					ORCS_PRINTF("======================================\n")
					ORCS_PRINTF("RM ROB Entry \n%s\n", this->reorderBuffer[this->robStart].content_to_string().c_str())
				}
			}
			if(this->reorderBuffer[this->robStart].sent==true){
				if(this->reorderBuffer[this->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_LOAD){
					this->remove_front_mob_read();
				} else if (this->reorderBuffer[this->robStart].uop.is_hive){
					this->remove_front_mob_hive();
				} else if (this->reorderBuffer[this->robStart].uop.is_vima){
					this->remove_front_mob_vima();
				} 
			}
			this->removeFrontROB();
		}
		/// Could not commit the older, then stop looking for ready uops
		else
		{
			if (DEBUG){
				ORCS_PRINTF ("=======Processor %lu, Cycle %lu=========\n", this->processor_id+1, orcs_engine.get_global_cycle())
				for (uint32_t i = 0; i < this->robUsed; i++){
					ORCS_PRINTF ("%u COMMIT: %s %s %s %lu %lu\n", i, get_enum_processor_stage_char (this->reorderBuffer[(i+robStart) % ROB_SIZE].stage), get_enum_instruction_operation_char (this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.uop_operation), get_enum_package_state_char (this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.status), this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.uop_number, this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.readyAt);
				}
			}
			break;
		}
	}
	if (COMMIT_DEBUG){
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	}

} //end method
// ============================================================================
void processor_t::solve_registers_dependency(reorder_buffer_line_t *rob_line){
		/// Remove pointers from Register Alias Table (RAT)
		for (uint32_t j = 0; j < MAX_REGISTERS; j++) {
			if (rob_line->uop.write_regs[j] < 0) break;
			
			uint32_t write_register = rob_line->uop.write_regs[j];
			ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);
			if (this->register_alias_table[write_register] != NULL && this->register_alias_table[write_register]->uop.uop_number == rob_line->uop.uop_number){
				this->register_alias_table[write_register] = NULL;
			} //end if
		}	 //end for

		// =========================================================================
		// SOLVE REGISTER DEPENDENCIES - RAT
		// =========================================================================
		for (uint32_t j = 0; j < ROB_SIZE; j++)
		{
			/// There is an unsolved dependency
			if (rob_line->reg_deps_ptr_array[j] != NULL){
				rob_line->wake_up_elements_counter--;
				rob_line->reg_deps_ptr_array[j]->wait_reg_deps_number--;
				/// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
				if (rob_line->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle()) rob_line->reg_deps_ptr_array[j]->uop.readyAt = orcs_engine.get_global_cycle();
				rob_line->reg_deps_ptr_array[j] = NULL;
			}
			/// All the dependencies are solved
			else
			{
				break;
			}
		}
}
// ============================================================================
void processor_t::statistics(){
	bool close = false;
	FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
		close=true;
	}
	if (output != NULL){
		utils_t::largestSeparator(output);
		fprintf(output, "Total_Cycle: %lu\n", this->get_ended_cycle());
		utils_t::largeSeparator(output);
		fprintf(output, "Stage_Opcode_and_Uop_Counters\n");
		utils_t::largeSeparator(output);
		fprintf(output, "Stage_Fetch: %lu\n", this->fetchCounter);
		fprintf(output, "Stage_Decode: %lu\n", this->decodeCounter);
		fprintf(output, "Stage_Rename: %lu\n", this->renameCounter);
		fprintf(output, "Stage_Commit: %lu\n", this->commit_uop_counter);
		utils_t::largestSeparator(output);
			if (MAX_PARALLEL_REQUESTS_CORE){
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_CORE_READ: %lu\n", this->get_times_reach_parallel_requests_read());
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_CORE_WRITE: %lu\n", this->get_times_reach_parallel_requests_write());
			}
		utils_t::largestSeparator(output);
		fprintf(output, "Instruction_Per_Cycle: %1.6lf\n", (float)this->fetchCounter/this->get_ended_cycle());
		// accessing LLC cache level
		int32_t *cache_indexes = new int32_t[3]();
		orcs_engine.cacheManager->generateIndexArray(this->processor_id, cache_indexes);
		fprintf(output, "MPKI: %lf\n", (float)orcs_engine.cacheManager->data_cache[2][cache_indexes[2]].get_cache_miss()/((float)this->fetchCounter/1000));
		fprintf(output, "Average_wait_cycles_wait_mem_req: %lf\n", (float)this->mem_req_wait_cycles/this->get_stat_inst_load_completed());
		fprintf(output, "Core_Request_RAM_AVG_Cycle: %lf\n", (float)this->core_ram_request_wait_cycles/this->get_core_ram_requests());
		fprintf(output, "Total Load Requests: %lu\n", this->get_stat_inst_load_completed());
		fprintf(output, "Total Store Requests: %lu\n", this->get_stat_inst_store_completed());
		fprintf(output, "Total HIVE Instructions: %lu\n", this->get_stat_inst_hive_completed());
		fprintf(output, "Total VIMA Instructions: %lu\n", this->get_stat_inst_vima_completed());
		utils_t::largestSeparator(output);
		delete[] cache_indexes;
	}
	if(close) fclose(output);
	this->disambiguator->statistics();
}

void processor_t::printCache(FILE *output) {
	fprintf(output, "===============Instruction $============\n");
	for (uint32_t i = 0; i < INSTRUCTION_CACHES; i++) {
		fprintf(output, "%u SIZE -> %u\n", INST_LEVEL[i], INST_SIZE[i]);
		fprintf(output, "%u ASSOCIATIVITY -> %u\n", INST_LEVEL[i], INST_ASSOCIATIVITY[i]);
		fprintf(output, "%u LATENCY -> %u\n", INST_LEVEL[i], INST_LATENCY[i]);
		fprintf(output, "%u SETS -> %u\n\n", INST_LEVEL[i], INST_SETS[i]);
	}
	fprintf(output, "==================Data $================\n");
	for (uint32_t i = 0; i < DATA_CACHES; i++) {
		fprintf(output, "%u SIZE -> %u\n", DATA_LEVEL[i], DATA_SIZE[i]);
		fprintf(output, "%u ASSOCIATIVITY -> %u\n", DATA_LEVEL[i], DATA_ASSOCIATIVITY[i]);
		fprintf(output, "%u LATENCY -> %u\n", DATA_LEVEL[i], DATA_LATENCY[i]);
		fprintf(output, "%u SETS -> %u\n\n", DATA_LEVEL[i], DATA_SETS[i]);
	}
}
// ============================================================================
void processor_t::printConfiguration(){
	FILE *output = fopen(orcs_engine.output_file_name, "a+");
	if (output != NULL)
	{
		fprintf(output, "===============Stages Width============\n");
		fprintf(output, "FETCH Width %d\n", FETCH_WIDTH);
		fprintf(output, "DECODE Width %d\n", DECODE_WIDTH);
		fprintf(output, "RENAME Width %d\n", RENAME_WIDTH);
		fprintf(output, "DISPATCH Width %d\n", DISPATCH_WIDTH);
		fprintf(output, "EXECUTE Width %d\n", EXECUTE_WIDTH);
		fprintf(output, "COMMIT Width %d\n", COMMIT_WIDTH);

		fprintf(output, "===============Structures Sizes============\n");
		fprintf(output, "Fetch Buffer ->%u\n", FETCH_BUFFER);
		fprintf(output, "Decode Buffer ->%u\n", DECODE_BUFFER);
		fprintf(output, "RAT ->%u\n", RAT_SIZE);
		fprintf(output, "ROB ->%u\n", ROB_SIZE);
		fprintf(output, "MOB Read ->%u\n", MOB_READ);
		fprintf(output, "MOB Write->%u\n", MOB_WRITE);
		fprintf(output, "Reservation Station->%u\n", UNIFIED_RS);
		fprintf(output, "===============Memory Configuration============\n");
		printCache(output);
		fprintf(output, "=============== PREFETCHER ============\n");
		fprintf(output, "PREFETCHER_ACTIVE ->%u\n", PREFETCHER_ACTIVE);

		fprintf(output, "===============RAM ============\n");
		fprintf(output, "RAM_LATENCY ->%u\n", RAM_LATENCY);
		fprintf(output, "=============== Limits ============\n");
		fprintf(output, "PARALLEL_LIM_ACTIVE ->%u\n", PARALLEL_LIM_ACTIVE);
		fprintf(output, "MAX_PARALLEL_REQUESTS_CORE ->%u\n", MAX_PARALLEL_REQUESTS_CORE);
	}
}

// ============================================================================
void processor_t::clock(){
	if (DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("============================PROCESSOR %lu===============================\n",this->processor_id)
			ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	}
	if (get_HAS_VIMA()) orcs_engine.vima_controller->clock();
	if (get_HAS_HIVE()) orcs_engine.hive_controller->clock();
	orcs_engine.cacheManager->clock();
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no ROB
	//// CommitStage
	//// ExecuteStage
	//// DispatchStage
	/////////////////////////////////////////////////
		if (this->robUsed != 0)
		{
			this->commit();   //commit instructions -> remove from ROB
			this->execute();  //verify Uops ready on UFs, then remove
			this->dispatch(); //dispath ready uops to UFs
		}
		/////////////////////////////////////////////////
		//// Verifica se existe coisas no DecodeBuffer
		//// Rename
		/////////////////////////////////////////////////
		if (!this->decodeBuffer.is_empty())
		{
			this->rename();
		}
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no FetchBuffer
	//// Decode
	/////////////////////////////////////////////////
	if (!this->fetchBuffer.is_empty())
	{
		this->decode();
	}
	/////////////////////////////////////////////////
	//// Verifica se trace is over
	//// Fetch
	/////////////////////////////////////////////////
	if ((!this->traceIsOver))
	{
		this->fetch();
	}

	if (!this->isBusy())
	{
		if(!this->snapshoted){
			this->set_ended_cycle(orcs_engine.get_global_cycle());
			this->snapshoted=true;
		}
	}
	if (DEBUG){
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("===================================================================\n")
			// sleep(1);
		}
	}
}
// ========================================================================================================================================================================================
