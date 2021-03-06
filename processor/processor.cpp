#include "./../simulator.hpp"
int maiorNumPartes = 0;
int DEBUG_STARTED = false;
int reps = 0;

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
	this->stall_full_ROB_VET = 0;

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
	this->MOB_VECTORIAL = 0;
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

	/*this->DEBUG = 0;
	this->PROCESSOR_DEBUG = 0;
	this->FETCH_DEBUG = 0;
	this->DECODE_DEBUG = 0;
	this->RENAME_DEBUG = 0;
	this->DISPATCH_DEBUG = 0;
	this->EXECUTE_DEBUG = 0;
	this->HIVE_DEBUG = 0;
	this->VIMA_DEBUGG = 0;
	this->COMMIT_DEBUG = 0;*/

	this->WAIT_CYCLE = 0;

	this->memory_read_executed = 0;
	this->memory_write_executed = 0;
	this->memory_vima_executed = 0;
	this->memory_hive_executed = 0;
	this->memory_vectorial_executed = 0;

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
	this->memory_order_buffer_vectorial = NULL;

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
	this->memory_order_buffer_vectorial_start = 0;
	this->memory_order_buffer_vectorial_used = 0;
	this->memory_order_buffer_vectorial_end = 0;
	//=========DESAMBIGUATION ============
	this->disambiguator = NULL;
	// ==========RAT======
	this->register_alias_table = NULL;
	// ==========ROB========
	this->reorderBuffer.reorderBuffer = NULL;
	this->vectorialReorderBuffer.reorderBuffer = NULL;

	// Dynamic vectorization configs
	VECTORIZATION_SIZE = 0;
	NUM_VR = 0;
	VRMT_SIZE = 0;
	TL_SIZE = 0;
	FU_VALIDATION_SIZE = 0;
	FU_VALIDATION_WAIT_NEXT = 0;
	FETCH_BUFFER_VECTORIZED = 0;
	DECODE_BUFFER_VECTORIZED = 0;
	ROB_VECTORIAL_SIZE = 0;
	VECTORIZATION_ENABLED = 0;



}

processor_t::~processor_t()
{
	for (size_t i = 0; i < MOB_READ; i++) {
		utils_t::template_delete_array(this->memory_order_buffer_read[i].mem_deps_ptr_array);
	}

	for (size_t i = 0; i < MOB_WRITE; i++) {
		utils_t::template_delete_array(this->memory_order_buffer_write[i].mem_deps_ptr_array);
	}

	if (this->get_HAS_HIVE()) {
		for (size_t i = 0; i < MOB_HIVE; i++) {
			utils_t::template_delete_array(this->memory_order_buffer_hive[i].mem_deps_ptr_array);
		}
	}

	if (this->get_HAS_VIMA()) {
		for (size_t i = 0; i < MOB_VIMA; i++) {
			utils_t::template_delete_array(this->memory_order_buffer_vima[i].mem_deps_ptr_array);
		}
	}

	for (size_t i = 0; i < MOB_VECTORIAL; i++) {
		utils_t::template_delete_array(this->memory_order_buffer_vectorial[i].mem_deps_ptr_array);
	}

	//Memory structures
	utils_t::template_delete_array(this->memory_order_buffer_read);
	utils_t::template_delete_array(this->memory_order_buffer_write);
	utils_t::template_delete_array(this->memory_order_buffer_hive);
	utils_t::template_delete_array(this->memory_order_buffer_vima);
	utils_t::template_delete_array(this->memory_order_buffer_vectorial);

	utils_t::template_delete_variable(this->disambiguator);
	//auxiliar var to maintain status oldest instruction
	utils_t::template_delete_variable(this->oldest_read_to_send);
	utils_t::template_delete_variable(this->oldest_write_to_send);
	utils_t::template_delete_variable(this->oldest_hive_to_send);
	utils_t::template_delete_variable(this->oldest_vima_to_send);
	utils_t::template_delete_variable(this->oldest_vectorial_to_send);


	//deleting deps array rob
	for (size_t i = 0; i < ROB_SIZE; i++)
	{
		utils_t::template_delete_array(this->reorderBuffer.reorderBuffer[i].reg_deps_ptr_array[0]);
	}
	// deleting rob
	utils_t::template_delete_array(this->reorderBuffer.reorderBuffer);
	utils_t::template_delete_array(this->vectorialReorderBuffer.reorderBuffer);

	//delete RAT
	utils_t::template_delete_array(this->register_alias_table);

	delete[] this->total_latency;
    delete[] this->total_operations;
    delete[] this->min_wait_operations;
    delete[] this->max_wait_operations;
	delete vectorizer;
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

	if (cfg_root.exists("VIMA_CONTROLLER")) {
		libconfig::Setting &cfg_vima = cfg_root["VIMA_CONTROLLER"];

		set_HAS_VIMA (1);
		set_MOB_VIMA (cfg_processor["MOB_VIMA"]);
		ORCS_PRINTF ("MOB_VIMA = %u\n", get_MOB_VIMA())
		set_VIMA_UNIT (cfg_processor["VIMA_UNIT"]);
		ORCS_PRINTF ("VIMA_UNIT = %u\n", get_VIMA_UNIT())
		set_WAIT_NEXT_MEM_VIMA (cfg_processor["WAIT_NEXT_MEM_VIMA"]);
		ORCS_PRINTF ("WAIT_NEXT_MEM_VIMA = %u\n", get_WAIT_NEXT_MEM_VIMA())
		set_LATENCY_MEM_VIMA (cfg_processor["LATENCY_MEM_VIMA"]);
		ORCS_PRINTF ("LATENCY_MEM_VIMA = %u\n", get_LATENCY_MEM_VIMA())
		if (cfg_vima.exists("VIMA_EXCEPT")) set_VIMA_EXCEPT (cfg_vima["VIMA_EXCEPT"]);
		else set_VIMA_EXCEPT (0);
	}
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
	set_MOB_VECTORIAL (cfg_processor["MOB_VECTORIAL"]);

	/*set_DEBUG(cfg_processor["DEBUG"]);
	set_PROCESSOR_DEBUG(cfg_processor["PROCESSOR_DEBUG"]);
	if (cfg_processor.exists ("MEMORY_DEBUG")) set_MEMORY_DEBUG (cfg_processor["MEMORY_DEBUG"]);
	else set_MEMORY_DEBUG (0);
	set_FETCH_DEBUG(cfg_processor["FETCH_DEBUG"]);
	set_DECODE_DEBUG(cfg_processor["DECODE_DEBUG"]);
	set_RENAME_DEBUG(cfg_processor["RENAME_DEBUG"]);
	set_DISPATCH_DEBUG(cfg_processor["DISPATCH_DEBUG"]);
	set_EXECUTE_DEBUG(cfg_processor["EXECUTE_DEBUG"]);
	set_COMMIT_DEBUG(cfg_processor["COMMIT_DEBUG"]);*/

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
		set_HIVE_UNIT (cfg_processor["HIVE_UNIT"]);
		set_WAIT_NEXT_MEM_HIVE (cfg_processor["WAIT_NEXT_MEM_HIVE"]);
		set_LATENCY_MEM_HIVE (cfg_processor["LATENCY_MEM_HIVE"]);
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

	set_CACHE_LEVELS((INSTRUCTION_CACHES > DATA_CACHES) ? INSTRUCTION_CACHES : DATA_CACHES);

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
	this->memory_vectorial_executed = 0;

	this->set_stall_full_FetchBuffer(0);
    this->set_stall_wrong_branch(0);

    this->set_stall_full_DecodeBuffer(0);

	this->set_registerWrite(0);
	this->set_stall_full_MOB_Read(0);
	this->set_stall_full_MOB_Write(0);
	this->set_stall_full_MOB_VET(0);


	this->set_stall_full_ROB(0);
	this->set_stall_full_ROB_VET(0);

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
	// Vectorization
	VECTORIZATION_SIZE = cfg_processor["VECTORIZATION_SIZE"];
	NUM_VR = cfg_processor["NUM_VR"];
	VRMT_SIZE = cfg_processor["VRMT_SIZE"];
	VRMT_ASSOCIATIVITY = cfg_processor["VRMT_ASSOCIATIVITY"];
	TL_SIZE = cfg_processor["TL_SIZE"];
	TL_ASSOCIATIVITY = cfg_processor["TL_ASSOCIATIVITY"];
	FU_VALIDATION_SIZE = cfg_processor["FU_VALIDATION_SIZE"];
	FU_VALIDATION_WAIT_NEXT = cfg_processor["FU_VALIDATION_WAIT_NEXT"];
	FETCH_BUFFER_VECTORIZED = cfg_processor["FETCH_BUFFER_VECTORIZED"];
	DECODE_BUFFER_VECTORIZED = cfg_processor["DECODE_BUFFER_VECTORIZED"];
	ROB_VECTORIAL_SIZE = cfg_processor["ROB_VECTORIAL_SIZE"];
	VECTORIZATION_ENABLED = cfg_processor["VECTORIZATION_ENABLED"];

	// FetchBuffer
	this->fetchBuffer.allocate(FETCH_BUFFER + FETCH_BUFFER_VECTORIZED);
	vec_on_fetchBuffer = 0;
	vec_on_decodeBuffer = 0;
	esc_on_decodeBuffer = 0;

	// DecodeBuffer
	this->decodeBuffer.allocate(DECODE_BUFFER + DECODE_BUFFER_VECTORIZED);

	// Register Alias Table
	this->register_alias_table = utils_t::template_allocate_initialize_array<reorder_buffer_line_t *>(RAT_SIZE, NULL);

	// Reorder Buffer
	this->reorderBuffer.init(ROB_SIZE);

	// Vectorial Reorder Buffer
	this->vectorialReorderBuffer.init(ROB_VECTORIAL_SIZE);
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
	// // Memory Order Buffer Vectorial
	this->memory_order_buffer_vectorial = utils_t::template_allocate_array<memory_order_buffer_line_t>(MOB_VECTORIAL);
	for (size_t i = 0; i < MOB_VECTORIAL; i++)
	{
		this->memory_order_buffer_vectorial[i].mem_deps_ptr_array = utils_t::template_allocate_initialize_array<memory_order_buffer_line_t *>(ROB_SIZE, NULL);
	}
	// =========================================================================================
	// LOAD
	this->memory_order_buffer_vectorial_start = 0;
	this->memory_order_buffer_vectorial_end = 0;
	this->memory_order_buffer_vectorial_used = 0;
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

	// =========================================================================================
	// Dynamic vectorization
	// =========================================================================================	
	this->inst_list.allocate(10); // Aloca tamanho suficiente para 1 vetorização
	pipeline_squashed = false;
	store_squashing = 0;
	this->vectorizer = new Vectorizer_t (&inst_list, &pipeline_squashed, &store_squashing);
	
	// parallel requests
	// =========================================================================================
	//DRAM
	// =========================================================================================
	this->request_DRAM=0;
	// =========================================================================================

    uint32_t fu_cnt = 0;

	// Alocate functional units
    int num_fus = orcs_engine.instruction_set->functional_units.size();
    this->functional_units.resize(num_fus);

    for (int i = 0; i < num_fus; ++i) {
        this->functional_units[i].allocate(
                fu_cnt++,
                orcs_engine.instruction_set->functional_units[i].size,
                orcs_engine.instruction_set->functional_units[i].wait_next
        );
    }

	// Allocate memory functional units
	this->fu_mem_load.allocate(fu_cnt++, LOAD_UNIT, WAIT_NEXT_MEM_LOAD);
	this->fu_mem_store.allocate(fu_cnt++, STORE_UNIT, WAIT_NEXT_MEM_STORE);

	if (get_HAS_HIVE())
        this->fu_mem_hive.allocate(fu_cnt++, HIVE_UNIT, WAIT_NEXT_MEM_HIVE);

	if (get_HAS_VIMA())
        this->fu_mem_vima.allocate(fu_cnt++, VIMA_UNIT, WAIT_NEXT_MEM_VIMA);

	// Dispatch validation
	this->fu_validation.allocate(fu_cnt++, FU_VALIDATION_SIZE, FU_VALIDATION_WAIT_NEXT);

	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_reservation_station.reserve(ROB_SIZE);
	this->unified_vectorial_reservation_station.reserve(ROB_VECTORIAL_SIZE);
	// reserving space to uops on UFs pipeline, waitng to executing ends
	this->unified_functional_units.reserve(ROB_SIZE);
	this->unified_vectorial_functional_units.reserve(ROB_VECTORIAL_SIZE);

	this->last_oldest_uop_dispatch = 0;

	this->total_latency = new uint64_t[INSTRUCTION_OPERATION_LAST]();
    this->total_operations = new uint64_t[INSTRUCTION_OPERATION_LAST]();
    this->min_wait_operations = new uint64_t[INSTRUCTION_OPERATION_LAST]();
    this->max_wait_operations = new uint64_t[INSTRUCTION_OPERATION_LAST]();

	for (int i = 0; i < INSTRUCTION_OPERATION_LAST; i++){
		this->min_wait_operations[i] = UINT64_MAX;
    	this->max_wait_operations[i] = 0;
	}

    this->wait_time = 0;
}
// =====================================================================
bool processor_t::isBusy(){
	return (this->traceIsOver == false ||
			!this->fetchBuffer.is_empty() ||
			!this->decodeBuffer.is_empty() ||
			reorderBuffer.robUsed != 0 ||
			vectorialReorderBuffer.robUsed != 0);
}

