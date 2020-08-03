// ============================================================================
// ============================================================================
class processor_t {
    private:    
	//=============
	//Fetch Related
	//=============
    uint64_t stall_full_FetchBuffer;
    uint64_t stall_wrong_branch;
	//=============
	//Statistics Decode
	//=============
    uint64_t stall_full_DecodeBuffer;
	//=============
	//Statistics Rename
	//=============
	uint64_t registerWrite;
	uint64_t stall_full_MOB_Read;
	uint64_t stall_full_MOB_Write;
	uint64_t stall_full_ROB;
	//=============
	//Statistics Dispatch
	//=============
	uint64_t stall_empty_RS;
	//=============
	//Statistics Execute
	//=============
	uint64_t stat_disambiguation_read_false_positive;
	uint64_t stat_disambiguation_write_false_positive;
	uint64_t stat_address_to_address;
	uint64_t times_reach_parallel_requests_read;
	uint64_t times_reach_parallel_requests_write;
	float_t instruction_per_cycle;
	uint64_t ended_cycle;
	uint64_t mem_req_wait_cycles;
	uint64_t core_ram_request_wait_cycles;
	uint64_t core_ram_requests;
	//=============
	//Statistics Commit
	//=============
	uint64_t stat_inst_int_alu_completed;
	uint64_t stat_inst_mul_alu_completed;
	uint64_t stat_inst_div_alu_completed;
	uint64_t stat_inst_int_fp_completed;
	uint64_t stat_inst_mul_fp_completed;
	uint64_t stat_inst_div_fp_completed;
	uint64_t stat_inst_hive_completed;
	uint64_t stat_inst_vima_completed;
	uint64_t stat_inst_nop_completed;
	uint64_t stat_inst_load_completed;
	uint64_t stat_inst_store_completed;
	uint64_t stat_inst_branch_completed;
	uint64_t stat_inst_other_completed;

	uint32_t HAS_HIVE;
	uint32_t HAS_VIMA;

	uint32_t FETCH_WIDTH;
	uint32_t DECODE_WIDTH;
	uint32_t RENAME_WIDTH;
	uint32_t DISPATCH_WIDTH;
	uint32_t EXECUTE_WIDTH;
	uint32_t COMMIT_WIDTH;

	uint32_t FETCH_LATENCY;
	uint32_t DECODE_LATENCY;
	uint32_t RENAME_LATENCY;
	uint32_t DISPATCH_LATENCY;
	uint32_t EXECUTE_LATENCY;
	uint32_t COMMIT_LATENCY;

	uint32_t LATENCY_INTEGER_ALU;
	uint32_t WAIT_NEXT_INT_ALU;
	uint32_t INTEGER_ALU;
	// INTEGER MUL
	uint32_t LATENCY_INTEGER_MUL;
	uint32_t WAIT_NEXT_INT_MUL;
	uint32_t INTEGER_MUL;
	// INTEGER DIV
	uint32_t LATENCY_INTEGER_DIV;
	uint32_t WAIT_NEXT_INT_DIV;
	uint32_t INTEGER_DIV;

	uint32_t QTDE_INTEGER_FU;

	//FP ULAS LATENCY 
	// FLOATING POINT DIV
	uint32_t LATENCY_FP_DIV;
	uint32_t WAIT_NEXT_FP_DIV;
	uint32_t FP_DIV;
	// FLOATING POINT MUL
	uint32_t LATENCY_FP_MUL;
	uint32_t WAIT_NEXT_FP_MUL;
	uint32_t FP_MUL;
	// FLOATING POINT ALU
	uint32_t LATENCY_FP_ALU;
	uint32_t WAIT_NEXT_FP_ALU;
	uint32_t FP_ALU;

	uint32_t QTDE_FP_FU;

	uint32_t PARALLEL_LOADS;
	uint32_t PARALLEL_STORES;

	// ======================
	///UNIFIED FUS

