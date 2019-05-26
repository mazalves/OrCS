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
	uint64_t stat_inst_nop_completed;
	uint64_t stat_inst_load_completed;
	uint64_t stat_inst_store_completed;
	uint64_t stat_inst_branch_completed;
	uint64_t stat_inst_other_completed;

	uint32_t FETCH_WIDTH = 4;
	uint32_t DECODE_WIDTH = 4;
	uint32_t RENAME_WIDTH = 4;
	uint32_t DISPATCH_WIDTH = 4;
	uint32_t EXECUTE_WIDTH = 4;
	uint32_t COMMIT_WIDTH = 4;

	uint32_t FETCH_LATENCY = 3;
	uint32_t DECODE_LATENCY = 3;
	uint32_t RENAME_LATENCY = 3;
	uint32_t DISPATCH_LATENCY = 2;
	uint32_t EXECUTE_LATENCY = 0;
	uint32_t COMMIT_LATENCY = 3;

	uint32_t LATENCY_INTEGER_ALU = 1;
	uint32_t WAIT_NEXT_INT_ALU = 1;
	uint32_t INTEGER_ALU = 3;
	// INTEGER MUL
	uint32_t LATENCY_INTEGER_MUL = 3;
	uint32_t WAIT_NEXT_INT_MUL = 1;
	uint32_t INTEGER_MUL = 1;
	// INTEGER DIV
	uint32_t LATENCY_INTEGER_DIV = 32;
	uint32_t WAIT_NEXT_INT_DIV = 32;
	uint32_t INTEGER_DIV = 1;

	uint32_t QTDE_INTEGER_FU = (INTEGER_ALU+INTEGER_MUL+INTEGER_DIV);

	//FP ULAS LATENCY 
	// FLOATING POINT DIV
	uint32_t LATENCY_FP_DIV = 10;
	uint32_t WAIT_NEXT_FP_DIV = 10;
	uint32_t FP_DIV = 1;
	// FLOATING POINT MUL
	uint32_t LATENCY_FP_MUL = 5;
	uint32_t WAIT_NEXT_FP_MUL = 1;
	uint32_t FP_MUL = 1;
	// FLOATING POINT ALU
	uint32_t LATENCY_FP_ALU = 3;
	uint32_t WAIT_NEXT_FP_ALU = 1;
	uint32_t FP_ALU = 1;

	uint32_t QTDE_FP_FU = (FP_ALU+FP_MUL+FP_DIV);

	uint32_t PARALLEL_LOADS = 2;
	uint32_t PARALLEL_STORES = 1;

	// ======================
	///UNIFIED FUS

	// PROCESSOR BUFFERS SIZE
	uint32_t FETCH_BUFFER = 18;
	uint32_t DECODE_BUFFER = 28;
	uint32_t RAT_SIZE = 260;
	uint32_t ROB_SIZE = 168;
	uint32_t UNIFIED_RS = 54;
	//MOB
	uint32_t MOB_READ = 64;
	uint32_t MOB_WRITE = 36;
	// =====================

	// =====================
	// MEMORY FU
	// =====================
	// Load Units
	uint32_t LOAD_UNIT = 2;
	uint32_t WAIT_NEXT_MEM_LOAD = 1;
	uint32_t LATENCY_MEM_LOAD = 1;
	// Store Units
	uint32_t STORE_UNIT = 1;
	uint32_t WAIT_NEXT_MEM_STORE = 1;
	uint32_t LATENCY_MEM_STORE = 1;

	uint32_t QTDE_MEMORY_FU = (LOAD_UNIT+STORE_UNIT);

	//uint32_t KILO = 1024;
	//uint32_t MEGA = KILO*KILO;

	uint32_t LINE_SIZE = 64;

	uint32_t L1_DATA_SIZE = 32*KILO;
	uint32_t L1_DATA_ASSOCIATIVITY = 8;
	uint32_t L1_DATA_LATENCY = 3;
	uint32_t L1_DATA_SETS = (L1_DATA_SIZE/LINE_SIZE)/L1_DATA_ASSOCIATIVITY;
	// I$
	uint32_t L1_INST_SIZE = 32*KILO;
	uint32_t L1_INST_ASSOCIATIVITY = 8;
	uint32_t L1_INST_LATENCY = 3;
	uint32_t L1_INST_SETS = (L1_INST_SIZE/LINE_SIZE)/L1_INST_ASSOCIATIVITY;

	uint32_t LLC_SIZE = 20*MEGA;
	uint32_t LLC_ASSOCIATIVITY = 20;
	uint32_t LLC_LATENCY = 44;
	uint32_t LLC_SETS = (LLC_SIZE/LINE_SIZE)/LLC_ASSOCIATIVITY;

	uint32_t RAM_LATENCY = 350;
	uint32_t PARALLEL_LIM_ACTIVE = 1;
	uint32_t MAX_PARALLEL_REQUESTS_CORE = 10;

	//uint32_t PREFETCHER_ACTIVE = 0;

	//uint32_t DESAMBIGUATION_ENABLED = 1;

	uint32_t DEBUG = 0;
	uint32_t FETCH_DEBUG = 0;
	uint32_t DECODE_DEBUG = 0;
	uint32_t RENAME_DEBUG = 0;
	uint32_t DISPATCH_DEBUG = 0;
	uint32_t EXECUTE_DEBUG = 0;
	uint32_t MOB_DEBUG = 0;
	uint32_t PRINT_MOB = 0;
	uint32_t PRINT_ROB = 0;
	uint32_t COMMIT_DEBUG = 0;

	uint32_t WAIT_CYCLE = 0;

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
		
		// ====================================================================
		/// Methods
		// ====================================================================
		processor_t();
		~processor_t();
	    void allocate();
	    void clock();
		void statistics();
		void printConfiguration();
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
		// Stage Methods
		// ====================================================================
		void fetch();
		void decode();
		void rename();
		void dispatch();
		void execute();

		uint32_t mob_read();
		uint32_t mob_write();
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
		desambiguation_t *desambiguator;
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
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_load_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_store_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_nop_completed)
		INSTANTIATE_GET_SET_ADD(uint64_t,stat_inst_other_completed)
		// ====================================================================
};