// ======================================
// Require a position to insert on ROB
// The Reorder Buffer behavior is a Circular FIFO
// @return position to insert
// ======================================
int32_t processor_t::searchPositionROB(ROB_t *rob){
	int32_t position = POSITION_FAIL;

	/// There is free space
	if (rob->robUsed < rob->SIZE)
	{
		position = rob->robEnd;
		rob->robUsed++;
		rob->robEnd++;
		if (rob->robEnd >= rob->SIZE)
		{
			rob->robEnd = 0;
		}
	}

	return position;
}

// ======================================
// Remove the Head of the reorder buffer
// The Reorder Buffer behavior is a Circular FIFO
// ======================================
void processor_t::removeFrontROB(ROB_t *rob){

	ERROR_ASSERT_PRINTF(rob->reorderBuffer[rob->robStart].reg_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",rob->reorderBuffer[rob->robStart].content_to_string().c_str())
	rob->reorderBuffer[rob->robStart].package_clean();
	rob->robUsed--;
	rob->robStart++;
	if (rob->robStart >= rob->SIZE)
	{
		rob->robStart = 0;
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
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Read Entry \n%s\n", this->memory_order_buffer_read[this->memory_order_buffer_read_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
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
// remove back mob read on rob entry missing
// ============================================================================
void processor_t::remove_back_mob_read(){
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Read Back Entry \n%s\n", this->memory_order_buffer_read[this->memory_order_buffer_read_end].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read_used > 0, "Removendo do MOB_READ sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[this->memory_order_buffer_read_end].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_read[this->memory_order_buffer_read_end].content_to_string().c_str())
	this->memory_order_buffer_read_used--;
	this->memory_order_buffer_read[this->memory_order_buffer_read_end].package_clean();
	if (this->memory_order_buffer_read_end == 0)
	{
		this->memory_order_buffer_read_end = MOB_READ - 1;
	} else {
		this->memory_order_buffer_read_end--;		
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
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB HIVE Entry \n%s\n", this->memory_order_buffer_hive[this->memory_order_buffer_hive_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
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
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB VIMA Entry \n%s\n", this->memory_order_buffer_vima[this->memory_order_buffer_vima_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_vima_used > 0, "Removendo do MOB_VIMA sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_vima[this->memory_order_buffer_vima_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_vima[this->memory_order_buffer_read_start].content_to_string().c_str())
	this->memory_order_buffer_vima_used--;
	this->memory_order_buffer_vima[this->memory_order_buffer_vima_start].package_clean();
	this->memory_order_buffer_vima_start++;
	if (this->memory_order_buffer_vima_start >= MOB_VIMA)
	{
		this->memory_order_buffer_vima_start = 0;
	}
}

// ============================================================================
// get position on MOB vectorial.
// ============================================================================
int32_t processor_t::search_position_mob_vectorial(){
	int32_t position = POSITION_FAIL;
	/// There is free space.
	if (this->memory_order_buffer_vectorial_used < MOB_VECTORIAL)
	{
		position = this->memory_order_buffer_vectorial_end;
		this->memory_order_buffer_vectorial_used++;
		this->memory_order_buffer_vectorial_end++;
		if (this->memory_order_buffer_vectorial_end >= MOB_VECTORIAL)
		{
			this->memory_order_buffer_vectorial_end = 0;
		}
	}
	return position;
}

// ============================================================================
// remove front mob read on commit
// ============================================================================
void processor_t::remove_front_mob_vectorial(){
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB vectorial Entry \n%s\n", this->memory_order_buffer_vectorial[this->memory_order_buffer_vectorial_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_vectorial_used > 0, "Removendo do MOB_VECTORIAL sem estar usado\n")
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_vectorial[this->memory_order_buffer_vectorial_start].mem_deps_ptr_array[0] == NULL, "Removendo sem resolver dependencias\n%s\n",this->memory_order_buffer_vectorial[this->memory_order_buffer_vectorial_start].content_to_string().c_str())
	this->memory_order_buffer_vectorial_used--;

	this->memory_order_buffer_vectorial[this->memory_order_buffer_vectorial_start].package_clean();
	this->memory_order_buffer_vectorial_start++;
	if (this->memory_order_buffer_vectorial_start >= MOB_VECTORIAL)
	{
		this->memory_order_buffer_vectorial_start = 0;
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
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Write Entry \n%s\n", this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	#if MEMORY_DEBUG
		ORCS_PRINTF ("[MOBL] %lu %lu %s removed from memory order buffer | %s | readyAt = %u.\n", orcs_engine.get_global_cycle(), memory_order_buffer_write[this->memory_order_buffer_write_start].memory_address, get_enum_memory_operation_char (memory_order_buffer_write[this->memory_order_buffer_write_start].memory_operation), get_enum_package_state_char(this->memory_order_buffer_write[this->memory_order_buffer_write_start].status), this->memory_order_buffer_write[this->memory_order_buffer_write_start].readyAt)
	#endif
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
// remove front mob read on commit
// ============================================================================
void processor_t::remove_back_mob_write(){
	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Write Back Entry \n%s\n", this->memory_order_buffer_write[this->memory_order_buffer_write_end].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
	#endif
	#if MEMORY_DEBUG
		ORCS_PRINTF ("[MOBL] %lu %lu %s removed from memory order buffer | %s | readyAt = %u.\n", orcs_engine.get_global_cycle(), memory_order_buffer_write[this->memory_order_buffer_write_end].memory_address, get_enum_memory_operation_char (memory_order_buffer_write[this->memory_order_buffer_write_end].memory_operation), get_enum_package_state_char(this->memory_order_buffer_write[this->memory_order_buffer_write_end].status), this->memory_order_buffer_write[this->memory_order_buffer_write_end].readyAt)
	#endif
	ERROR_ASSERT_PRINTF(this->memory_order_buffer_write_used > 0, "Removendo do MOB_WRITE sem estar usado\n")

	this->memory_order_buffer_write_used--;
	this->memory_order_buffer_write[this->memory_order_buffer_write_end].package_clean();
	if (this->memory_order_buffer_write_end == 0)
	{
		this->memory_order_buffer_write_end = MOB_READ - 1;
	} else {
		this->memory_order_buffer_write_end--;		
	}

}
// ============================================================================

uint64_t vet = 0;
uint64_t esc = 0;
void processor_t::fetch(){
	opcode_package_t operation;
	// uint32_t position;
	// Trace ->fetchBuffer
	for (uint32_t i = 0; (i < FETCH_WIDTH) || (inst_list.size > 0); i++) {
		if (orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_INST) ||
		    (inst_list.size > 0)){
			operation.package_clean();
			//bool updated = false;

			//=============================
			//Stall pipeline squashed
			//=============================
			if (this->pipeline_squashed) {
				break;
			}

			//=============================
			//Stall full fetch buffer
			//=============================
			//printf("%u\n",this->fetchBuffer.size - vec_on_fetchBuffer);
			if (this->fetchBuffer.size == (FETCH_BUFFER + vec_on_fetchBuffer)) {
				//printf("STALL\n");
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
			if (inst_list.size > 0) {
				operation = *this->inst_list.back();
				this->inst_list.pop_back();
				--i;

			} else if (!orcs_engine.trace_reader[this->processor_id].trace_fetch(&operation)){
				this->traceIsOver = true;
				break;
			}



			//============================
			// Vetorização
			//============================
			// Tenta vetorizar
			vectorizer->new_inst (&operation);
			/*if (operation.is_read && operation.is_read2) {
				printf("ERROR: One instruction with two reads\n");
				printf(">> %s\n", operation.opcode_assembly);
				exit(1);
			}*/


			//vectorizer->debug();

			// * ============================ * //

			//============================
			//add control variables
			//============================
			if (operation.is_vectorial_part < 0) {
				operation.readyAt = orcs_engine.get_global_cycle() + FETCH_LATENCY;
			} else {
				operation.readyAt = orcs_engine.get_global_cycle();
			}

			operation.opcode_number = this->fetchCounter;
			this->fetchCounter++;

			#if FETCH_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("%s - Opcode Fetched %s (%d - VR: %d) Opcode Number %lu %s\n", operation.opcode_assembly, 
				(operation.is_vectorial_part >= 0) ? "Vec" :
				(operation.is_validation) ? "Val" : "Ins",
				(operation.is_validation) ? operation.will_validate_offset : operation.is_vectorial_part,
				 operation.VR_id,
				 operation.opcode_number,
				 operation.content_to_string2().c_str())
			#endif

			//============================
			// Vetorização
			//============================
			// Envia para o pipeline
			vectorizer->enter_pipeline(&operation);
			if (operation.is_vectorial_part >= 0) {
				++vet;
			} else {
				++esc;
			}


			//============================
			///Solve Branch
			//============================

			if (this->hasBranch){
				//solve
				uint32_t stallWrongBranch = orcs_engine.branchPredictor[this->processor_id].solveBranch(this->previousBranch, operation);
				this->set_stall_wrong_branch (orcs_engine.get_global_cycle() + stallWrongBranch);
				this->hasBranch = false;

				// Vectorization
				// // Change GMRBB
				if (this->previousBranch.opcode_address > operation.opcode_address) {
					operation.is_BB = true; // A próxima indica que houve mudança
					operation.BB_addr = this->previousBranch.opcode_address;
				}

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
			
			#if PROCESSOR_DEBUG
				ORCS_PRINTF("%lu processor %lu fetch(): addr %lu opcode %lu %s, Type: %s, readyAt %u, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
						orcs_engine.get_global_cycle(),
                        this->processor_id,
						operation.opcode_address,
                        operation.opcode_number,
                        get_enum_instruction_operation_char(operation.opcode_operation),
						(operation.is_validation) ? "Val" : (operation.is_vectorial_part >= 0) ? "VP" : "Ins",
                        operation.readyAt,
                        this->fetchBuffer.get_size(),
                        this->decodeBuffer.get_size(),
                        reorderBuffer.robUsed);				
			#endif

			if (fetchBuffer.back()->is_vectorial_part < 0) {
				memory_package_t* request = new memory_package_t();

				request->clients.push_back (fetchBuffer.back());
				request->processor_id = this->processor_id;
				request->uop_number = fetchBuffer.back()->opcode_number;
				request->opcode_address = fetchBuffer.back()->opcode_address;
				request->opcode_number = fetchBuffer.back()->opcode_number;
				request->memory_address = fetchBuffer.back()->opcode_address;
				request->memory_size = fetchBuffer.back()->opcode_size;
				request->memory_operation = MEMORY_OPERATION_INST;
				request->is_hive = false;
				request->is_vima = false;
				request->status = PACKAGE_STATE_UNTREATED;
				request->readyAt = orcs_engine.get_global_cycle();
				request->born_cycle = orcs_engine.get_global_cycle();
				request->sent_to_ram = false;
				request->type = INSTRUCTION;
				request->op_count[request->memory_operation]++;

				#if MEMORY_DEBUG
					ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.I\n", orcs_engine.get_global_cycle(), request->opcode_number, request->memory_address , get_enum_memory_operation_char (request->memory_operation))
				#endif

				if (!orcs_engine.cacheManager->searchData(request)) delete request;
			} else {
				fetchBuffer.back()->status = PACKAGE_STATE_READY;
				++vec_on_fetchBuffer;
			}
		}
	}
}

void processor_t::set_registers_bits(opcode_package_t *instr, int32_t num_uops) {
	// Check that is not in the sent state
		if (instr->VR_id >= 0)
		{
			if (instr->is_validation) {
				vectorizer->set_sent(instr->VR_id, instr->will_validate_offset, true);
				vectorizer->set_U(instr->VR_id, instr->will_validate_offset, num_uops);
				vectorizer->set_V(instr->VR_id, instr->will_validate_offset, num_uops);
			} else if (instr->is_vectorial_part >= 0) {
				for (int32_t i=0; i < VECTORIZATION_SIZE; ++i) {
					vectorizer->set_executed(instr->VR_id, i, true);
					vectorizer->add_R(instr->VR_id, i, num_uops);
				}
			}

			vectorizer->vr_control_bits[instr->VR_id].associated_not_decoded--;
		}
		if (instr->will_free >= 0) {
			vectorizer->set_free(instr->will_free, instr->will_free_offset, true);
			vectorizer->set_F(instr->will_free, instr->will_free_offset, num_uops);
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
std::map<std::string, int> instr_cnt;
//int32_t validations = 0;
void processor_t::decode(){
	#if DECODE_DEBUG
		if (DEBUG_STARTED)
			ORCS_PRINTF("Decode Stage\n")
		if (DEBUG_STARTED)
			if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("Opcode to decode %lu %s\n", this->fetchBuffer.front()->opcode_number, this->fetchBuffer.front()->content_to_string2().c_str())
			}
	#endif
	
	uop_package_t new_uop;
	int32_t statusInsert = POSITION_FAIL;
	for (size_t i = 0; i < DECODE_WIDTH; i++)
	{
        if (this->fetchBuffer.is_empty()) {
		    break;
		}

		opcode_package_t *instr = this->fetchBuffer.front();
        instruction_set_t *instr_set = orcs_engine.instruction_set;
        instruction_operation_t instr_op = instr->opcode_operation;
		int32_t uops_created = 0;

        // First instruction must be ready
		if (instr->status != PACKAGE_STATE_READY || instr->readyAt > orcs_engine.get_global_cycle()) {	
			break;
		}

        uint32_t num_uops = 0;

        if ((get_HAS_HIVE() && instr->is_hive) ||
            (get_HAS_VIMA() && instr->is_vima) ||
			(instr->is_validation))
            num_uops += 1;
        else {
            num_uops += instr->is_read + instr->is_read2 + instr->is_write;
            num_uops += (instr_op == INSTRUCTION_OPERATION_BRANCH);

            if (instr_op != INSTRUCTION_OPERATION_BRANCH &&
                instr_op != INSTRUCTION_OPERATION_MEM_LOAD &&
                instr_op != INSTRUCTION_OPERATION_MEM_STORE)
                num_uops += instr_set->uops_per_instruction[instr->instruction_id].size();
        }
		

        // Make sure there's enough space in decodeBuffer
		if (instr->is_vectorial_part < 0) {
			//printf("Decode left: %d\n",DECODE_BUFFER - esc_on_decodeBuffer);			
			if (DECODE_BUFFER - esc_on_decodeBuffer < num_uops) {
				//printf("Decode STALL\n");
				this->add_stall_full_DecodeBuffer();
				//printf("Scalar decode full: %u/%u\n", esc_on_decodeBuffer, DECODE_BUFFER);
				break;
			}
			esc_on_decodeBuffer += num_uops;
		} else {
			if (DECODE_BUFFER_VECTORIZED - vec_on_decodeBuffer < num_uops) {
				this->add_stall_full_DecodeBuffer();
				//printf("Vectorial decode full: %u/%u\n", vec_on_decodeBuffer, DECODE_BUFFER_VECTORIZED);
				break;
			}
			vec_on_decodeBuffer += num_uops;

		}
		
		/*
		if (this->decodeBuffer.get_capacity() - this->decodeBuffer.get_size() < num_uops)
		{
			this->add_stall_full_DecodeBuffer();
			break;
		}*/

		// Count execute instructions
		if (instr->is_validation)
		{
			instr_cnt[std::string(instr->opcode_assembly) + " - Val"]++;
		} else if (instr->is_vectorial_part >= 0) {
			instr_cnt[std::string(instr->opcode_assembly) + " - VP"]++;
		} else {
			instr_cnt[std::string(instr->opcode_assembly) + " - INS"]++;
		}

		// REMOVE
		/*printf("Is_validation: %s\n", (instr->is_validation) ? "true" : "false");
		printf("Is_vec_part: %s\n", (instr->is_vectorial_part >= 0) ? "true" : "false");
		
		printf("---- New opcode: %lu - %d\n", instr->opcode_number, validations);
		printf("---- Instr_op: %d\n", instr_op);*/
		//instr->opcode_number -= validations;

		//printf("Decode counter: %lu -- Received: %lu\n", this->decodeCounter, instr->opcode_number);
		ERROR_ASSERT_PRINTF(this->decodeCounter == instr->opcode_number,
                "Trying decode out-of-order");

		this->decodeCounter++;


		// HIVE
		if (get_HAS_HIVE())
        {
			if (instr_op == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
                instr_op == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
                instr_op == INSTRUCTION_OPERATION_HIVE_FP_MUL ||
                instr_op == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
                instr_op == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
                instr_op == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
                instr_op == INSTRUCTION_OPERATION_HIVE_LOCK ||
                instr_op == INSTRUCTION_OPERATION_HIVE_UNLOCK)
            {
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
                        instr_op,
                        0,
						1,
                        this->LATENCY_MEM_HIVE, this->WAIT_NEXT_MEM_HIVE, &(this->fu_mem_hive),
                        *instr);
				++uops_created;
				new_uop.is_hive = true;
				new_uop.is_vima = false;
				new_uop.hive_read1 = instr->hive_read1;
				new_uop.hive_read2 = instr->hive_read2;
				new_uop.hive_write = instr->hive_write;

				new_uop.updatePackageWait(DECODE_LATENCY);
				new_uop.born_cycle = orcs_engine.get_global_cycle();
				this->total_operations[new_uop.opcode_operation]++;
				statusInsert = this->decodeBuffer.push_back(new_uop);

				#if DECODE_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				#endif

				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
                        "Erro, Tentando decodificar mais uops que o maximo permitido");

				this->fetchBuffer.pop_front();
				set_registers_bits(instr, uops_created);
				return;
			} else if (instr_op == INSTRUCTION_OPERATION_HIVE_LOAD){
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
                        instr_op,
                        instr->read_address,
                        instr->read_size,
                        this->LATENCY_MEM_HIVE, this->WAIT_NEXT_MEM_HIVE, &(this->fu_mem_hive),
                        *instr);

				++uops_created;
				new_uop.is_hive = true;
				new_uop.is_vima = false;
				new_uop.hive_read1    = instr->hive_read1;
				new_uop.read_address  = instr->read_address;
				new_uop.hive_read2    = instr->hive_read2;
				new_uop.read2_address = instr->read2_address;
				new_uop.hive_write    = instr->hive_write;
				new_uop.write_address = instr->write_address;

				new_uop.updatePackageWait(DECODE_LATENCY);
				new_uop.born_cycle = orcs_engine.get_global_cycle();
				this->total_operations[new_uop.opcode_operation]++;
				statusInsert = this->decodeBuffer.push_back(new_uop);

				#if DECODE_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				#endif

				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
                        "Erro, Tentando decodificar mais uops que o maximo permitido");

				this->fetchBuffer.pop_front();
				set_registers_bits(instr, uops_created);
				return;
			} else if (instr_op == INSTRUCTION_OPERATION_HIVE_STORE){
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
                        instr_op,
                        instr->write_address,
                        instr->write_size,
                        this->LATENCY_MEM_HIVE, this->WAIT_NEXT_MEM_HIVE, &(this->fu_mem_hive),
                        *instr);
				++uops_created;
				new_uop.is_hive = true;
				new_uop.is_vima = false;
				new_uop.hive_read1    = instr->hive_read1;
				new_uop.read_address  = instr->read_address;
				new_uop.hive_read2    = instr->hive_read2;
				new_uop.read2_address = instr->read2_address;
				new_uop.hive_write    = instr->hive_write;
				new_uop.write_address = instr->write_address;

				new_uop.updatePackageWait (DECODE_LATENCY);
				new_uop.born_cycle = orcs_engine.get_global_cycle();
				this->total_operations[new_uop.opcode_operation]++;
				statusInsert = this->decodeBuffer.push_back(new_uop);

				#if DECODE_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				#endif

				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
                        "Erro, Tentando decodificar mais uops que o maximo permitido");

				this->fetchBuffer.pop_front();
				set_registers_bits(instr, uops_created);
				return;
			}
		}


		// VIMA
		if (get_HAS_VIMA())
        {
			if (instr_op == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
                instr_op == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
                instr_op == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
                instr_op == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
                instr_op == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
                instr_op == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
                instr_op == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
                instr_op == INSTRUCTION_OPERATION_VIMA_FP_MLA)
            {
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
                        instr_op,
                        0,
                        1,
                        this->LATENCY_MEM_VIMA, this->WAIT_NEXT_MEM_VIMA, &(this->fu_mem_vima),
                        *instr);

				++uops_created;
				new_uop.is_hive = false;
				new_uop.hive_read1 = -1;
				new_uop.hive_read2 = -1;
				new_uop.hive_write = -1;

				new_uop.is_vima = true;
				new_uop.read_address  = instr->read_address;
				new_uop.read2_address = instr->read2_address;
				new_uop.write_address = instr->write_address;

				new_uop.updatePackageWait (DECODE_LATENCY);
				new_uop.born_cycle = orcs_engine.get_global_cycle();
				this->total_operations[new_uop.opcode_operation]++;
				statusInsert = this->decodeBuffer.push_back(new_uop);

				#if DECODE_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				#endif

				#if VIMA_DEBUGG
					ORCS_PRINTF ("%lu Processor decode(): VIMA instruction %lu decoded!\n",
                            orcs_engine.get_global_cycle(), this->fetchBuffer.front()->opcode_number)
				#endif

				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
                        "Erro, Tentando decodificar mais uops que o maximo permitido");

				this->fetchBuffer.pop_front();
				set_registers_bits(instr, uops_created);
				return;
			}
		}

		if (instr->is_validation == false) {
			// =====================
			// Decode Read 1
			// =====================
			if (instr->is_read)
			{
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
        	            INSTRUCTION_OPERATION_MEM_LOAD,
        	            instr->read_address,
        	            instr->read_size,
        	            this->LATENCY_MEM_LOAD, this->WAIT_NEXT_MEM_LOAD, &(this->fu_mem_load),
        	            *this->fetchBuffer.front());

				++uops_created;
        	    // If op is not load, clear registers
				if (instr_op != INSTRUCTION_OPERATION_MEM_LOAD)
				{
					// ===== Read Regs =============================================
					/// Clear RRegs
        	        for (uint32_t i = 0; i < MAX_REGISTERS; i++)
					{
        	            new_uop.read_regs[i] = POSITION_FAIL;
					}

					/// Insert BASE and INDEX into RReg
					new_uop.read_regs[0] = instr->base_reg;
					new_uop.read_regs[1] = instr->index_reg;

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
				new_uop.born_cycle = orcs_engine.get_global_cycle();
				this->total_operations[new_uop.opcode_operation]++;
				statusInsert = this->decodeBuffer.push_back(new_uop);

				#if DECODE_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				#endif

				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
        	            "Erro, Tentando decodificar mais uops que o maximo permitido");
			}

			// =====================
			// Decode Read 2
			// =====================
			if (instr->is_read2)
			{
				new_uop.package_clean();
				new_uop.opcode_to_uop(this->uopCounter++,
        	            INSTRUCTION_OPERATION_MEM_LOAD,
        	            instr->read2_address,
        	            instr->read2_size,
        	            this->LATENCY_MEM_LOAD, this->WAIT_NEXT_MEM_LOAD, &(this->fu_mem_load),
        	            *instr);

				++uops_created;				
        	    // If op is not load, clear registers
				if (instr_op != INSTRUCTION_OPERATION_MEM_LOAD)
				{
					// ===== Read Regs =============================================
					/// Clear RRegs
					for (uint32_t i = 0; i < MAX_REGISTERS; i++)
					{
						new_uop.read_regs[i] = POSITION_FAIL;
					}

					/// Insert BASE and INDEX into RReg
					new_uop.read_regs[0] = instr->base_reg;
					new_uop.read_regs[1] = instr->index_reg;

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
				new_uop.born_cycle = orcs_engine.get_global_cycle();
				this->total_operations[new_uop.opcode_operation]++;
				statusInsert = this->decodeBuffer.push_back(new_uop);

				#if DECODE_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
				#endif

				ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
        	            "Erro, Tentando decodificar mais uops que o maximo permitido");
			}

			// =====================
			// Decode ALU Operation
			// =====================
			if (instr_op != INSTRUCTION_OPERATION_BRANCH &&
				instr_op != INSTRUCTION_OPERATION_MEM_LOAD &&
				instr_op != INSTRUCTION_OPERATION_MEM_STORE)
			{
        	    uint32_t instr_id = instr->instruction_id;
        	    std::vector<uint32_t> &uops = instr_set->uops_per_instruction[instr_id];

        	    // Iterate over uops from instruction
        	    for (uint32_t uop_idx : uops)
        	    {
        	        uop_info_t uop = instr_set->uops[uop_idx];

        	        new_uop.package_clean();
        	        new_uop.opcode_to_uop(this->uopCounter++,
        	            instr_op,
        	            0, 0,
        	            uop.latency,
        	            this->functional_units[uop.fu_id].wait_next,
        	            &(this->functional_units[uop.fu_id]),
        	            *instr);

					++uops_created;
        	        if (instr->is_read || instr->is_read2)
        	        {
        	            // ===== Read Regs =============================================
        	            //registers /258 aux onde pos[i] = fail
        	            bool inserted_258 = false;
        	            for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        	                if (new_uop.read_regs[i] == POSITION_FAIL) {
        	                    new_uop.read_regs[i] = 258;
        	                    inserted_258 = true;
        	                    break;
        	                }
        	            }

        	            ERROR_ASSERT_PRINTF(inserted_258,
        	                    "Could not insert register_258, all MAX_REGISTERS(%d) used.\n",
        	                    MAX_REGISTERS);
        	        }

        	        if (instr->is_write)
        	        {
        	            // ===== Write Regs =============================================
        	            //registers /258 aux onde pos[i] = fail
        	            bool inserted_258 = false;
        	            for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        	                if (new_uop.write_regs[i] == POSITION_FAIL) {
        	                    new_uop.write_regs[i] = 258;
        	                    inserted_258 = true;
        	                    break;
        	                }
        	            }

        	            ERROR_ASSERT_PRINTF(inserted_258,
        	                    "Could not insert register_258, all MAX_REGISTERS(%d) used.\n",
        	                    MAX_REGISTERS);
        	        }

        	        new_uop.updatePackageWait(DECODE_LATENCY);
        	        new_uop.born_cycle = orcs_engine.get_global_cycle();
        	        this->total_operations[new_uop.opcode_operation]++;
        	        statusInsert = this->decodeBuffer.push_back(new_uop);

        	        #if DECODE_DEBUG
					if (DEBUG_STARTED)
        	            ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
        	        #endif

        	        ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
        	                "Erro, Tentando decodificar mais uops que o maximo permitido");
        	    }
			}
		} else { // Se é uma validação
				// Envia para commit
			// REMOVE
			new_uop.package_clean();
        	new_uop.opcode_to_uop(this->uopCounter++, // 0
        	    instr_op,
        	    0, 0,
        	    0,
        	    this->fu_validation.wait_next,
        	    &(this->fu_validation),
        	    *instr);

			++uops_created;
			new_uop.read_regs[0] = 258;

        	if (instr->is_write)
        	{
        	    // ===== Write Regs =============================================
        	    //registers /258 aux onde pos[i] = fail
        	    bool inserted_258 = false;
        	    for (uint32_t i = 0; i < MAX_REGISTERS; i++) {
        	        if (new_uop.write_regs[i] == POSITION_FAIL) {
        	            new_uop.write_regs[i] = 258;
        	            inserted_258 = true;
        	            break;
        	        }
        	    }
        	    ERROR_ASSERT_PRINTF(inserted_258,
        	            "Could not insert register_258, all MAX_REGISTERS(%d) used.\n",
        	            MAX_REGISTERS);
        	}
			new_uop.updatePackageWait(0);
        	new_uop.born_cycle = orcs_engine.get_global_cycle();
        	this->total_operations[new_uop.opcode_operation]++;
			// REMOVE
        	statusInsert = this->decodeBuffer.push_back(new_uop);

			// Send to commit
			//this->reorderBuffer[pos_rob].uop = new_uop;
			//this->reorderBuffer[pos_rob].stage = PROCESSOR_STAGE_COMMIT;
			//this->reorderBuffer[pos_rob].uop.status = PACKAGE_STATE_READY;
			//this->reorderBuffer[pos_rob].uop.readyAt = orcs_engine.get_global_cycle();
			//validations++;
			//this->decodeCounter--;
			//vectorizer->new_commit(&new_uop);

        	#if DECODE_DEBUG
			if (DEBUG_STARTED)
        	    ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
        	#endif

        	//ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
        	//        "Erro, Tentando decodificar mais uops que o maximo permitido");
		}
		// =====================
		//Decode Branch
		// =====================
		if (instr_op == INSTRUCTION_OPERATION_BRANCH)
		{
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
                    INSTRUCTION_OPERATION_BRANCH,
                    0, 0,
                    this->LATENCY_INTEGER_ALU, this->WAIT_NEXT_INT_ALU, &(this->functional_units[0]),
                    *instr);
			++uops_created;
			if (instr->is_read || instr->is_read2)
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

				ERROR_ASSERT_PRINTF(inserted_258,
                        "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS);
			}

			if (instr->is_write)
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

				ERROR_ASSERT_PRINTF(inserted_258,
                        "Todos Max regs usados. %u \n", MAX_REGISTERS);
			}

			new_uop.updatePackageWait(DECODE_LATENCY);
			new_uop.born_cycle = orcs_engine.get_global_cycle();
			this->total_operations[new_uop.opcode_operation]++;
			statusInsert = this->decodeBuffer.push_back(new_uop);

			#if DECODE_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			#endif

			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
                    "Erro, Tentando decodificar mais uops que o maximo permitido")
		}

		// =====================
		//Decode Write
		// =====================
		if (instr->is_write && (instr->is_validation == false))
		{
			new_uop.package_clean();
			new_uop.opcode_to_uop(this->uopCounter++,
                    INSTRUCTION_OPERATION_MEM_STORE,
                    instr->write_address,
                    instr->write_size,
                    this->LATENCY_MEM_STORE, this->WAIT_NEXT_MEM_STORE, &(this->fu_mem_store),
                    *instr);

			++uops_created;
			//
			if (instr_op != INSTRUCTION_OPERATION_MEM_STORE)
            {
				bool inserted_258 = false;
				for (uint32_t i = 0; i < MAX_REGISTERS; i++){
					if (new_uop.read_regs[i] == POSITION_FAIL){
						new_uop.read_regs[i] = 258;
						inserted_258 = true;
						break;
					}
				}

				ERROR_ASSERT_PRINTF(inserted_258,
                        "Could not insert register_258, all MAX_REGISTERS(%d) used.", MAX_REGISTERS)

				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
			}

			new_uop.updatePackageWait(DECODE_LATENCY);
			new_uop.born_cycle = orcs_engine.get_global_cycle();
			this->total_operations[new_uop.opcode_operation]++;
			statusInsert = this->decodeBuffer.push_back(new_uop);

			#if DECODE_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
			#endif

			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL,
                    "Erro, Tentando decodificar mais uops que o maximo permitido")
		}

		#if PROCESSOR_DEBUG
			ORCS_PRINTF("%lu processor %lu decode(): addr %lu uop %lu %s, Type: %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
					orcs_engine.get_global_cycle(),
                    this->processor_id,
					new_uop.opcode_address,
                    new_uop.uop_number,
                    get_enum_instruction_operation_char(new_uop.uop_operation),
					(new_uop.is_validation) ? "Val" : (new_uop.is_vectorial_part >= 0) ? "VP" : "Ins",
                    new_uop.readyAt,
                    this->fetchBuffer.get_size(),
                    this->decodeBuffer.get_size(),
                    reorderBuffer.robUsed);
		#endif
	

		set_registers_bits(instr, uops_created);

		if (new_uop.is_vectorial_part >= 0) {
			--vec_on_fetchBuffer;
			--i; // Parte vetorial não conta pq começa aqui
		}
		this->fetchBuffer.pop_front();
		
	}
}