	// PROCESSOR BUFFERS SIZE
	uint32_t FETCH_BUFFER;
	uint32_t DECODE_BUFFER;
	uint32_t RAT_SIZE;
	uint32_t ROB_SIZE;
	uint32_t UNIFIED_RS;
	//MOB
	uint32_t MOB_READ;
	uint32_t MOB_WRITE;
	uint32_t MOB_HIVE;
	uint32_t MOB_VIMA;
	// =====================

	// =====================
	// MEMORY FU
	// =====================
	// Load Units
	uint32_t LOAD_UNIT;
	uint32_t WAIT_NEXT_MEM_LOAD;
	uint32_t LATENCY_MEM_LOAD;
	// Store Units
	uint32_t STORE_UNIT;
	uint32_t WAIT_NEXT_MEM_STORE;
	uint32_t LATENCY_MEM_STORE;
	// HIVE Units
	uint32_t HIVE_UNIT;
	uint32_t WAIT_NEXT_MEM_HIVE;
	uint32_t LATENCY_MEM_HIVE;
	// VIMA Units
	uint32_t VIMA_UNIT;
	uint32_t WAIT_NEXT_MEM_VIMA;
	uint32_t LATENCY_MEM_VIMA;

	uint32_t QTDE_MEMORY_FU;

	//uint32_t KILO = 1024;
	//uint32_t MEGA = KILO*KILO;

	uint32_t LINE_SIZE;
	uint32_t CACHE_LEVELS;

	uint32_t DATA_CACHES;
	uint32_t *DATA_SIZE;
	uint32_t *DATA_ASSOCIATIVITY;
	uint32_t *DATA_LATENCY;
	uint32_t *DATA_SETS;
	uint32_t *DATA_LEVEL;
	// I$
	uint32_t INSTRUCTION_CACHES;
	uint32_t *INST_SIZE;
	uint32_t *INST_ASSOCIATIVITY;
	uint32_t *INST_LATENCY;
	uint32_t *INST_SETS;
	uint32_t *INST_LEVEL;

	uint32_t RAM_LATENCY;
	uint32_t PARALLEL_LIM_ACTIVE;
	uint32_t MAX_PARALLEL_REQUESTS_CORE;

	uint32_t PREFETCHER_ACTIVE;

	uint32_t DISAMBIGUATION_ENABLED;
	disambiguation_method_t DISAMBIGUATION_METHOD;

	uint32_t DEBUG;
	uint32_t PROCESSOR_DEBUG;
	uint32_t FETCH_DEBUG;
	uint32_t DECODE_DEBUG;
	uint32_t RENAME_DEBUG;
	uint32_t DISPATCH_DEBUG;
	uint32_t EXECUTE_DEBUG;
	uint32_t MOB_DEBUG;
	uint32_t PRINT_MOB;
	uint32_t PRINT_ROB;
	uint32_t HIVE_DEBUG;
	uint32_t VIMA_DEBUG;
	uint32_t COMMIT_DEBUG;
	uint32_t MSHR_DEBUG;
	uint32_t MULTICORE_DEBUG;

	uint32_t WAIT_CYCLE;

    public:
		
		// ====================================================================
		/// Attributes
		// ====================================================================
		uint32_t processor_id;
		//control Branches
		bool hasBranch;
		opcode_package_t previousBranch;
		//error at insert fetch buffer
		bool insertError;
		opcode_package_t opcodeError;
		// ====================================================================
		// Control
		// ====================================================================
		bool traceIsOver;
		bool snapshoted;
		uint64_t fetchCounter;
		uint64_t decodeCounter;
		uint64_t renameCounter;
		uint64_t uopCounter;
		uint64_t commit_uop_counter;
		uint32_t memory_read_executed;
		uint32_t memory_write_executed;
		uint32_t memory_hive_executed;
		uint32_t memory_vima_executed;
		
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
		~processor_t();
	    void allocate();
	    void clock();
		void statistics();
		void printConfiguration();
		void printCache(FILE *output);
		uint32_t get_cache_list(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs, uint32_t *ASSOCIATIVITY, uint32_t *LATENCY, uint32_t *SIZE, uint32_t *SETS, uint32_t *LEVEL);

