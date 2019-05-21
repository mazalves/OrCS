#include "./../simulator.hpp"

// =====================================================================
processor_t::processor_t()
{
	//Setting Pointers to NULL
	// ========OLDEST MEMORY OPERATIONS POINTER======
	this->oldest_read_to_send = NULL;
	this->oldest_write_to_send = NULL;
	// ========MOB======
	this->memory_order_buffer_read = NULL;
	this->memory_order_buffer_write = NULL;
	//=========DESAMBIGUATION ============
	this->desambiguator = NULL;
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

}
processor_t::~processor_t()
{
	//Memory structures
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_read);
	utils_t::template_delete_array<memory_order_buffer_line_t>(this->memory_order_buffer_write);
	utils_t::template_delete_variable<desambiguation_t>(this->desambiguator);
	//auxiliar var to maintain status oldest instruction
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_read_to_send);
	utils_t::template_delete_variable<memory_order_buffer_line_t>(this->oldest_write_to_send);

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
	// =====================================================================
}
// =====================================================================
void processor_t::allocate()
{
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
	//desambiguator
	this->desambiguator = new desambiguation_t;
	this->desambiguator->allocate();
	// parallel requests
	// =========================================================================================
	//DRAM
	// =========================================================================================
	this->counter_mshr_read = 0;
	this->counter_mshr_write = 0;
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
	#if COMMIT_DEBUG
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
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("==========\n")
			ORCS_PRINTF("RM MOB Write Entry \n%s\n", this->memory_order_buffer_write[this->memory_order_buffer_write_start].content_to_string().c_str())
			ORCS_PRINTF("==========\n")
		}
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