// ============================================================================
void processor_t::update_registers(reorder_buffer_line_t *new_rob_line){
	/// Control the Register Dependency - Register READ
	for (uint32_t k = 0; k < MAX_REGISTERS; k++) {
		if (new_rob_line->uop.read_regs[k] < 0) break;
		uint32_t read_register = new_rob_line->uop.read_regs[k];
		ERROR_ASSERT_PRINTF(read_register < RAT_SIZE,
                "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, RAT_SIZE);

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
		ERROR_ASSERT_PRINTF(write_register < RAT_SIZE,
                "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);

		this->register_alias_table[write_register] = new_rob_line;
	}
}

// ============================================================================
void processor_t::rename(){
	#if RENAME_DEBUG
	if (DEBUG_STARTED)
		ORCS_PRINTF("Rename Stage\n")
	#endif
	size_t i;
	int32_t pos_rob, pos_mob;

	for (i = 0; i < RENAME_WIDTH; i++)
	{
		memory_order_buffer_line_t *mob_line = NULL;

		// Checando se há uop decodificado, se está pronto, e se o ciclo de pronto
		// é maior ou igual ao atual
		if (this->decodeBuffer.is_empty() ||
            this->decodeBuffer.front()->status != PACKAGE_STATE_WAIT ||
            this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle()) {
			break;
		}
	
		ERROR_ASSERT_PRINTF(this->decodeBuffer.front()->uop_number == this->renameCounter, "Erro, renomeio incorreto\n");
		bool is_vectorial_part = (this->decodeBuffer.front()->is_vectorial_part >= 0);
		//=======================
		// Memory Operation Read
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD && (this->decodeBuffer.front()->is_validation == false))
		{

			if(	(is_vectorial_part == false) && (this->memory_order_buffer_read_used>=MOB_READ || reorderBuffer.robUsed>=ROB_SIZE) ) {
				break;
			} else if ( (is_vectorial_part == true) && (this->memory_order_buffer_vectorial_used>=MOB_VECTORIAL || vectorialReorderBuffer.robUsed>=ROB_VECTORIAL_SIZE) ) {
				break;
			}
			
			pos_mob = (is_vectorial_part == false) ? this->search_position_mob_read()
												   : this->search_position_mob_vectorial();
			if (pos_mob == POSITION_FAIL)
			{
				#if RENAME_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				#endif
				if (is_vectorial_part == false) {
				this->add_stall_full_MOB_Read();
				} else {
					this->add_stall_full_MOB_VET();
				}
				break;
			}

			#if RENAME_DEBUG
			if (DEBUG_STARTED){		
				if (is_vectorial_part) {
					ORCS_PRINTF("Get_Position_MOB_VECTORIAL %d\n",pos_mob);
				} else {
					ORCS_PRINTF("Get_Position_MOB_READ %d\n",pos_mob);
				}
			}
			#endif

			mob_line = (is_vectorial_part == false) ? &this->memory_order_buffer_read[pos_mob]
													: &this->memory_order_buffer_vectorial[pos_mob];
			
		}

		//=======================
		// Memory Operation Write
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
			if(	this->memory_order_buffer_write_used>=MOB_READ || reorderBuffer.robUsed>=ROB_SIZE ) {
				break;
			}

			
			pos_mob = this->search_position_mob_write();
			if (pos_mob == POSITION_FAIL) {
				#if RENAME_DEBUG
				if (DEBUG_STARTED)
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				#endif
				this->add_stall_full_MOB_Write();
				break;
			}

			#if RENAME_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("Get_Position_MOB_WRITE %d\n",pos_mob);
			#endif

			mob_line = &this->memory_order_buffer_write[pos_mob];
		}

		//=======================
		// Memory Operation HIVE
		//=======================
		if (get_HAS_HIVE()){
            instruction_operation_t uop_operation = this->decodeBuffer.front()->uop_operation;

			if (uop_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_LOAD ||
                uop_operation == INSTRUCTION_OPERATION_HIVE_STORE)
            {
				if (this->memory_order_buffer_hive_used>=MOB_HIVE || reorderBuffer.robUsed>=ROB_SIZE)
                    break;

				pos_mob = this->search_position_mob_hive();
				if (pos_mob == POSITION_FAIL) {
					this->add_stall_full_MOB_Read();
					break;
				}

				mob_line = &this->memory_order_buffer_hive[pos_mob];
				//ORCS_PRINTF("reservando espaço no MOB para instrução %s %lu\n",
                //            get_enum_instruction_operation_char(this->decodeBuffer.front()->opcode_operation),
                //            this->decodeBuffer.front()->opcode_number);
			}
		}

		//=======================
		// Memory Operation VIMA
		//=======================
		if (get_HAS_VIMA()){
            instruction_operation_t uop_operation = this->decodeBuffer.front()->uop_operation;

			if (uop_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
                uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA)
            {
				if (this->memory_order_buffer_vima_used >= MOB_VIMA || reorderBuffer.robUsed >= ROB_SIZE)
                    break;
				else {
					#if VIMA_DEBUGG
						ORCS_PRINTF ("%lu Processor rename(): memory_order_buffer_vima used = %u.\n",
                                orcs_engine.get_global_cycle(),
                                this->memory_order_buffer_vima_used);
					#endif
				}

				pos_mob = this->search_position_mob_vima();
				if (pos_mob == POSITION_FAIL) {
					this->add_stall_full_MOB_Read();
					break;
				}

				mob_line = &this->memory_order_buffer_vima[pos_mob];
			}
		}

		//=======================
		// Verificando se tem espaco no ROB se sim vamos inserir
		//=======================
		pos_rob = this->searchPositionROB((is_vectorial_part == false) ? &reorderBuffer : &vectorialReorderBuffer);
		//printf("ROB USED: %d\n", reorderBuffer.robUsed);
		if (pos_rob == POSITION_FAIL)
		{
			#if RENAME_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("Stall_ROB_Full\n")
			#endif
			/*ORCS_PRINTF("%lu - Stall_ROB_Full common: %u/%u -- vectorial: %u/%u\n", 
						orcs_engine.get_global_cycle(), 
						reorderBuffer.robUsed, reorderBuffer.SIZE, 
						vectorialReorderBuffer.robUsed, 
						vectorialReorderBuffer.SIZE);*/
			if (is_vectorial_part == false) {
				//printf("ROB STALL\n");
				this->add_stall_full_ROB();
			} else {
				this->add_stall_full_ROB_VET();
			}
			break;
		}

		// ===============================================
		// Inserting on ROB
		// ===============================================
		reorder_buffer_line_t *rob_line = NULL;
		if (is_vectorial_part == false) {
			rob_line = &this->reorderBuffer.reorderBuffer[pos_rob];
		} else {
			rob_line = &this->vectorialReorderBuffer.reorderBuffer[pos_rob];
		}

		rob_line->uop = *this->decodeBuffer.front();

		//remove uop from decodebuffer
		if (is_vectorial_part == false) {
			--esc_on_decodeBuffer;
		} else {
			--vec_on_decodeBuffer;
			--i; // Não conta no decode
		}
		this->decodeBuffer.front()->package_clean();
		this->decodeBuffer.pop_front();
		this->renameCounter++;


		// =======================
		// Setting controls to ROB.
		// =======================
		rob_line->stage = PROCESSOR_STAGE_RENAME;
		rob_line->uop.updatePackageWait(RENAME_LATENCY + DISPATCH_LATENCY);
		rob_line->mob_ptr = mob_line;
		rob_line->processor_id = this->processor_id;
		
		// =======================
		// Making registers dependences
		// =======================
		if(is_vectorial_part == false) this->update_registers(rob_line);



		#if RENAME_DEBUG
		if (DEBUG_STARTED) {
			ORCS_PRINTF("Rename [%s] Opcode number: %lu %s\n", 
			(rob_line->uop.is_vectorial_part >= 0) ? "Vec" :
			(rob_line->uop.is_validation) ? "Val" : "Ins",
			rob_line->uop.opcode_number,
			 rob_line->content_to_string().c_str())
		}
		#endif

		// =======================
		// Insert into Reservation Station
		// =======================
		// ONE_LOAD
		// Faz a validação ir direto ao commit
		if (rob_line->uop.is_validation == false) {
			if (is_vectorial_part == false)
				this->unified_reservation_station.push_back(rob_line);
			else 
				this->unified_vectorial_reservation_station.push_back(rob_line);
		} else {
			rob_line->stage = PROCESSOR_STAGE_COMMIT;
			rob_line->uop.updatePackageReady(COMMIT_LATENCY);

		}


		// =======================
		// Insert into MOB.
		// =======================
		if (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD  && (rob_line->uop.is_validation == false))
		{
			#if RENAME_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("Mem Load\n")
			#endif
			rob_line->mob_ptr->opcode_address = rob_line->uop.opcode_address;
			rob_line->mob_ptr->memory_address = rob_line->uop.memory_address;
			rob_line->mob_ptr->memory_size = rob_line->uop.memory_size;
			rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_READ;
			rob_line->mob_ptr->status = PACKAGE_STATE_WAIT;
			rob_line->mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			rob_line->mob_ptr->uop_number = rob_line->uop.uop_number;
			rob_line->mob_ptr->processor_id = this->processor_id;
			#if MEMORY_DEBUG
				ORCS_PRINTF("[ROBL] %lu {%lu} %lu %s added to reorder order buffer (Ready: %lu).\n",
                        orcs_engine.get_global_cycle(),
						rob_line->uop.opcode_number,
                        rob_line->mob_ptr->memory_address,
                        get_enum_memory_operation_char (rob_line->mob_ptr->memory_operation),
						rob_line->mob_ptr->readyToGo);
			#endif
		}
		else if (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
        {
			#if RENAME_DEBUG
			if (DEBUG_STARTED)
				ORCS_PRINTF("Mem Store\n")
			#endif
			rob_line->mob_ptr->opcode_address = rob_line->uop.opcode_address;
			rob_line->mob_ptr->memory_address = rob_line->uop.memory_address;
			rob_line->mob_ptr->memory_size = rob_line->uop.memory_size;
			rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_WRITE;
			rob_line->mob_ptr->status = PACKAGE_STATE_WAIT;
			rob_line->mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			rob_line->mob_ptr->uop_number = rob_line->uop.uop_number;
			rob_line->mob_ptr->processor_id = this->processor_id;
			#if MEMORY_DEBUG
				ORCS_PRINTF("[ROBL] %lu {%lu} %lu %s added to reorder order buffer.\n",
                        orcs_engine.get_global_cycle(),
						rob_line->uop.opcode_number,
                        rob_line->mob_ptr->memory_address,
                        get_enum_memory_operation_char (rob_line->mob_ptr->memory_operation));
			#endif
		}
		else if (this->get_HAS_HIVE() &&
                (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOAD ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_STORE ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL))
        {
			rob_line->mob_ptr->is_hive = true;
			rob_line->mob_ptr->is_hive = false;
			rob_line->mob_ptr->opcode_address = rob_line->uop.opcode_address;
			rob_line->mob_ptr->hive_read1 = rob_line->uop.hive_read1;
			rob_line->mob_ptr->hive_read2 = rob_line->uop.hive_read2;
			rob_line->mob_ptr->hive_write = rob_line->uop.hive_write;
			rob_line->mob_ptr->memory_size = rob_line->uop.memory_size;

			switch (rob_line->uop.uop_operation){
				case INSTRUCTION_OPERATION_HIVE_LOCK:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_LOCK;
					break;
				case INSTRUCTION_OPERATION_HIVE_UNLOCK:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_UNLOCK;
					break;
				case INSTRUCTION_OPERATION_HIVE_LOAD:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_LOAD;
					rob_line->mob_ptr->memory_address = rob_line->uop.read_address;
					break;
				case INSTRUCTION_OPERATION_HIVE_STORE:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_STORE;
					rob_line->mob_ptr->memory_address = rob_line->uop.write_address;
					break;
				case INSTRUCTION_OPERATION_HIVE_INT_ALU:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_INT_ALU;
					break;
            	case INSTRUCTION_OPERATION_HIVE_INT_MUL:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_INT_MUL;
					break;
            	case INSTRUCTION_OPERATION_HIVE_INT_DIV:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_INT_DIV;
					break;
            	case INSTRUCTION_OPERATION_HIVE_FP_ALU:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_FP_ALU;
					break;
            	case INSTRUCTION_OPERATION_HIVE_FP_MUL:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_FP_MUL;
					break;
            	case INSTRUCTION_OPERATION_HIVE_FP_DIV:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_HIVE_FP_DIV;
					break;
				default:
					break;
			}
			rob_line->mob_ptr->status = PACKAGE_STATE_WAIT;
			rob_line->mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			rob_line->mob_ptr->uop_number = rob_line->uop.uop_number;
			rob_line->mob_ptr->processor_id = this->processor_id;
		}
		else if (this->get_HAS_VIMA() &&
                (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA))
        {
			rob_line->mob_ptr->is_hive = false;
			rob_line->mob_ptr->is_vima = true;
			rob_line->mob_ptr->vima_read1 = rob_line->uop.read_address;
			rob_line->mob_ptr->vima_read2 = rob_line->uop.read2_address;
			rob_line->mob_ptr->vima_write = rob_line->uop.write_address;
			rob_line->mob_ptr->opcode_address = rob_line->uop.opcode_address;
			rob_line->mob_ptr->memory_size = rob_line->uop.memory_size;

			switch (rob_line->uop.uop_operation){
				case INSTRUCTION_OPERATION_VIMA_INT_ALU:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_ALU;
					break;
            	case INSTRUCTION_OPERATION_VIMA_INT_MUL:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_MUL;
					break;
            	case INSTRUCTION_OPERATION_VIMA_INT_DIV:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_DIV;
					break;
            	case INSTRUCTION_OPERATION_VIMA_FP_ALU:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_ALU;
					break;
            	case INSTRUCTION_OPERATION_VIMA_FP_MUL:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_MUL;
					break;
            	case INSTRUCTION_OPERATION_VIMA_FP_DIV:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_DIV;
					break;
				case INSTRUCTION_OPERATION_VIMA_INT_MLA:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_INT_MLA;
					break;
				case INSTRUCTION_OPERATION_VIMA_FP_MLA:
                	rob_line->mob_ptr->memory_operation = MEMORY_OPERATION_VIMA_FP_MLA;
					break;
				default:
					break;
			}
			rob_line->mob_ptr->status = PACKAGE_STATE_WAIT;
			rob_line->mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			rob_line->mob_ptr->uop_number = rob_line->uop.uop_number;
			rob_line->mob_ptr->processor_id = this->processor_id;
		}

		//linking rob and mob
		if (rob_line->uop.is_validation) {
			// Nada.
		} else if (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD ||
			rob_line->uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			
			mob_line->rob_ptr = rob_line;
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->make_memory_dependences(rob_line->mob_ptr);
			}
		} else if (this->get_HAS_HIVE() &&
                (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOAD ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_STORE ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_LOCK ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_UNLOCK ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_INT_MUL ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HIVE_FP_MUL))
		{
			mob_line->rob_ptr = rob_line;
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->make_memory_dependences(rob_line->mob_ptr);
			}
		} else if (this->get_HAS_VIMA() &&
                (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MUL ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_ALU ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_DIV ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MUL ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_MLA ||
                rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_FP_MLA))
		{
			mob_line->rob_ptr = rob_line;
			#if VIMA_DEBUGG
				ORCS_PRINTF ("%lu Processor rename(): VIMA instruction %lu uop %lu renamed!\n",
                        orcs_engine.get_global_cycle(),
                        rob_line->uop.opcode_number,
                        rob_line->uop.uop_number);
			#endif

			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->make_memory_dependences(rob_line->mob_ptr);
			}
		}

		#if PROCESSOR_DEBUG
			ORCS_PRINTF ("%lu processor %lu rename(): uop %lu %s, Type: %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
					orcs_engine.get_global_cycle(),
                    this->processor_id,
                    rob_line->uop.uop_number,
                    get_enum_instruction_operation_char(rob_line->uop.uop_operation),
                    (rob_line->uop.is_validation) ? "Val" : (rob_line->uop.is_vectorial_part >= 0) ? "VP" : "Ins",
					rob_line->uop.readyAt,
                    this->fetchBuffer.get_size(),
                    this->decodeBuffer.get_size(),
                    reorderBuffer.robUsed)
		#endif


	} //end for
}