		// ====================================================================
		// ROB RELATED
		void update_registers(reorder_buffer_line_t *robLine);
		void solve_registers_dependency(reorder_buffer_line_t *rob_line);
		int32_t searchPositionROB();
		void removeFrontROB();
		// ====================================================================
		// MOB READ RELATED
		int32_t search_position_mob_read();
		void remove_front_mob_read();	
		// ====================================================================
		// MOB WRITE RELATED
		int32_t search_position_mob_write();
		void remove_front_mob_write();
		// ====================================================================
		// MOB HIVE RELATED
		void print_mob_hive();
		int32_t search_position_mob_hive();
		void remove_front_mob_hive();
		// ====================================================================
		// MOB HIVE RELATED
		void print_mob_vima();
		int32_t search_position_mob_vima();
		void remove_front_mob_vima();
		// ====================================================================
		// Stage Methods
		// ====================================================================
		void fetch();
		void decode();
		void rename();
		void dispatch();
		void execute();

		uint32_t mob_read();
		void clean_mob_read();
		uint32_t mob_write();
		void clean_mob_write();
		uint32_t mob_hive();
		void clean_mob_hive();
		uint32_t mob_vima();
		void clean_mob_vima();
		
		void commit();
		// ====================================================================
		// Bool Functions @return 
		bool isBusy();
		// ====================================================================
		// Other Methods
		// ====================================================================
		// =======================
		// Buffers
		// =======================
		circular_buffer_t<uop_package_t> decodeBuffer;
		circular_buffer_t<opcode_package_t> fetchBuffer;
		
		// =======================
		// Register Alias Table - RAT
		// =======================
		reorder_buffer_line_t **register_alias_table;
		// =======================
		// Reorder Buffer
		// =======================
        reorder_buffer_line_t *reorderBuffer;
        uint32_t robStart;
        uint32_t robEnd;
        uint32_t robUsed;

		// ======================
		// Memory Order Buffer
		// ======================
		desambiguation_t *disambiguator;
		// ======================
		//READ
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_read;
        uint32_t memory_order_buffer_read_start;
        uint32_t memory_order_buffer_read_end;
        uint32_t memory_order_buffer_read_used;
		memory_order_buffer_line_t* get_next_op_load();
		// Pointers to retain oldests memory operations
		memory_order_buffer_line_t *oldest_read_to_send;
		// ======================
		//WRITE
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_write;
		uint32_t memory_order_buffer_write_start;
        uint32_t memory_order_buffer_write_end;
        uint32_t memory_order_buffer_write_used;
		memory_order_buffer_line_t* get_next_op_store();
		// Pointers to retain oldests memory operations
		memory_order_buffer_line_t *oldest_write_to_send;
		// ======================
		//HIVE
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_hive;
		uint32_t memory_order_buffer_hive_start;
        uint32_t memory_order_buffer_hive_end;
        uint32_t memory_order_buffer_hive_used;
		memory_order_buffer_line_t* get_next_op_hive();
		// Pointers to retain oldests memory operations
		memory_order_buffer_line_t *oldest_hive_to_send;
		// ======================
		//VIMA
		// ======================
		memory_order_buffer_line_t *memory_order_buffer_vima;
		uint32_t memory_order_buffer_vima_start;
        uint32_t memory_order_buffer_vima_end;
        uint32_t memory_order_buffer_vima_used;
		memory_order_buffer_line_t* get_next_op_vima();
		// Pointers to retain oldests memory operations
		memory_order_buffer_line_t *oldest_vima_to_send;
		// ======================
		// Parallel requests
		uint32_t counter_mshr_read;
		uint32_t counter_mshr_write;
		int32_t request_DRAM;
		// ======================
		//Reservation Station 
		container_ptr_reorder_buffer_line_t unified_reservation_station;
		// ====================== 
		// ======================
		// Funcional Unitis - FUs
		// ======================
		// Integer FUs
		uint64_t *fu_int_alu;
		uint64_t *fu_int_mul;
		uint64_t *fu_int_div;
		// Floating Points FUs
		uint64_t *fu_fp_alu;
		uint64_t *fu_fp_mul;
		uint64_t *fu_fp_div;
		// Memory FUs
		uint64_t *fu_mem_load;
		uint64_t *fu_mem_store;
		uint64_t *fu_mem_hive;
		uint64_t *fu_mem_vima;
		//container to accelerate  execution
		container_ptr_reorder_buffer_line_t unified_functional_units;