void processor_t::fetch(){
	#if FETCH_DEBUG
		ORCS_PRINTF("Fetch Stage\n")
	#endif
	opcode_package_t operation;
	// uint32_t position;
	// Trace ->fetchBuffer
	for (int i = 0; i < FETCH_WIDTH; i++)
	{
		operation.package_clean();
		bool updated = false;
		//=============================
		//Stall full fetch buffer
		//=============================
		if (this->fetchBuffer.is_full())
		{
			this->add_stall_full_FetchBuffer();
			break;
		}
		//=============================
		//Stall branch wrong predict
		//=============================
		if (this->get_stall_wrong_branch() > orcs_engine.get_global_cycle())
		{
			break;
		}
		//=============================
		//Get new Opcode
		//=============================
		if (!orcs_engine.trace_reader[this->processor_id].trace_fetch(&operation))
		{
			this->traceIsOver = true;
			break;
		}
		#if FETCH_DEBUG

				ORCS_PRINTF("Opcode Fetched %s\n", operation.content_to_string2().c_str())

		#endif
		//============================
		//add control variables
		//============================
		operation.opcode_number = this->fetchCounter;
		this->fetchCounter++;
		//============================
		///Solve Branch
		//============================

		if (this->hasBranch)
		{
			//solve
			uint32_t stallWrongBranch = orcs_engine.branchPredictor[this->processor_id].solveBranch(this->previousBranch, operation);
			this->set_stall_wrong_branch(orcs_engine.get_global_cycle() + stallWrongBranch);
			this->hasBranch = false;
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(this->processor_id,operation.opcode_address);
			// ORCS_PRINTF("ready after wrong branch %lu\n",this->get_stall_wrong_branch()+ttc)
			operation.updatePackageReady(FETCH_LATENCY+stallWrongBranch + ttc);
			updated = true;
			this->previousBranch.package_clean();
			// ORCS_PRINTF("Stall Wrong Branch %u\n",stallWrongBranch)
		}
		//============================
		// Operation Branch, set flag
		//============================
		if (operation.opcode_operation == INSTRUCTION_OPERATION_BRANCH)
		{
			orcs_engine.branchPredictor[this->processor_id].branches++;
			this->previousBranch = operation;
			this->hasBranch = true;
		}
		//============================
		//Insert into fetch buffer
		//============================
		if (POSITION_FAIL == this->fetchBuffer.push_back(operation))
		{
			break;
		}
		if (!updated)
		{
			uint32_t ttc = orcs_engine.cacheManager->searchInstruction(this->processor_id,operation.opcode_address);
			this->fetchBuffer.back()->updatePackageReady(FETCH_LATENCY + ttc);

		}
	}
		// #if FETCH_DEBUG
		// 	if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
		// 		for(uint32_t i = 0;i < this->fetchBuffer.size;i++){
		// 			ORCS_PRINTF("Opcode list-> %s\n", this->fetchBuffer[i].content_to_string2().c_str())

		// 		}
		// 	}
		// #endif
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

	#if DECODE_DEBUG
		ORCS_PRINTF("Decode Stage\n")
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
				ORCS_PRINTF("Opcode to decode %s\n", this->fetchBuffer.front()->content_to_string2().c_str())
			}
	#endif
	uop_package_t new_uop;
	int32_t statusInsert = POSITION_FAIL;
	for (size_t i = 0; i < DECODE_WIDTH; i++)
	{
		if (this->fetchBuffer.is_empty() ||
			this->fetchBuffer.front()->status != PACKAGE_STATE_READY ||
			this->fetchBuffer.front()->readyAt > orcs_engine.get_global_cycle())
		{
			break;
		}
		if (this->decodeBuffer.get_capacity() - this->decodeBuffer.get_size() < MAX_UOP_DECODED)
		{
			this->add_stall_full_DecodeBuffer();
			break;
		}
		ERROR_ASSERT_PRINTF(this->decodeCounter == this->fetchBuffer.front()->opcode_number, "Trying decode out-of-order");
		this->decodeCounter++;

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
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
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
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
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
			new_uop.updatePackageReady(DECODE_LATENCY);
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		// =====================
		//Decode Branch
		// =====================
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
			new_uop.updatePackageReady(DECODE_LATENCY);
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
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
			if (this->fetchBuffer.front()->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE)
			{
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
				// assert(!inserted_258 && "Max registers used");
				// ===== Write Regs =============================================
				/// Clear WRegs
				for (uint32_t i = 0; i < MAX_REGISTERS; i++)
				{
					new_uop.write_regs[i] = POSITION_FAIL;
				}
			}
			new_uop.updatePackageReady(DECODE_LATENCY);
			// printf("\n UOP Created %s \n",new_uop.content_to_string().c_str());
			statusInsert = this->decodeBuffer.push_back(new_uop);
	#if DECODE_DEBUG
				ORCS_PRINTF("uop created %s\n", this->decodeBuffer.back()->content_to_string2().c_str())
	#endif
			ERROR_ASSERT_PRINTF(statusInsert != POSITION_FAIL, "Erro, Tentando decodificar mais uops que o maximo permitido")
		}
		this->fetchBuffer.pop_front();
	}
}