// ============================================================================
void processor_t::dispatch(){
    uint32_t total_dispatched = 0;

    for (auto &fu : this->functional_units) {
        fu.dispatch_cnt = 0;
    }

    this->fu_mem_load.dispatch_cnt = 0;
    this->fu_mem_store.dispatch_cnt = 0;

    if (get_HAS_VIMA()) {
        this->fu_mem_vima.dispatch_cnt = 0;
	}
    if (get_HAS_HIVE()) {
        this->fu_mem_hive.dispatch_cnt = 0;
	}
	this->fu_validation.dispatch_cnt = 0;
	for (uint32_t type = 0; type < 2; ++type) {
		total_dispatched = 0; // Pode despachar mais vetoriais
		container_ptr_reorder_buffer_line_t *urs = (type == 0) ? &this->unified_reservation_station
															   : &this->unified_vectorial_reservation_station;
    	container_ptr_reorder_buffer_line_t *ufs = (type == 0) ? &this->unified_functional_units
															   : &this->unified_vectorial_functional_units;
		
		for (uint32_t i = 0; i < urs->size() && i < UNIFIED_RS; i++)
    	{
    	    //pointer to entry
    	    reorder_buffer_line_t *rob_line = (*urs)[i];
    	    uop_package_t *uop = &(rob_line->uop);


			#if DISPATCH_DEBUG
    	        if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
    	            ORCS_PRINTF("cycle %lu\n", orcs_engine.get_global_cycle())
    	            ORCS_PRINTF("=================\n")
    	            ORCS_PRINTF("Unified Reservations Station on use: %lu\n",urs->size())
    	            ORCS_PRINTF("Trying Dispatch %s - %s\n", (rob_line->uop.is_validation) ? "Val" : (rob_line->uop.is_vectorial_part >= 0) ? "VP" : "Ins", rob_line->content_to_string().c_str())
    	            ORCS_PRINTF("=================\n")
    	        }
			#endif

    	    if (total_dispatched >= DISPATCH_WIDTH){
    	        break;
    	    }

    	    if (VIMA_EXCEPT) {
    	        if	(rob_line->uop.uop_operation == INSTRUCTION_OPERATION_VIMA_INT_ALU ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_INT_MUL ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_INT_DIV ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_FP_ALU ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_FP_MUL ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_FP_DIV ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_INT_MLA ||
    	             rob_line->uop.uop_operation ==  INSTRUCTION_OPERATION_VIMA_FP_MLA)
    	        {
    	            rob_line->wait_reg_deps_number = 0;
    	        }
    	    }

    	    if ((uop->readyAt <= orcs_engine.get_global_cycle()) && (rob_line->wait_reg_deps_number == 0))
    	    {
    	        ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_WAIT,
    	                "Error, uop not ready being dispatched\n %s\n",
    	                rob_line->content_to_string().c_str());

    	        ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_RENAME,
    	                "Error, uop not in Rename to rename stage\n %s\n",
    	                rob_line->content_to_string().c_str());

    	        //if dispatched
    	        bool dispatched = false;

    	        if (rob_line->uop.uop_operation == INSTRUCTION_OPERATION_BARRIER ||
    	            rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HMC_ROA ||
    	            rob_line->uop.uop_operation == INSTRUCTION_OPERATION_HMC_ROWA ||
    	            rob_line->uop.uop_operation == INSTRUCTION_OPERATION_LAST)
    	        {
    	            ERROR_PRINTF("Invalid instruction LAST||BARRIER||HMC_ROA||HMC_ROWA being dispatched.\n");
    	            continue;
    	        }

    	        functional_unit_t *fu = uop->functional_unit;
    	        if (fu->dispatch_cnt < fu->size)
    	        {
    	            for (uint8_t k = 0; k < fu->size; ++k)
    	            {
    	                if (fu->slot[k] <= orcs_engine.get_global_cycle())
    	                {
    	                    fu->slot[k] = orcs_engine.get_global_cycle() + fu->wait_next;
    	                    fu->dispatch_cnt++;
    	                    dispatched = true;
    	                    rob_line->stage = PROCESSOR_STAGE_EXECUTION;
							uop->updatePackageWait(uop->latency);

    	                    break;
    	                }
    	            }
    	        }


    	        //remover os postos em execucao aqui
    	        if (dispatched == true)
    	        {

				    #if PROCESSOR_DEBUG
    	                if(orcs_engine.get_global_cycle()>WAIT_CYCLE) {
    	                    ORCS_PRINTF("Dispatched %s\n", rob_line->content_to_string().c_str())
    	                    ORCS_PRINTF("===================================================================\n")
    	                }
    	            #endif

    	            // update Dispatched
    	            total_dispatched++;
    	            // insert on FUs waiting structure
    	            ufs->push_back(rob_line);
    	            // remove from reservation station
    	            urs->erase(urs->begin() + i);
    	            i--;
    	        } //end if dispatched

				#if PROCESSOR_DEBUG
    	            ORCS_PRINTF ("%lu processor %lu dispatch(): uop %lu %s, Type: %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
							orcs_engine.get_global_cycle(),
    	                    this->processor_id, rob_line->uop.uop_number,
    	                    get_enum_instruction_operation_char (rob_line->uop.uop_operation),
							(rob_line->uop.is_validation) ? "Val" : (rob_line->uop.is_vectorial_part >= 0) ? "VP" : "Ins",
    	                    rob_line->uop.readyAt, this->fetchBuffer.get_size(), this->decodeBuffer.get_size(), reorderBuffer.robUsed);
    	        #endif


    	    } //end if robline is ready
    	} //end for
	}
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
			#if DEBUG
				ORCS_PRINTF ("Processor clean_mob_hive(): HIVE instruction %lu %s, %u!\n", this->memory_order_buffer_hive[pos].uop_number, get_enum_processor_stage_char (this->memory_order_buffer_hive[pos].rob_ptr->stage), this->memory_order_buffer_hive[pos].readyAt)
			#endif
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
			#if VIMA_DEBUGG
				ORCS_PRINTF ("%lu Processor clean_mob_vima(): memory_vima_executed %u, processor %lu ", orcs_engine.get_global_cycle(), this->memory_vima_executed, this->processor_id)
			#endif
			this->memory_order_buffer_vima[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_vima[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_vima[pos].processed=true;
			this->memory_vima_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_vima[pos].rob_ptr);
			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->solve_memory_dependences(&this->memory_order_buffer_vima[pos]);
			}
			#if VIMA_DEBUGG
				ORCS_PRINTF ("VIMA instruction %lu %s, %u!\n", this->memory_order_buffer_vima[pos].uop_number, get_enum_processor_stage_char (this->memory_order_buffer_vima[pos].rob_ptr->stage), this->memory_order_buffer_vima[pos].readyAt)
			#endif
		}
		pos++;
		if(pos >= MOB_VIMA) pos = 0;
	}
}