		INSTANTIATE_GET_SET(uint32_t,processor_id)
		// ====================================================================
		// Statistics
		// ====================================================================

		INSTANTIATE_GET_SET_ADD(uint64_t,registerWrite)
		/////
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_FetchBuffer)
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_wrong_branch)
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_DecodeBuffer)
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_MOB_Read)
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_MOB_Write)
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_full_ROB)
		INSTANTIATE_GET_SET_ADD(uint64_t,stall_empty_RS)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_read_false_positive)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_disambiguation_write_false_positive)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_address_to_address)
		INSTANTIATE_GET_SET_ADD(uint64_t,times_reach_parallel_requests_read)
		INSTANTIATE_GET_SET_ADD(uint64_t,times_reach_parallel_requests_write)
		INSTANTIATE_GET_SET_ADD(float_t,instruction_per_cycle)
		INSTANTIATE_GET_SET_ADD(uint64_t,ended_cycle)
		INSTANTIATE_GET_SET_ADD(uint32_t,core_ram_requests)
		INSTANTIATE_GET_SET_ADD(uint64_t,mem_req_wait_cycles)
		INSTANTIATE_GET_SET_ADD(uint64_t,core_ram_request_wait_cycles)
		// ====================================================================
		// Statistics inst completed
		// ====================================================================
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_branch_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_div_alu_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_div_fp_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_int_alu_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_int_fp_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_mul_alu_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_mul_fp_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_hive_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_vima_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_load_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_store_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_nop_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_other_completed)

		INSTANTIATE_GET_SET_ADD(uint32_t,HAS_HIVE)
		INSTANTIATE_GET_SET_ADD(uint32_t,HAS_VIMA)

		INSTANTIATE_GET_SET_ADD(uint32_t,FETCH_WIDTH)
		INSTANTIATE_GET_SET_ADD(uint32_t,DECODE_WIDTH)
		INSTANTIATE_GET_SET_ADD(uint32_t,RENAME_WIDTH)
		INSTANTIATE_GET_SET_ADD(uint32_t,DISPATCH_WIDTH)
		INSTANTIATE_GET_SET_ADD(uint32_t,EXECUTE_WIDTH)
		INSTANTIATE_GET_SET_ADD(uint32_t,COMMIT_WIDTH)

		INSTANTIATE_GET_SET_ADD(uint32_t,FETCH_LATENCY)
		INSTANTIATE_GET_SET_ADD(uint32_t,DECODE_LATENCY)
		INSTANTIATE_GET_SET_ADD(uint32_t,RENAME_LATENCY)
		INSTANTIATE_GET_SET_ADD(uint32_t,DISPATCH_LATENCY)
		INSTANTIATE_GET_SET_ADD(uint32_t,EXECUTE_LATENCY)
		INSTANTIATE_GET_SET_ADD(uint32_t,COMMIT_LATENCY)

		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_INTEGER_ALU)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_INT_ALU)
		INSTANTIATE_GET_SET_ADD(uint32_t,INTEGER_ALU)
		// INTEGER MUL
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_INTEGER_MUL)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_INT_MUL)
		INSTANTIATE_GET_SET_ADD(uint32_t,INTEGER_MUL)
		// INTEGER DIV
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_INTEGER_DIV)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_INT_DIV)
		INSTANTIATE_GET_SET_ADD(uint32_t,INTEGER_DIV)

		INSTANTIATE_GET_SET_ADD(uint32_t,QTDE_INTEGER_FU)

		//FP ULAS LATENCY 
		// FLOATING POINT DIV
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_FP_DIV)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_FP_DIV)
		INSTANTIATE_GET_SET_ADD(uint32_t,FP_DIV)
		// FLOATING POINT MUL
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_FP_MUL)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_FP_MUL)
		INSTANTIATE_GET_SET_ADD(uint32_t,FP_MUL)
		// FLOATING POINT ALU
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_FP_ALU)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_FP_ALU)
		INSTANTIATE_GET_SET_ADD(uint32_t,FP_ALU)

		INSTANTIATE_GET_SET_ADD(uint32_t,QTDE_FP_FU)

		INSTANTIATE_GET_SET_ADD(uint32_t,PARALLEL_LOADS)
		INSTANTIATE_GET_SET_ADD(uint32_t,PARALLEL_STORES)

		// ======================
		///UNIFIED FUS

		// PROCESSOR BUFFERS SIZE
		INSTANTIATE_GET_SET_ADD(uint32_t,FETCH_BUFFER)
		INSTANTIATE_GET_SET_ADD(uint32_t,DECODE_BUFFER)
		INSTANTIATE_GET_SET_ADD(uint32_t,RAT_SIZE)
		INSTANTIATE_GET_SET_ADD(uint32_t,ROB_SIZE)
		INSTANTIATE_GET_SET_ADD(uint32_t,UNIFIED_RS)
		//MOB
		INSTANTIATE_GET_SET_ADD(uint32_t,MOB_READ)
		INSTANTIATE_GET_SET_ADD(uint32_t,MOB_WRITE)
		INSTANTIATE_GET_SET_ADD(uint32_t,MOB_HIVE)
		INSTANTIATE_GET_SET_ADD(uint32_t,MOB_VIMA)
		// =====================

		// =====================
		// MEMORY FU
		// =====================
		// Load Units
		INSTANTIATE_GET_SET_ADD(uint32_t,LOAD_UNIT)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_MEM_LOAD)
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_MEM_LOAD)
		// Store Units
		INSTANTIATE_GET_SET_ADD(uint32_t,STORE_UNIT)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_MEM_STORE)
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_MEM_STORE)

		INSTANTIATE_GET_SET_ADD(uint32_t,HIVE_UNIT)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_MEM_HIVE)
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_MEM_HIVE)

		INSTANTIATE_GET_SET_ADD(uint32_t,VIMA_UNIT)
		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_NEXT_MEM_VIMA)
		INSTANTIATE_GET_SET_ADD(uint32_t,LATENCY_MEM_VIMA)

		INSTANTIATE_GET_SET_ADD(uint32_t,QTDE_MEMORY_FU)

		INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)

		INSTANTIATE_GET_SET_ADD(uint32_t,RAM_LATENCY)
		INSTANTIATE_GET_SET_ADD(uint32_t,PARALLEL_LIM_ACTIVE)
		INSTANTIATE_GET_SET_ADD(uint32_t,MAX_PARALLEL_REQUESTS_CORE)

		INSTANTIATE_GET_SET_ADD(uint32_t,PREFETCHER_ACTIVE)

		INSTANTIATE_GET_SET_ADD(uint32_t,DISAMBIGUATION_ENABLED)

		INSTANTIATE_GET_SET_ADD(uint32_t,DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,PROCESSOR_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,FETCH_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,DECODE_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,RENAME_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,DISPATCH_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,EXECUTE_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,MOB_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,PRINT_MOB)
		INSTANTIATE_GET_SET_ADD(uint32_t,PRINT_ROB)
		INSTANTIATE_GET_SET_ADD(uint32_t,HIVE_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,VIMA_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,COMMIT_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,MSHR_DEBUG)
		INSTANTIATE_GET_SET_ADD(uint32_t,MULTICORE_DEBUG)

		INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)

		INSTANTIATE_GET_SET_ADD(uint32_t,INSTRUCTION_CACHES)
		INSTANTIATE_GET_SET_ADD(uint32_t,DATA_CACHES)
		INSTANTIATE_GET_SET_ADD(uint32_t,CACHE_LEVELS)
		
		// ====================================================================
};