// ============================================================================
void processor_t::update_registers(reorder_buffer_line_t *new_rob_line){
	/// Control the Register Dependency - Register READ
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
		if (new_rob_line->uop.read_regs[k] < 0)
		{
			break;
		}
		uint32_t read_register = new_rob_line->uop.read_regs[k];
		ERROR_ASSERT_PRINTF(read_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n", read_register, RAT_SIZE);
		/// If there is a dependency
		if (this->register_alias_table[read_register] != NULL)
		{
			for (uint32_t j = 0; j < ROB_SIZE; j++)
			{
				if (this->register_alias_table[read_register]->reg_deps_ptr_array[j] == NULL)
				{
					this->register_alias_table[read_register]->wake_up_elements_counter++;
					this->register_alias_table[read_register]->reg_deps_ptr_array[j] = new_rob_line;
					new_rob_line->wait_reg_deps_number++;
					break;
				}
			}
		}
	}

	/// Control the Register Dependency - Register WRITE
	for (uint32_t k = 0; k < MAX_REGISTERS; k++)
	{
		this->add_registerWrite();
		if (new_rob_line->uop.write_regs[k] < 0)
		{
			break;
		}
		uint32_t write_register = new_rob_line->uop.write_regs[k];
		ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Write Register (%d) > Register Alias Table Size (%d)\n", write_register, RAT_SIZE);

		this->register_alias_table[write_register] = new_rob_line;
	}
}
// ============================================================================
void processor_t::rename(){
	#if RENAME_DEBUG
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
			this->decodeBuffer.front()->status != PACKAGE_STATE_READY ||
			this->decodeBuffer.front()->readyAt > orcs_engine.get_global_cycle())
		{
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
				#if RENAME_DEBUG
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				#endif
				this->add_stall_full_MOB_Read();
				break;
			}
			#if RENAME_DEBUG
				ORCS_PRINTF("Get_Position_MOB_READ %d\n",pos_mob)
			#endif
			mob_line = &this->memory_order_buffer_read[pos_mob];
		}
		//=======================
		// Memory Operation Write
		//=======================
		if (this->decodeBuffer.front()->uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
			if(	this->memory_order_buffer_write_used>=MOB_WRITE ||
				this->robUsed>=ROB_SIZE )break;
			pos_mob = this->search_position_mob_write();
			if (pos_mob == POSITION_FAIL)
			{
				#if RENAME_DEBUG
					ORCS_PRINTF("Stall_MOB_Read_Full\n")
				#endif
				this->add_stall_full_MOB_Write();
				break;
			}
			#if RENAME_DEBUG
				ORCS_PRINTF("Get_Position_MOB_WRITE %d\n",pos_mob)
			#endif
			mob_line = &this->memory_order_buffer_write[pos_mob];
		}
		//=======================
		// Verificando se tem espaco no ROB se sim bamos inserir
		//=======================
		pos_rob = this->searchPositionROB();
		if (pos_rob == POSITION_FAIL)
		{
			#if RENAME_DEBUG
				ORCS_PRINTF("Stall_MOB_Read_Full\n")
			#endif
			this->add_stall_full_ROB();
			break;
		}
		// ===============================================
		// Insserting on ROB
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
		this->reorderBuffer[pos_rob].uop.updatePackageReady(RENAME_LATENCY + DISPATCH_LATENCY);
		this->reorderBuffer[pos_rob].mob_ptr = mob_line;
		this->reorderBuffer[pos_rob].processor_id = this->processor_id;
		// =======================
		// Making registers dependences
		// =======================
		this->update_registers(&this->reorderBuffer[pos_rob]);
	#if RENAME_DEBUG
			ORCS_PRINTF("Rename %s\n", this->reorderBuffer[pos_rob].content_to_string().c_str())
	#endif
		// =======================
		// Insert into Reservation Station
		// =======================
		this->unified_reservation_station.push_back(&this->reorderBuffer[pos_rob]);
		// =======================
		// Insert into MOB.
		// =======================
		if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_LOAD)
		{
	#if RENAME_DEBUG
				ORCS_PRINTF("Mem Load\n")
	#endif
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_READ;
			this->reorderBuffer[pos_rob].mob_ptr->status = PACKAGE_STATE_WAIT;
			this->reorderBuffer[pos_rob].mob_ptr->readyToGo = orcs_engine.get_global_cycle() + RENAME_LATENCY + DISPATCH_LATENCY;
			this->reorderBuffer[pos_rob].mob_ptr->uop_number = this->reorderBuffer[pos_rob].uop.uop_number;
			this->reorderBuffer[pos_rob].mob_ptr->processor_id = this->processor_id;
		}
		else if (this->reorderBuffer[pos_rob].uop.uop_operation == INSTRUCTION_OPERATION_MEM_STORE)
		{
	#if RENAME_DEBUG
				ORCS_PRINTF("Mem Store\n")
	#endif
			this->reorderBuffer[pos_rob].mob_ptr->opcode_address = this->reorderBuffer[pos_rob].uop.opcode_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_address = this->reorderBuffer[pos_rob].uop.memory_address;
			this->reorderBuffer[pos_rob].mob_ptr->memory_size = this->reorderBuffer[pos_rob].uop.memory_size;
			this->reorderBuffer[pos_rob].mob_ptr->memory_operation = MEMORY_OPERATION_WRITE;
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
	#if DESAMBIGUATION_ENABLED
				this->desambiguator->make_memory_dependences(this->reorderBuffer[pos_rob].mob_ptr);
	#endif
		}
	} //end for
}
// ============================================================================
void processor_t::dispatch(){
	#if DISPATCH_DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("====================================================================\n")
			ORCS_PRINTF("Dispatch Stage\n")
			ORCS_PRINTF("====================================================================\n")
		}
	#endif
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

		for (uint32_t i = 0; i < this->unified_reservation_station.size() && i < UNIFIED_RS; i++)
		{
			//pointer to entry
			reorder_buffer_line_t *rob_line = this->unified_reservation_station[i];
			#if DISPATCH_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("cycle %lu\n", orcs_engine.get_global_cycle())
					ORCS_PRINTF("=================\n")
					ORCS_PRINTF("Unified Reservations Station on use: %lu\n",this->unified_reservation_station.size())
					ORCS_PRINTF("Trying Dispatch %s\n", rob_line->content_to_string().c_str())
					ORCS_PRINTF("=================\n")
				}
			#endif

			if (total_dispatched >= DISPATCH_WIDTH){
				break;
			}

			if ((rob_line->uop.readyAt <= orcs_engine.get_global_cycle()) &&
				(rob_line->wait_reg_deps_number == 0)){
				ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY, "Error, uop not ready being dispatched\n %s\n", rob_line->content_to_string().c_str())
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
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_ALU);
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
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_MUL);
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
								rob_line->uop.updatePackageReady(LATENCY_INTEGER_DIV);
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
								rob_line->uop.updatePackageReady(LATENCY_FP_ALU);
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
								rob_line->uop.updatePackageReady(LATENCY_FP_MUL);
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
								rob_line->uop.updatePackageReady(LATENCY_FP_DIV);
								break;
							}
						}
					}
					break;
				// ====================================================
				// Operation LOAD
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
								rob_line->uop.updatePackageReady(LATENCY_MEM_LOAD);
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
								rob_line->uop.updatePackageReady(LATENCY_MEM_STORE);
								break;
							}
						}
					}
					break;

				// ====================================================
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
					ERROR_PRINTF("Invalid instruction BARRIER||HMC_ROA||HMC_ROWA being dispatched.\n");
					break;
				} //end switch
				//remover os postos em execucao aqui
				if (dispatched == true)
				{
			#if DISPATCH_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("Dispatched %s\n", rob_line->content_to_string().c_str())
						ORCS_PRINTF("===================================================================\n")
					}
			#endif
					// update Dispatched
					total_dispatched++;
					// insert on FUs waiting structure
					this->unified_functional_units.push_back(rob_line);
					// remove from reservation station
					this->unified_reservation_station.erase(this->unified_reservation_station.begin() + i);
					i--;
				} //end if dispatched
			}	 //end if robline is ready
		}		  //end for
		// sleep(1);
} //end method
// ============================================================================
void processor_t::execute()
{
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Execute Stage ==========\n")
		}
	#endif
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
			#if DESAMBIGUATION_ENABLED
				this->desambiguator->solve_memory_dependences(&this->memory_order_buffer_read[pos]);
			#endif
			#if PARALLEL_LIM_ACTIVE
				if(!this->memory_order_buffer_read[pos].forwarded_data){
						ERROR_ASSERT_PRINTF(this->counter_mshr_read > 0,"ERRO, Contador negativo READ\n")
						this->counter_mshr_read--;
				}
			#endif
			if(this->memory_order_buffer_read[pos].waiting_DRAM){
				ERROR_ASSERT_PRINTF(this->request_DRAM > 0,"ERRO, Contador negativo Waiting DRAM\n")
				#if EXECUTE_DEBUG
					if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
						ORCS_PRINTF("\nReducing DRAM COUNTER\n\n")
					}
			#endif
				this->request_DRAM--;
			}
		}
		pos++;
		if( pos >= MOB_READ) pos=0;
	}
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
			ERROR_ASSERT_PRINTF(rob_line->uop.status == PACKAGE_STATE_READY, "FU with Package not in ready state")
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
				// MEMORY LOAD/STORE ==========================================
				case INSTRUCTION_OPERATION_MEM_LOAD:
				{
					ERROR_ASSERT_PRINTF(rob_line->mob_ptr != NULL, "Read with a NULL pointer to MOB\n%s\n",rob_line->content_to_string().c_str())
					this->memory_read_executed++;
					rob_line->mob_ptr->uop_executed = true;
					rob_line->uop.updatePackageReady(EXECUTE_LATENCY);
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
					rob_line->uop.updatePackageReady(EXECUTE_LATENCY);
					uop_total_executed++;
					/// Remove from the Functional Units
					this->unified_functional_units.erase(this->unified_functional_units.begin() + i);
					i--;
				}
				break;
				case INSTRUCTION_OPERATION_BARRIER:
				case INSTRUCTION_OPERATION_HMC_ROA:
				case INSTRUCTION_OPERATION_HMC_ROWA:
					ERROR_PRINTF("Invalid BARRIER | HMC ROA |HMC ROWA.\n");
					break;
			} //end switch
		#if EXECUTE_DEBUG
				if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
					ORCS_PRINTF("Executed %s\n", rob_line->content_to_string().c_str())
				}
		#endif
		} //end if ready package
	}	 //end for
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("Memory Operations Read Executed %u\n",this->memory_read_executed)
			ORCS_PRINTF("Memory Operations Write Executed %u\n",this->memory_write_executed)
			ORCS_PRINTF("Requests to DRAM on the Fly %d \n",this->request_DRAM)
			ORCS_PRINTF("Parallel Request Data %d \n",this->counter_mshr_read)
			ORCS_PRINTF("Parallel Write Data %d \n",this->counter_mshr_write)
		}
	#endif
		// =========================================================================
		// Verificar se foi executado alguma operação de leitura,
		//  e executar a mais antiga no MOB
		// =========================================================================
		if(this->memory_read_executed!=0){
			this->mob_read();
		}

		// ==================================
		// Executar o MOB Write, com a escrita mais antiga.
		// depois liberar e tratar as escrita prontas;
		// ==================================

		if(this->memory_write_executed!=0){
			this->mob_write();
		}
		// =====================================
	#if EXECUTE_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	#endif

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
	#if MOB_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
			ORCS_PRINTF("=========== MOB Read ===========\n")
			ORCS_PRINTF("Parallel Requests %d > MAX\n",this->counter_mshr_read)
			ORCS_PRINTF("MOB Read Start %u\n",this->memory_order_buffer_read_start)
			ORCS_PRINTF("MOB Read End %u\n",this->memory_order_buffer_read_end)
			ORCS_PRINTF("MOB Read Used %u\n",this->memory_order_buffer_read_used)
			#if PRINT_MOB
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					memory_order_buffer_line_t::printAllOrder(this->memory_order_buffer_read,MOB_READ,this->memory_order_buffer_read_start,this->memory_order_buffer_read_used);
				}
			#endif
			if(oldest_read_to_send!=NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("MOB Read Atual %s\n",this->oldest_read_to_send->content_to_string().c_str())
				}
			}
		}
	#endif
	if(this->oldest_read_to_send == NULL){

			this->oldest_read_to_send = this->get_next_op_load();
			#if MOB_DEBUG
				if(oldest_read_to_send==NULL){
					if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
						ORCS_PRINTF("Oldest Read NULL\n")
					}
				}
			#endif
	}
	if (this->oldest_read_to_send != NULL){
		#if PARALLEL_LIM_ACTIVE
			if(this->counter_mshr_read >= MAX_PARALLEL_REQUESTS_CORE){
				this->add_times_reach_parallel_requests_read();
				return FAIL;
			}
		#endif
		#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("=================================\n")
				ORCS_PRINTF("Sending to memory request to data\n")
				ORCS_PRINTF("%s\n",this->oldest_read_to_send->content_to_string().c_str())
				ORCS_PRINTF("=================================\n")
			}
		#endif
		uint32_t ttc = orcs_engine.cacheManager->searchData(this->oldest_read_to_send);
		this->oldest_read_to_send->cycle_send_request = orcs_engine.get_global_cycle(); //Cycle which sent request to memory system
		this->oldest_read_to_send->updatePackageReady(ttc);
		this->oldest_read_to_send->sent=true;
		this->oldest_read_to_send->rob_ptr->sent=true;								///Setting flag which marks sent request. set to remove entry on mob at commit
		#if PARALLEL_LIM_ACTIVE
			this->counter_mshr_read++; //numero de req paralelas, add+1
		#endif
		this->oldest_read_to_send = NULL;
	} //end if mob_line null
	#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
		}
	#endif
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
	#if MOB_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("==========================================================\n")
			ORCS_PRINTF("=========== MOB Write ===========\n")
			ORCS_PRINTF("MOB Write Start %u\n",this->memory_order_buffer_write_start)
			ORCS_PRINTF("MOB Write End %u\n",this->memory_order_buffer_write_end)
			ORCS_PRINTF("MOB Write Used %u\n",this->memory_order_buffer_write_used)
			#if PRINT_MOB
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
					memory_order_buffer_line_t::printAllOrder(this->memory_order_buffer_write,MOB_WRITE,this->memory_order_buffer_write_start,this->memory_order_buffer_write_used);
				}
			#endif
			if(this->oldest_write_to_send!=NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("MOB write Atual %s\n",this->oldest_write_to_send->content_to_string().c_str())
				}
			}
		}
	#endif
	if(this->oldest_write_to_send==NULL){
		this->oldest_write_to_send = this->get_next_op_store();
	//////////////////////////////////////
		#if MOB_DEBUG
			if(this->oldest_write_to_send==NULL){
				if(orcs_engine.get_global_cycle() > WAIT_CYCLE){
					ORCS_PRINTF("Oldest Write NULL\n")
				}
			}
		#endif
	/////////////////////////////////////////////
	}
	if (this->oldest_write_to_send != NULL)
	{
			#if PARALLEL_LIM_ACTIVE
				if (this->counter_mshr_write >= MAX_PARALLEL_REQUESTS_CORE)
				{
					this->add_times_reach_parallel_requests_write();
					return FAIL;
				}
			#endif
		uint32_t ttc = 0;
		#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("=================================\n")
				ORCS_PRINTF("Sending to memory WRITE to data\n")
				ORCS_PRINTF("%s\n",this->oldest_write_to_send->content_to_string().c_str())
				ORCS_PRINTF("=================================\n")
			}
		#endif

		//sendind to write data
		ttc = orcs_engine.cacheManager->writeData(oldest_write_to_send);
		// updating package
		// =============================================================
		this->oldest_write_to_send->rob_ptr->stage = PROCESSOR_STAGE_COMMIT;
		this->oldest_write_to_send->rob_ptr->uop.updatePackageReady(ttc);
		this->oldest_write_to_send->rob_ptr->sent = true;
		//MOB
		this->oldest_write_to_send->sent = true;
		this->oldest_write_to_send->updatePackageReady(ttc);
		this->solve_registers_dependency(this->oldest_write_to_send->rob_ptr);
		this->desambiguator->solve_memory_dependences(this->oldest_write_to_send);
		this->remove_front_mob_write();
		#if PARALLEL_LIM_ACTIVE
			this->counter_mshr_write++; //numero de req paralelas, add+1
		#endif
		this->memory_write_executed--; //numero de writes executados
		this->oldest_write_to_send=NULL;
		// =============================================================
	} //end if mob_line null
		#if MOB_DEBUG
			if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
				ORCS_PRINTF("Parallel Requests %d > MAX\n",this->counter_mshr_write)
				ORCS_PRINTF("==========================================================\n")
			}
		#endif
	return OK;
}
// ============================================================================
void processor_t::commit(){
	#if COMMIT_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
		{
			ORCS_PRINTF("=========================================================================\n")
			ORCS_PRINTF("========== Commit Stage ==========\n")
			ORCS_PRINTF("Cycle %lu\n", orcs_engine.get_global_cycle())
			ORCS_PRINTF("ROB Head %s\n",this->reorderBuffer[this->robStart].content_to_string().c_str())
			#if PRINT_ROB
				this->print_ROB();
			#endif
			ORCS_PRINTF("==================================\n")
		}
	#endif
	int32_t pos_buffer;
	/// Commit the packages
	for (uint32_t i = 0; i < COMMIT_WIDTH; i++){
		pos_buffer = this->robStart;
		if (this->robUsed != 0 &&
			this->reorderBuffer[pos_buffer].stage == PROCESSOR_STAGE_COMMIT &&
			this->reorderBuffer[pos_buffer].uop.status == PACKAGE_STATE_READY &&
			this->reorderBuffer[pos_buffer].uop.readyAt <= orcs_engine.get_global_cycle())
		{
		#if !LOCKING_COMMIT
		if(this->verify_uop_on_emc(&this->reorderBuffer[pos_buffer])){
			break;
		}
		#endif
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

				// MEMORY OPERATIONS - READ
				case INSTRUCTION_OPERATION_MEM_LOAD:{
					if(this->reorderBuffer[pos_buffer].mob_ptr->waiting_DRAM){
						this->core_ram_request_wait_cycles+=(this->reorderBuffer[pos_buffer].mob_ptr->readyAt - this->reorderBuffer[pos_buffer].mob_ptr->cycle_send_request);
						this->add_core_ram_requests();
					}
					this->mem_req_wait_cycles+=(this->reorderBuffer[pos_buffer].mob_ptr->readyAt - this->reorderBuffer[pos_buffer].mob_ptr->readyToGo);
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
					ERROR_PRINTF("Invalid instruction BARRIER| HMC ROA | HMC ROWA.\n");
					break;
			}

			ERROR_ASSERT_PRINTF(uint32_t(pos_buffer) == this->robStart, "Commiting different from the position start\n");
			#if COMMIT_DEBUG
				if (orcs_engine.get_global_cycle() > WAIT_CYCLE)
				{
					ORCS_PRINTF("======================================\n")
					ORCS_PRINTF("RM ROB Entry \n%s\n", this->reorderBuffer[this->robStart].content_to_string().c_str())
				}
			#endif
			if(this->reorderBuffer[this->robStart].sent==true){
				if(this->reorderBuffer[this->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_LOAD){
					this->remove_front_mob_read();
				}
				else if(this->reorderBuffer[this->robStart].uop.uop_operation==INSTRUCTION_OPERATION_MEM_STORE){
					ERROR_ASSERT_PRINTF(this->counter_mshr_write > 0,"Erro, reduzindo requests paralelos abaixo de 0\n")
					this->counter_mshr_write--;
				}
			}
			this->removeFrontROB();
		}
		/// Could not commit the older, then stop looking for ready uops
		else
		{
			break;
		}
	}
		#if COMMIT_DEBUG
		if (orcs_engine.get_global_cycle() > WAIT_CYCLE){
			ORCS_PRINTF("=========================================================================\n")
		}
	#endif

} //end method
// ============================================================================
void processor_t::solve_registers_dependency(reorder_buffer_line_t *rob_line){

		/// Remove pointers from Register Alias Table (RAT)
		for (uint32_t j = 0; j < MAX_REGISTERS; j++)
		{
			if (rob_line->uop.write_regs[j] < 0)
			{
				break;
			}
			uint32_t write_register = rob_line->uop.write_regs[j];
			ERROR_ASSERT_PRINTF(write_register < RAT_SIZE, "Read Register (%d) > Register Alias Table Size (%d)\n",
								write_register, RAT_SIZE);
			if (this->register_alias_table[write_register] != NULL &&
				this->register_alias_table[write_register]->uop.uop_number == rob_line->uop.uop_number)
			{
				this->register_alias_table[write_register] = NULL;
			} //end if
		}	 //end for

		// =========================================================================
		// SOLVE REGISTER DEPENDENCIES - RAT
		// =========================================================================
		for (uint32_t j = 0; j < ROB_SIZE; j++)
		{
			/// There is an unsolved dependency
			if (rob_line->reg_deps_ptr_array[j] != NULL)
			{
				rob_line->wake_up_elements_counter--;
				rob_line->reg_deps_ptr_array[j]->wait_reg_deps_number--;
				/// This update the ready cycle, and it is usefull to compute the time each instruction waits for the functional unit
				if (rob_line->reg_deps_ptr_array[j]->uop.readyAt <= orcs_engine.get_global_cycle())
				{
					rob_line->reg_deps_ptr_array[j]->uop.readyAt = orcs_engine.get_global_cycle();
				}
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
			#if MAX_PARALLEL_REQUESTS_CORE
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_CORE_READ: %lu\n", this->get_times_reach_parallel_requests_read());
				fprintf(output, "Times_Reach_MAX_PARALLEL_REQUESTS_CORE_WRITE: %lu\n", this->get_times_reach_parallel_requests_write());
			#endif
		utils_t::largestSeparator(output);
		fprintf(output, "Instruction_Per_Cycle: %1.6lf\n", (float)this->fetchCounter/this->get_ended_cycle());
		// accessing LLC cache level
		int32_t *cache_indexes = new int32_t[2];
		orcs_engine.cacheManager->generateIndexArray(this->processor_id, cache_indexes);
		fprintf(output, "MPKI: %lf\n", (float)orcs_engine.cacheManager->data_cache[2][cache_indexes[2]].get_cache_miss()/((float)this->fetchCounter/1000));
		fprintf(output, "Average_wait_cycles_wait_mem_req: %lf\n", (float)this->mem_req_wait_cycles/this->get_stat_inst_load_completed());
		fprintf(output, "Core_Request_RAM_AVG_Cycle: %lf\n", (float)this->core_ram_request_wait_cycles/this->get_core_ram_requests());
		utils_t::largestSeparator(output);
		}
		if(close) fclose(output);
		this->desambiguator->statistics();
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
		fprintf(output, "===============Instruction $============\n");
		fprintf(output, "L1_INST_SIZE ->%u\n", L1_INST_SIZE);
		fprintf(output, "L1_INST_ASSOCIATIVITY ->%u\n", L1_INST_ASSOCIATIVITY);
		fprintf(output, "L1_INST_LATENCY ->%u\n", L1_INST_LATENCY);
		fprintf(output, "L1_INST_SETS ->%u\n", L1_INST_SETS);
		fprintf(output, "===============Data $ L1============\n");
		fprintf(output, "L1_DATA_SIZE ->%u\n", L1_DATA_SIZE);
		fprintf(output, "L1_DATA_ASSOCIATIVITY ->%u\n", L1_DATA_ASSOCIATIVITY);
		fprintf(output, "L1_DATA_LATENCY ->%u\n", L1_DATA_LATENCY);
		fprintf(output, "L1_DATA_SETS ->%u\n", L1_DATA_SETS);
		fprintf(output, "===============LLC ============\n");
		fprintf(output, "LLC_SIZE ->%u\n", LLC_SIZE);
		fprintf(output, "LLC_ASSOCIATIVITY ->%u\n", LLC_ASSOCIATIVITY);
		fprintf(output, "LLC_LATENCY ->%u\n", LLC_LATENCY);
		fprintf(output, "LLC_SETS ->%u\n", LLC_SETS);
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
			ORCS_PRINTF("============================PROCESSOR %u===============================\n",this->processor_id)
			ORCS_PRINTF("Cycle %lu\n",orcs_engine.get_global_cycle())
		}
	#endif
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
	#if DEBUG
		if(orcs_engine.get_global_cycle()>WAIT_CYCLE){
			ORCS_PRINTF("===================================================================\n")
			// sleep(1);
		}
	#endif
}
// ========================================================================================================================================================================================