void processor_t::clean_mob_vectorial(){
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	uint32_t pos = this->memory_order_buffer_vectorial_start;
	for (uint8_t i = 0; i < this->memory_order_buffer_vectorial_used; i++)
    {
		if (DEBUG_STARTED)
			printf("Next mob vectorial: uop: %lu Status: %s ReadyAt: %u Processed %s\n",
				this->memory_order_buffer_vectorial[pos].uop_number,
				get_enum_package_state_char(this->memory_order_buffer_vectorial[pos].status),
				this->memory_order_buffer_vectorial[pos].readyAt,
				this->memory_order_buffer_vectorial[pos].processed ? "true" : "false");
		if (this->memory_order_buffer_vectorial[pos].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_vectorial[pos].readyAt <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_vectorial[pos].processed == false)
        {
			#if MEMORY_DEBUG
				ORCS_PRINTF ("[MOBL] %lu %lu %s removed from memory order buffer | %s | readyAt = %u.\n",
                        orcs_engine.get_global_cycle(),
                        memory_order_buffer_vectorial[pos].memory_address,
                        get_enum_memory_operation_char (memory_order_buffer_vectorial[pos].memory_operation),
                        get_enum_package_state_char(this->memory_order_buffer_vectorial[pos].status),
                        this->memory_order_buffer_vectorial[pos].readyAt);
			#endif

			ERROR_ASSERT_PRINTF(this->memory_order_buffer_vectorial[pos].uop_executed == true,
                    "Removing memory read before being executed.\n");

			ERROR_ASSERT_PRINTF(this->memory_order_buffer_vectorial[pos].wait_mem_deps_number == 0,
                    "Number of memory dependencies should be zero.\n %s\n",
                    this->memory_order_buffer_vectorial[i].rob_ptr->content_to_string().c_str());

			#if EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("\nSolving %s\n\n", this->memory_order_buffer_vectorial[pos].rob_ptr->content_to_string().c_str())
				}
			#endif

			this->memory_order_buffer_vectorial[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_vectorial[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_vectorial[pos].processed=true;
			this->memory_vectorial_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_vectorial[pos].rob_ptr);

			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->solve_memory_dependences(&this->memory_order_buffer_vectorial[pos]);
			}

			if(this->memory_order_buffer_vectorial[pos].waiting_DRAM){
				ERROR_ASSERT_PRINTF(this->request_DRAM > 0,"ERRO, Contador negativo Waiting DRAM\n");
				#if EXECUTE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("\nReducing DRAM COUNTER\n\n");
					}
				#endif

				this->request_DRAM--;
			}
		}

		pos++;
		if (pos >= MOB_VECTORIAL) pos = 0;
	}
}

void processor_t::clean_mob_read(){
	// ==================================
	// verificar leituras prontas no ciclo,
	// remover do MOB e atualizar os registradores,
	// ==================================
	uint32_t pos = this->memory_order_buffer_read_start;
	for (uint8_t i = 0; i < this->memory_order_buffer_read_used; i++)
    {
		if (this->memory_order_buffer_read[pos].status == PACKAGE_STATE_READY &&
			this->memory_order_buffer_read[pos].readyAt <= orcs_engine.get_global_cycle() &&
			this->memory_order_buffer_read[pos].processed == false)
        {
			#if MEMORY_DEBUG
				ORCS_PRINTF ("[MOBL] %lu %lu %s removed from memory order buffer | %s | readyAt = %u.\n",
                        orcs_engine.get_global_cycle(),
                        memory_order_buffer_read[pos].memory_address,
                        get_enum_memory_operation_char (memory_order_buffer_read[pos].memory_operation),
                        get_enum_package_state_char(this->memory_order_buffer_read[pos].status),
                        this->memory_order_buffer_read[pos].readyAt);
			#endif

			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[pos].uop_executed == true,
                    "Removing memory read before being executed.\n");

			ERROR_ASSERT_PRINTF(this->memory_order_buffer_read[pos].wait_mem_deps_number == 0,
                    "Number of memory dependencies should be zero.\n %s\n",
                    this->memory_order_buffer_read[i].rob_ptr->content_to_string().c_str());

			#if EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("\nSolving %s\n\n", this->memory_order_buffer_read[pos].rob_ptr->content_to_string().c_str())
				}
			#endif

			this->memory_order_buffer_read[pos].rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->memory_order_buffer_read[pos].rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->memory_order_buffer_read[pos].processed=true;
			this->memory_read_executed--;
			this->solve_registers_dependency(this->memory_order_buffer_read[pos].rob_ptr);

			if (DISAMBIGUATION_ENABLED){
				this->disambiguator->solve_memory_dependences(&this->memory_order_buffer_read[pos]);
			}

			if(this->memory_order_buffer_read[pos].waiting_DRAM){
				ERROR_ASSERT_PRINTF(this->request_DRAM > 0,"ERRO, Contador negativo Waiting DRAM\n");
				#if EXECUTE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("\nReducing DRAM COUNTER\n\n");
					}
				#endif

				this->request_DRAM--;
			}
		}

		pos++;
		if (pos >= MOB_READ) pos = 0;
	}
}



// ============================================================================
void processor_t::execute()
{
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Execute Stage ==========\n")
		}
	#endif

	if (this->get_HAS_VIMA()) this->clean_mob_vima();
	if (this->get_HAS_HIVE()) this->clean_mob_hive();

	this->clean_mob_read();
	this->clean_mob_vectorial();
	uint32_t uop_total_executed = 0;
	for (int32_t type = 0; type < 2; ++type) {
		container_ptr_reorder_buffer_line_t * unified_fu = (type == 0) ? &this->unified_functional_units
																	   : &this->unified_vectorial_functional_units;
		

		for (uint32_t i = 0; i < unified_fu->size(); i++){
			reorder_buffer_line_t *rob_line = (*unified_fu)[i];
			
			if (uop_total_executed == EXECUTE_WIDTH){
				break;
			}
			if (rob_line == NULL){
				break;
			}

			if (rob_line->uop.readyAt <= orcs_engine.get_global_cycle()){
				ERROR_ASSERT_PRINTF(rob_line->stage == PROCESSOR_STAGE_EXECUTION, "ROB not on execution state")
				ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_WAIT, "FU with Package not in ready state")
				bool is_vectorial_part = (rob_line->uop.is_vectorial_part >= 0);
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
						unified_fu->erase(unified_fu->begin() + i);
						unified_fu->shrink_to_fit();
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
						unified_fu->erase(unified_fu->begin() + i);
						unified_fu->shrink_to_fit();
						i--;

						#if DEBUG
							ORCS_PRINTF ("Processor execute(): HIVE instruction %lu executed!\n", rob_line->uop.uop_number)
						#endif
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
						unified_fu->erase(unified_fu->begin() + i);
						unified_fu->shrink_to_fit();
						i--;

						#if VIMA_DEBUGG
							ORCS_PRINTF ("%lu Processor execute(): VIMA instruction %lu executed!\n", orcs_engine.get_global_cycle(), rob_line->uop.uop_number)
						#endif
					}
					break;
					case INSTRUCTION_OPERATION_MEM_LOAD:
					{

						if (rob_line->uop.is_validation) {
							printf("Erro: validação no estágio execute\n");
							exit(1);
							//rob_line->stage = PROCESSOR_STAGE_COMMIT;
							//rob_line->uop.updatePackageReady(EXECUTE_LATENCY + COMMIT_LATENCY);
							//this->solve_registers_dependency(rob_line);
							//uop_total_executed++;
							///// Remove from the Functional Units
							//unified_fu->erase(unified_fu->begin() + i);
							//unified_fu->shrink_to_fit();
							//i--;
						} else {
							ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
							
							if (is_vectorial_part == false)	this->memory_read_executed++;
							else this->memory_vectorial_executed++;
							
							rob_line->mob_ptr->uop_executed = true;
							rob_line->uop.updatePackageWait(EXECUTE_LATENCY);
							uop_total_executed++;
							/// Remove from the Functional Units
							unified_fu->erase(unified_fu->begin() + i);
							unified_fu->shrink_to_fit();
							i--;

						}
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
						unified_fu->erase(unified_fu->begin() + i);
						unified_fu->shrink_to_fit();
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
				#if EXECUTE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("Executed %s\n", rob_line->content_to_string().c_str())
					}
				#endif

				#if PROCESSOR_DEBUG
					ORCS_PRINTF ("%lu processor %lu execute(): uop %lu %s, Type: %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
							orcs_engine.get_global_cycle(),
    	                    this->processor_id,
    	                    rob_line->uop.uop_number,
    	                    get_enum_instruction_operation_char(rob_line->uop.uop_operation),
							(rob_line->uop.is_validation) ? "Val" : (rob_line->uop.is_vectorial_part >= 0) ? "VP" : "Ins",
    	                    rob_line->uop.readyAt,
    	                    this->fetchBuffer.get_size(),
    	                    this->decodeBuffer.get_size(),
    	                    reorderBuffer.robUsed);
				#endif



			} //end if ready package
		}	 //end for
	} // end type
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("Memory Operations Read Executed %u\n",this->memory_read_executed)
			ORCS_PRINTF("Memory Operations Write Executed %u\n",this->memory_write_executed)
			ORCS_PRINTF("Memory Operations Vectorial Executed %u\n",this->memory_vectorial_executed)			
			ORCS_PRINTF("Requests to DRAM on the Fly %d \n",this->request_DRAM)
		}
	#endif
	// =========================================================================
	// Verificar se foi executado alguma operação de leitura,
	//  e executar a mais antiga no MOB
	// =========================================================================
	for (size_t i = 0; i < PARALLEL_LOADS; i++){
		/*if(this->memory_read_executed!=0){
			this->mob_read();
		} else if (this->memory_vectorial_executed != 0) {
			this->mob_vectorial();
		}*/
		if ((this->memory_read_executed!=0) || (this->memory_vectorial_executed != 0)){
			this->mob_vet_and_read();
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
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	#endif
} //end method

// ============================================================================
memory_order_buffer_line_t* processor_t::get_next_op_load() {
	uint32_t pos = this->memory_order_buffer_read_start;
	for(uint32_t i = 0 ; i < this->memory_order_buffer_read_used; i++)
    {
		if (this->memory_order_buffer_read[pos].uop_executed &&
			this->memory_order_buffer_read[pos].status == PACKAGE_STATE_WAIT &&
			this->memory_order_buffer_read[pos].sent==false &&
        	this->memory_order_buffer_read[pos].wait_mem_deps_number == 0 &&
			this->memory_order_buffer_read[pos].readyToGo <= orcs_engine.get_global_cycle())
        {
				assert(this->memory_order_buffer_read[pos].rob_ptr != NULL);
				return &this->memory_order_buffer_read[pos];
		}

		pos++;
		if (pos >= MOB_READ) pos = 0;
	}
	return NULL;
}

// ============================================================================
uint32_t processor_t::mob_read(){
	if(this->oldest_read_to_send == NULL) this->oldest_read_to_send = this->get_next_op_load();

	if (this->oldest_read_to_send != NULL && !this->oldest_read_to_send->sent){
		if (!orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)) return OK;
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
		request->type = DATA;
		request->uop_number = oldest_read_to_send->uop_number;
		request->processor_id = this->processor_id;
		request->op_count[request->memory_operation]++;
		request->clients.shrink_to_fit();

		#if MEMORY_DEBUG
			ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.R\n", orcs_engine.get_global_cycle(), request->opcode_number,
                    request->memory_address,
                    get_enum_memory_operation_char(request->memory_operation));
		#endif

		if (orcs_engine.cacheManager->searchData(request)){
			this->oldest_read_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
			this->oldest_read_to_send->sent=true;
			this->oldest_read_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit

		} else {
			this->add_times_reach_parallel_requests_read();
			delete request;
		}

		this->oldest_read_to_send = NULL;
	} //end if request null
	return OK;
} //end method

void processor_t::print_mob_hive(){
	ORCS_PRINTF ("Cycle: %lu\n", orcs_engine.get_global_cycle())
	for(uint32_t j = 0; j < this->memory_order_buffer_hive_used; j++){
		ORCS_PRINTF ("Processor print_mob_hive(): %s %s %lu %u %lu.\n",
                get_enum_package_state_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].status),
                get_enum_memory_operation_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].memory_operation),
                this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].uop_number,
                this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].wait_mem_deps_number,
                this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].readyToGo);
	}
}

void processor_t::print_mob_vima(){
	ORCS_PRINTF ("Cycle: %lu\n", orcs_engine.get_global_cycle())
	for(uint32_t j = 0; j < this->memory_order_buffer_vima_used; j++){
		ORCS_PRINTF ("Processor print_mob_vima(): %s %s %lu %u %lu.\n",
                get_enum_package_state_char (this->memory_order_buffer_vima[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].status),
                get_enum_memory_operation_char (this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].memory_operation),
                this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].uop_number,
                this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].wait_mem_deps_number,
                this->memory_order_buffer_hive[(this->memory_order_buffer_vima_start+j) % MOB_VIMA].readyToGo);
	}
}

void processor_t::print_mob_vectorial(){
	ORCS_PRINTF ("Cycle: %lu\n", orcs_engine.get_global_cycle())
	for(uint32_t j = 0; j < this->memory_order_buffer_vectorial_used; j++){
		ORCS_PRINTF ("Processor print_mob_vectorial(): %s %s %lu %u %lu.\n",
                get_enum_package_state_char (this->memory_order_buffer_vectorial[(this->memory_order_buffer_vectorial_start+j) % MOB_VECTORIAL].status),
                get_enum_memory_operation_char (this->memory_order_buffer_vectorial[(this->memory_order_buffer_vectorial_start+j) % MOB_VECTORIAL].memory_operation),
                this->memory_order_buffer_vectorial[(this->memory_order_buffer_vectorial_start+j) % MOB_VECTORIAL].uop_number,
                this->memory_order_buffer_vectorial[(this->memory_order_buffer_vectorial_start+j) % MOB_VECTORIAL].wait_mem_deps_number,
                this->memory_order_buffer_vectorial[(this->memory_order_buffer_vectorial_start+j) % MOB_VECTORIAL].readyToGo);
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
				#if DEBUG
					//ORCS_PRINTF ("Processor get_next_op_hive(): fetching next HIVE instruction from MOB.\n")
					for(uint32_t j = 0; j < this->memory_order_buffer_hive_used; j++){
						//ORCS_PRINTF ("Processor get_next_op_hive(): %s %s %lu %u %lu.\n",
                        //             get_enum_package_state_char(this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].status),
                        //             get_enum_memory_operation_char(this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].memory_operation),
                        //             this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].uop_number,
                        //             this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].wait_mem_deps_number,
                        //             this->memory_order_buffer_hive[(this->memory_order_buffer_hive_start+j) % MOB_HIVE].readyToGo);
					}
				#endif
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
memory_order_buffer_line_t* processor_t::get_next_op_vectorial() {
	uint32_t pos = this->memory_order_buffer_vectorial_start;
	for(uint32_t i = 0 ; i < this->memory_order_buffer_vectorial_used; i++)
    {

		if (this->memory_order_buffer_vectorial[pos].uop_executed &&
			this->memory_order_buffer_vectorial[pos].status == PACKAGE_STATE_WAIT &&
			this->memory_order_buffer_vectorial[pos].sent==false &&
        	this->memory_order_buffer_vectorial[pos].wait_mem_deps_number == 0 &&
			this->memory_order_buffer_vectorial[pos].readyToGo <= orcs_engine.get_global_cycle())
        {
				return &this->memory_order_buffer_vectorial[pos];
		}



		pos++;
		if (pos >= MOB_VECTORIAL) pos = 0;
	}
	return NULL;
}

// ============================================================================
uint32_t processor_t::mob_hive(){
	if(this->oldest_hive_to_send==NULL)	this->oldest_hive_to_send = this->get_next_op_hive();

	if (this->oldest_hive_to_send != NULL && !this->oldest_hive_to_send->sent){
		if (!orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)) return OK;

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
		request->uop_number = oldest_hive_to_send->uop_number;
		request->processor_id = this->processor_id;
		request->op_count[request->memory_operation]++;
		request->clients.shrink_to_fit();

		#if MEMORY_DEBUG
			ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.H\n", orcs_engine.get_global_cycle(), request->opcode_number, request->memory_address , get_enum_memory_operation_char (request->memory_operation))
		#endif

		if (orcs_engine.cacheManager->searchData(request)){
			this->oldest_hive_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
			this->oldest_hive_to_send->sent=true;
			this->oldest_hive_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
		} else delete request;
		this->oldest_hive_to_send = NULL;
	}
	return OK;
}

// ============================================================================
uint32_t processor_t::mob_vima(){
	if (this->oldest_vima_to_send == NULL) this->oldest_vima_to_send = this->get_next_op_vima();

	if (this->oldest_vima_to_send != NULL && !this->oldest_vima_to_send->sent){
		if (!orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)) return OK;

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
		request->uop_number = oldest_vima_to_send->uop_number;
		request->processor_id = this->processor_id;
		request->op_count[request->memory_operation]++;
		request->clients.shrink_to_fit();

		#if MEMORY_DEBUG
			ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.V\n", orcs_engine.get_global_cycle(), request->opcode_number, request->memory_address , get_enum_memory_operation_char (request->memory_operation))
		#endif

		if (orcs_engine.cacheManager->searchData(request)){
			this->oldest_vima_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
			this->oldest_vima_to_send->sent=true;
			this->oldest_vima_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
		} else delete request;
		this->oldest_vima_to_send = NULL;
	}
	return OK;
}
// ============================================================================
uint32_t processor_t::mob_vectorial(){
	printf("Mob vectorial\n");
	if(this->oldest_vectorial_to_send == NULL) this->oldest_vectorial_to_send = this->get_next_op_vectorial();
	if (this->oldest_vectorial_to_send != NULL && !this->oldest_vectorial_to_send->sent){
		printf("Request from uop: %lu\n", this->oldest_vectorial_to_send->uop_number);
		
		if (!orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)) return OK;
		memory_package_t* request = new memory_package_t();

		request->clients.push_back (oldest_vectorial_to_send);
		request->opcode_address = oldest_vectorial_to_send->opcode_address;
		request->memory_address = oldest_vectorial_to_send->memory_address;
		request->memory_size = oldest_vectorial_to_send->memory_size;
		request->memory_operation = oldest_vectorial_to_send->memory_operation;
		request->status = PACKAGE_STATE_UNTREATED;
		request->is_hive = false;
		request->is_vima = false;
		request->hive_read1 = oldest_vectorial_to_send->hive_read1;
		request->hive_read2 = oldest_vectorial_to_send->hive_read2;
		request->hive_write = oldest_vectorial_to_send->hive_write;
		request->readyAt = orcs_engine.get_global_cycle();
		request->born_cycle = orcs_engine.get_global_cycle();
		request->sent_to_ram = false;
		request->type = DATA;
		request->uop_number = oldest_vectorial_to_send->uop_number;
		request->processor_id = this->processor_id;
		request->op_count[request->memory_operation]++;
		request->clients.shrink_to_fit();

		#if MEMORY_DEBUG
			ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.R\n", orcs_engine.get_global_cycle(), request->opcode_number,
                    request->memory_address,
                    get_enum_memory_operation_char(request->memory_operation));
		#endif

		if (orcs_engine.cacheManager->searchData(request)){
			this->oldest_vectorial_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
			this->oldest_vectorial_to_send->sent=true;
			this->oldest_vectorial_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit

		} else {
			this->add_times_reach_parallel_requests_read();
			delete request;
		}

		this->oldest_vectorial_to_send = NULL;
	} //end if request null
	return OK;
} //end method

// ============================================================================
uint32_t processor_t::mob_vet_and_read(){
	if(this->oldest_read_to_send == NULL) this->oldest_read_to_send = this->get_next_op_load();
	if(this->oldest_vectorial_to_send == NULL) this->oldest_vectorial_to_send = this->get_next_op_vectorial();

	memory_order_buffer_line_t *to_send = this->oldest_read_to_send;

	if ((this->oldest_vectorial_to_send != NULL) &&
	    ((this->oldest_read_to_send == NULL) || 
	    this->oldest_read_to_send->sent    ||
		this->oldest_read_to_send->uop_number > this->oldest_vectorial_to_send->uop_number)){
		to_send = this->oldest_vectorial_to_send;
	} 
	

	if (to_send != NULL && !to_send->sent){
		assert(to_send->rob_ptr != NULL);
		if (!orcs_engine.cacheManager->available (this->processor_id, MEMORY_OPERATION_READ)) return OK;
		memory_package_t* request = new memory_package_t();

		request->clients.push_back (to_send);
		request->opcode_address = to_send->opcode_address;
		request->memory_address = to_send->memory_address;
		request->memory_size = to_send->memory_size;
		request->memory_operation = to_send->memory_operation;
		request->status = PACKAGE_STATE_UNTREATED;
		request->is_hive = false;
		request->is_vima = false;
		request->hive_read1 = to_send->hive_read1;
		request->hive_read2 = to_send->hive_read2;
		request->hive_write = to_send->hive_write;
		request->readyAt = orcs_engine.get_global_cycle();
		request->born_cycle = orcs_engine.get_global_cycle();
		request->sent_to_ram = false;
		request->type = DATA;
		request->uop_number = to_send->uop_number;
		request->processor_id = this->processor_id;
		request->op_count[request->memory_operation]++;
		request->clients.shrink_to_fit();

		#if MEMORY_DEBUG
			ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.R\n", orcs_engine.get_global_cycle(), request->opcode_number,
                    request->memory_address,
                    get_enum_memory_operation_char(request->memory_operation));
		#endif

		if (orcs_engine.cacheManager->searchData(request)){
			to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
			to_send->sent=true;
			to_send->rob_ptr->sent=true;							///Setting flag which marks sent request. set to remove entry on mob at commit

		} else {
			this->add_times_reach_parallel_requests_read();
			delete request;
		}

		if (to_send == this->oldest_read_to_send) {
			this->oldest_read_to_send = NULL;
		} else {
			this->oldest_vectorial_to_send = NULL;
		}

	} //end if request null

	return OK;
} //end method
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
	if (this->oldest_write_to_send == NULL)
        this->oldest_write_to_send = this->get_next_op_store();


	if (this->oldest_write_to_send != NULL && !this->oldest_write_to_send->sent) {
		
		if (!orcs_engine.cacheManager->available(this->processor_id, MEMORY_OPERATION_WRITE))
            return OK;
		

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
		request->uop_number = oldest_write_to_send->uop_number;
		request->processor_id = this->processor_id;
		request->op_count[request->memory_operation]++;

		#if MEMORY_DEBUG
			ORCS_PRINTF ("[PROC] %lu {%lu} %lu %s sent to memory.W\n", orcs_engine.get_global_cycle(), request->opcode_number,
                    request->memory_address,
                    get_enum_memory_operation_char (request->memory_operation));
		#endif

		if (orcs_engine.cacheManager->searchData(request)) {

			this->oldest_write_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
			this->oldest_write_to_send->sent=true;
			this->oldest_write_to_send->rob_ptr->sent=true;	///Setting flag which marks sent request. set to remove entry on mob at commit
			this->oldest_write_to_send->rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
			this->oldest_write_to_send->rob_ptr->uop.updatePackageReady(COMMIT_LATENCY);
			this->oldest_write_to_send->processed=true;
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

		this->oldest_write_to_send = NULL;
		// =============================================================
	} //end if request null
	return OK;
}

// ============================================================================
void processor_t::commit(){

	#if COMMIT_DEBUG
	if (orcs_engine.get_global_cycle() >= 318957) {
		reps++;
		DEBUG_STARTED = true;
		if (reps >= 1000) {
			DEBUG_STARTED = false;
			exit(1);
		}
	} else if (orcs_engine.get_global_cycle() % 100000 == 0) {
		printf(">> %lu\n", orcs_engine.get_global_cycle());
	}
	if (DEBUG_STARTED)
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
		{
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Commit Stage ==========\n")
			ORCS_PRINTF("Cycle %lu\n", orcs_engine.get_global_cycle())
			if (this->reorderBuffer.robUsed > 0)
				std::cout << "ROB Head " << this->reorderBuffer.reorderBuffer[reorderBuffer.robStart].content_to_string() << std::endl;
			if (this->vectorialReorderBuffer.robUsed > 0)
				ORCS_PRINTF("ROB Vectorial Head %s\n",this->vectorialReorderBuffer.reorderBuffer[vectorialReorderBuffer.robStart].content_to_string().c_str())
			
			/*if (orcs_engine.get_global_cycle() > 10000){
			//for (uint32_t i = this->reorderBuffer.robStart;  i != this->reorderBuffer.robEnd; (i == this->ROB_SIZE - 1) ? i = 0 : ++i) {
			//	ORCS_PRINTF("ROB[%u] %s\n", i, this->reorderBuffer.reorderBuffer[i].content_to_string().c_str())
			//}
			printf("Fim de jogo ROB start: %u ROB end: %u Elements: %u:p\n", 
											this->reorderBuffer.robStart,
											this->reorderBuffer.robEnd,
											this->reorderBuffer.robUsed);
			exit(1);
			}*/
			ORCS_PRINTF("==================================\n")
		}
	#endif
	

	int32_t pos_buffer;
	/// Commit the packages
	for (int32_t type = 0; type < 2; ++type) {
		ROB_t *rob = (type == 0) ? &this->reorderBuffer
								 : &this->vectorialReorderBuffer;

		if (rob->robUsed == 0) {
			continue;
		}


		for (uint32_t i = 0; i < COMMIT_WIDTH; i++){
			pos_buffer = rob->robStart;
			reorder_buffer_line_t *rob_line = &rob->reorderBuffer[pos_buffer];
			
			if (DEBUG_STARTED)
				printf("%lu: Trying commit %lu %s (op_n: %lu) -- stage: %s -> %s Executed: %s (sent:%s) Ready at: %lu [%u/%u]\n",
						orcs_engine.get_global_cycle(),
						rob_line->uop.uop_number,
						(rob_line->uop.is_validation) ? "Val" : (rob_line->uop.is_vectorial_part >= 0) ? "Vec" : "Ins",
						rob_line->uop.opcode_number,
						get_enum_processor_stage_char(rob_line->stage),
						get_enum_package_state_char(rob_line->uop.status),
						(rob_line->mob_ptr) ? ((rob_line->mob_ptr->uop_executed) ? "true" : "false") : "--",
						(rob_line->mob_ptr) ? ((rob_line->mob_ptr->sent) ? "true" : "false") : "--",
						rob_line->uop.readyAt,
						rob->robUsed, rob->SIZE);
			if (DEBUG_STARTED)			
				if (rob_line->uop.is_validation) {
					auto bits = &vectorizer->vr_control_bits[rob_line->uop.VR_id].positions[rob_line->uop.will_validate_offset];
					printf(" -> Val VR: %d Part %d [V: %d; R: %d; U: %d; F: %d] Sent: %s Exec: %s Free: %s\n",
					rob_line->uop.VR_id,
					rob_line->uop.will_validate_offset,
					bits->V,
					bits->R,
					bits->U,
					bits->F,
					bits->sent ? "true" : "false",
					bits->executed ? "true" : "false",
					bits->free ? "true" : "false");
				}
			if (DEBUG_STARTED)			
				if (rob_line->uop.is_vectorial_part >= 0) {
					auto bits = &vectorizer->vr_control_bits[rob_line->uop.VR_id].positions[rob_line->uop.is_vectorial_part];
					printf(" -> Vec VR: %d Part %d [V: %d; R: %d; U: %d; F: %d] Sent: %s Exec: %s Free: %s\n",
					rob_line->uop.VR_id,
					rob_line->uop.is_vectorial_part,
					bits->V,
					bits->R,
					bits->U,
					bits->F,
					bits->sent ? "true" : "false",
					bits->executed ? "true" : "false",
					bits->free ? "true" : "false");
				}




			if ((rob->robUsed != 0) &&
				rob_line->stage == PROCESSOR_STAGE_COMMIT &&
				rob_line->uop.status == PACKAGE_STATE_READY &&
				rob_line->uop.readyAt <= orcs_engine.get_global_cycle() &&
				((rob_line->uop.is_validation == false) ||
				 (vectorizer->vr_control_bits[rob_line->uop.VR_id].positions[rob_line->uop.will_validate_offset].R == 0 && 
				 vectorizer->vr_control_bits[rob_line->uop.VR_id].positions[rob_line->uop.will_validate_offset].executed == true)))
			{
				bool is_vectorial_part = (rob_line->uop.is_vectorial_part >= 0);

				// Vectorizer
				vectorizer->new_commit(&rob_line->uop);
				
				if (rob_line->uop.is_validation) {
					this->solve_registers_dependency(rob_line);
				}

				this->commit_uop_counter++;
				switch (rob_line->uop.uop_operation){
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
						#if DEBUG
							ORCS_PRINTF("Processor commit(): instruction HIVE %lu, %s committed, readyAt %lu.\n",
    	                            rob_line->uop.uop_number,
    	                            get_enum_instruction_operation_char(rob_line->uop.uop_operation),
    	                            rob_line->uop.readyAt);
						#endif
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
						#if VIMA_DEBUGG
							ORCS_PRINTF("%lu Processor commit(): instruction VIMA %lu, %s committed, readyAt %lu.\n",
    	                            orcs_engine.get_global_cycle(),
    	                            rob_line->uop.uop_number,
    	                            get_enum_instruction_operation_char(rob_line->uop.uop_operation),
    	                            rob_line->uop.readyAt);
						#endif
						break;
					// MEMORY OPERATIONS - READ
					case INSTRUCTION_OPERATION_MEM_LOAD:{

						if(rob_line->uop.is_validation) {
							#if MEMORY_DEBUG
							ORCS_PRINTF("[ROBL] %lu {%lu} Validation load uop: %s removed from reorder order buffer.\n",
									orcs_engine.get_global_cycle(),
									rob_line->uop.opcode_number,
    	                            get_enum_instruction_operation_char (rob_line->uop.opcode_operation));
							#endif
							break;
						}
						#if MEMORY_DEBUG
							ORCS_PRINTF("[ROBL] %lu {%lu} %lu %s removed from reorder order buffer.\n",
    	                            orcs_engine.get_global_cycle(),
									rob_line->uop.opcode_number,
    	                            rob_line->mob_ptr->memory_address,
    	                            get_enum_memory_operation_char (rob_line->mob_ptr->memory_operation));
						#endif
						if(rob_line->mob_ptr->waiting_DRAM){
							this->core_ram_request_wait_cycles+=(rob_line->mob_ptr->readyAt - rob_line->mob_ptr->cycle_send_request);
							this->add_core_ram_requests();
						}

						this->mem_req_wait_cycles += (rob_line->mob_ptr->readyAt - rob_line->mob_ptr->readyToGo);
    	                // #if PROCESSOR_DEBUG
    	                //    ORCS_PRINTF ("%lu processor %lu commit(): uop %lu %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
    	                //                 this->processor_id,
    	                //                 orcs_engine.get_global_cycle(),
    	                //                 rob_line->uop.uop_number,
    	                //                 get_enum_instruction_operation_char(rob_line->uop.uop_operation),
    	                //                 rob_line->uop.readyAt,
    	                //                 this->fetchBuffer.get_size(),
    	                //                 this->decodeBuffer.get_size(),
    	                //                 reorderBuffer.robUsed);
    	                // #endif
						this->add_stat_inst_load_completed();
						break;
					}
					// MEMORY OPERATIONS - WRITE
					case INSTRUCTION_OPERATION_MEM_STORE:
						#if MEMORY_DEBUG
							ORCS_PRINTF("[ROBL] %lu {%lu} %lu %s removed from reorder order buffer.\n",
    	                            orcs_engine.get_global_cycle(),
									rob_line->uop.opcode_number,
    	                            rob_line->mob_ptr->memory_address,
    	                            get_enum_memory_operation_char(rob_line->mob_ptr->memory_operation));
						#endif

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

				ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == rob->robStart, "Commiting different from the position start\n");
				#if PROCESSOR_DEBUG
					ORCS_PRINTF("%lu processor %lu commit(): uop %lu %s, Type: %s, readyAt %lu, fetchBuffer: %u, decodeBuffer: %u, robUsed: %u.\n",
							orcs_engine.get_global_cycle(),
    	                    this->processor_id,
    	                    rob_line->uop.uop_number,
    	                    get_enum_instruction_operation_char(rob_line->uop.uop_operation),
							(rob_line->uop.is_validation) ? "Val" : (rob_line->uop.is_vectorial_part >= 0) ? "VP" : "Ins",
    	                    rob_line->uop.readyAt,
    	                    this->fetchBuffer.get_size(),
    	                    this->decodeBuffer.get_size(),
    	                    rob->robUsed);
				#endif


				#if COMMIT_DEBUG
				if (DEBUG_STARTED)
					if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
					{
						ORCS_PRINTF("======================================\n")
						ORCS_PRINTF("RM ROB Entry \n%s\n", rob->reorderBuffer[rob->robStart].content_to_string().c_str())
					}
				#endif

				if (rob->reorderBuffer[rob->robStart].sent==true){
					if(rob->reorderBuffer[rob->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_LOAD) {
						if (is_vectorial_part == false) {
							this->remove_front_mob_read();
						} else {
							this->remove_front_mob_vectorial();
						}
					} else if (rob->reorderBuffer[rob->robStart].uop.is_hive) {
						this->remove_front_mob_hive();
					} else if (rob->reorderBuffer[rob->robStart].uop.is_vima) {
						this->remove_front_mob_vima();
					}
				}

				this->wait_time = orcs_engine.get_global_cycle() - rob_line->uop.born_cycle;
				this->total_latency[rob_line->uop.opcode_operation] += this->wait_time;

				if (this->wait_time > this->max_wait_operations[rob_line->uop.opcode_operation])
    	            this->max_wait_operations[rob_line->uop.opcode_operation] = this->wait_time;

				if (this->wait_time < this->min_wait_operations[rob_line->uop.opcode_operation])
    	            this->min_wait_operations[rob_line->uop.opcode_operation] = this->wait_time;

				this->removeFrontROB(rob);
			}
			/// Could not commit the older, then stop looking for ready uops
			else
			{
				i = 0;
				#if DEBUG
					//ORCS_PRINTF ("=======Processor %lu, Cycle %lu=========\n", this->processor_id+1, orcs_engine.get_global_cycle())
					for (uint32_t i = 0; i < reorderBuffer.robUsed + vectorialReorderBuffer.robUsed; i++){
						ORCS_PRINTF("%u COMMIT: %s %s %s %lu %lu\n",
    	                        i, get_enum_processor_stage_char(this->reorderBuffer[(i+robStart) % ROB_SIZE].stage),
    	                        get_enum_instruction_operation_char(this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.uop_operation),
    	                        get_enum_package_state_char(this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.status),
    	                        this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.uop_number,
    	                        this->reorderBuffer[(i+robStart) % ROB_SIZE].uop.readyAt);
					}
				#endif
				break;
			}
		} // end for

	} // end type

	#if COMMIT_DEBUG
	if (DEBUG_STARTED)
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	#endif

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

	if (orcs_engine.output_file_name != NULL) {
		output = fopen(orcs_engine.output_file_name,"a+");
		close=true;
	}

    for (auto it : instr_cnt) {
        std::cout << it.first << " " << it.second << '\n';
    }

	if (output != NULL){
		printf("Vetorial: %lu -- Escalar: %lu\n", vet, esc);
		utils_t::largestSeparator(output);
		fprintf(output, "Total_Cycle:  %lu\n", this->get_ended_cycle());
		utils_t::largeSeparator(output);
		fprintf(output, "Stage_Opcode_and_Uop_Counters\n");
		utils_t::largeSeparator(output);
		fprintf(output, "Stage_Fetch:  %lu\n", this->fetchCounter);
		fprintf(output, "Stage_Decode: %lu\n", this->decodeCounter);
		fprintf(output, "Stage_Rename: %lu\n", this->renameCounter);
		fprintf(output, "Stage_Commit: %lu\n", this->commit_uop_counter);
		utils_t::largestSeparator(output);
		fprintf(output, "Instruction_Per_Cycle:            %1.6lf\n", (float)this->fetchCounter/this->get_ended_cycle());
		// accessing LLC cache level
		uint64_t plevels = orcs_engine.cacheManager->get_POINTER_LEVELS();
		int32_t *cache_indexes = new int32_t[plevels]();
		orcs_engine.cacheManager->generateIndexArray(this->processor_id, cache_indexes);

		//assuming you want LLC, plevels-1 should be LLC
		fprintf(output, "MPKI:                             %lf\n", (float)orcs_engine.cacheManager->data_cache[plevels-1][cache_indexes[plevels-1]].get_cache_miss()/((float)this->fetchCounter/1000));
		fprintf(output, "Average_wait_cycles_wait_mem_req: %lf\n", (float)this->mem_req_wait_cycles/this->get_stat_inst_load_completed());
		fprintf(output, "Core_Request_RAM_AVG_Cycle:       %lf\n", (float)this->core_ram_request_wait_cycles/this->get_core_ram_requests());
		fprintf(output, "Total_Load_Requests:              %lu\n", this->get_stat_inst_load_completed());
		fprintf(output, "Total_Store_Requests:             %lu\n", this->get_stat_inst_store_completed());
		fprintf(output, "Total_HIVE_Instructions:          %lu\n", this->get_stat_inst_hive_completed());
		fprintf(output, "Total_VIMA_Instructions:          %lu\n", this->get_stat_inst_vima_completed());
		utils_t::largestSeparator(output);
		fprintf(output, "Stalls Fetch:          %lu\n", this->get_stall_full_FetchBuffer());
		fprintf(output, "Stalls Decode:          %lu\n", this->get_stall_full_DecodeBuffer());
		fprintf(output, "Stalls Rename:          %lu\n", this->get_stall_full_ROB());
		fprintf(output, "Stalls Rename Vectorial:          %lu\n", this->get_stall_full_ROB_VET());


		utils_t::largestSeparator(output);
		for (int i = 0; i < INSTRUCTION_OPERATION_LAST; i++){
			if (this->total_operations[i] > 0){
				fprintf(output, "Total_%s_Instructions:         %lu\n", get_enum_instruction_operation_char ((instruction_operation_t) i), this->total_operations[i]);
				fprintf(output, "Total_%s_Instructions_Latency: %lu\n", get_enum_instruction_operation_char ((instruction_operation_t) i), this->total_latency[i]);
				fprintf(output, "Avg._%s_Instructions_Latency:  %lu\n", get_enum_instruction_operation_char ((instruction_operation_t) i), this->total_latency[i]/this->total_operations[i]);

				if (this->max_wait_operations[i] > 0)
                    fprintf(output, "Max_%s_Instructions_Latency:   %lu\n", get_enum_instruction_operation_char ((instruction_operation_t) i), this->max_wait_operations[i]);

				if (this->min_wait_operations[i] < UINT64_MAX)
                    fprintf(output, "Min_%s_Instructions_Latency:   %lu\n", get_enum_instruction_operation_char ((instruction_operation_t) i), this->min_wait_operations[i]);
			}
		}

		utils_t::largestSeparator(output);
		delete[] cache_indexes;
	}

	if (close) fclose(output);
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
		fprintf(output, "Vectorial ROB ->%u\n", ROB_VECTORIAL_SIZE);
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
	#if DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("============================PROCESSOR %lu===============================\n",this->processor_id)
			ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
	if (get_HAS_VIMA()) orcs_engine.vima_controller->clock();
	if (get_HAS_HIVE()) orcs_engine.hive_controller->clock();
	orcs_engine.cacheManager->clock();
	/////////////////////////////////////////////////
	//// Verifica se existe coisas no ROB
	//// CommitStage
	//// ExecuteStage
	//// DispatchStage
	/////////////////////////////////////////////////
		if ((reorderBuffer.robUsed != 0) || (vectorialReorderBuffer.robUsed != 0))
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
	#if DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("===================================================================\n")
			// sleep(1);
		}
	#endif
}
// ========================================================================================================================================================================================
