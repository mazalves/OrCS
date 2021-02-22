#include "./../simulator.hpp"
vector_map_table_t::vector_map_table_t  (int32_t num_entries, 
                                        register_rename_table_t *RRT,
                                        Vectorizer_t *vectorizer, 
                                        table_of_loads_t *TL, 
                                        circular_buffer_t <opcode_package_t> *inst_list) {
    // Inicializações
    this->entries_size = num_entries;
    this->entries = new vector_map_table_entry_t[this->entries_size];
    this->next_replacement = 0;

    this->TL = TL;
    this->vectorizer = vectorizer;
    this->register_rename_table = RRT;
    this->inst_list = inst_list;
}
        
vector_map_table_t::~vector_map_table_t () {

    // Libera alocações
    delete[] this->entries;

}

int32_t vector_map_table_t::allocate_entry () {
    int32_t r = next_replacement;
    ++next_replacement;

    if(next_replacement == entries_size) 
    	next_replacement = 0;

    return r;

}


bool vector_map_table_t::compare_registers (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry) {
    // Verifica interferência de instruções escalares
    if (register_rename_table[inst->read_regs[0]].vectorial && register_rename_table[inst->read_regs[1]].vectorial){

    	// Verifica se os VR se mantém
    	if (   (vrmt_entry->source_operand_1 == (uint64_t)register_rename_table[inst->read_regs[0]].correspondent_vectorial_reg)
       		&& (vrmt_entry->source_operand_2 == (uint64_t)register_rename_table[inst->read_regs[1]].correspondent_vectorial_reg) ) 
    	{
    		return true;
    	}
    }
    return false;
}

DV::DV_ERROR vector_map_table_t::convert_to_validation (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry, int32_t validation_index) {
    inst->is_vectorial_part = -1;
    inst->VR_id = vrmt_entry->correspondent_VR;
    inst->is_validation = true;
    inst->will_validate_offset = validation_index;

    return DV::SUCCESS;
}

vector_map_table_entry_t *vector_map_table_t::find_pc (uint64_t pc) {
    for (int32_t i = 0; i < this->entries_size; ++i) {
        if(this->entries[i].pc == pc)
            return &this->entries[i];
    }
    return NULL;
}

void vector_map_table_t::invalidate (vector_map_table_entry_t *vrmt_entry) {
    vrmt_entry->pc = 0;
}

bool vector_map_table_t::new_store (opcode_package_t *inst) {
    bool r = false;
    for (int32_t i = 0; i < this->entries_size; ++i) {
	    if(this->entries[i].is_load) {
	    	if((entries[i].source_operand_1 <= inst->write_address) &&
	    	  (entries[i].source_operand_2 > inst->write_address))
            {
                this->invalidate(&this->entries[i]);
		        r = true;
            }
        }
    }
    return r;
}

DV::DV_ERROR vector_map_table_t::validate (opcode_package_t *inst, vector_map_table_entry_t *vrmt_entry) {
    #if DV_DEBUG == 1
        printf("ACTION::Validation\n");
    #endif

    // **************************************
    // vector_map_table_t::validate (load)
    // **************************************
    if (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD) {

        // Set_U
        vectorizer->set_U (vrmt_entry->correspondent_VR,  vrmt_entry->offset, true);

        // Offset++
        vrmt_entry->offset++;

        // Convert to validation
        this->convert_to_validation(inst, vrmt_entry, vrmt_entry->offset - 1);


        if (inst->will_validate_offset == VECTORIZATION_SIZE - 1) {
            // Pre-vectorize next part
            //printf("Offset maior ou igual a 3: %d\n", vrmt_entry->offset);
            DV::DV_ERROR stats = vectorize(inst, &vrmt_entry, true);
            return stats;
        }
        #if DV_DEBUG == 1
            printf("Stats::SUCCESS\n");
        #endif

        return DV::SUCCESS;

    // **************************************
    // vector_map_table_t::validate (operation)
    // **************************************

    } else if ((inst->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD) &&
               (inst->opcode_operation != INSTRUCTION_OPERATION_MEM_STORE)  )
    {
        // Check if registers remain the same
        bool registers_persist = compare_registers(inst, vrmt_entry);

        if (registers_persist == false) {

            // Invalidate vrmt_entry
            this->invalidate(vrmt_entry);

            // Check for vectorial operands
            bool op_vectorial;
            op_vectorial = vectorizer->vectorial_operands(inst);

            if (op_vectorial == false) {
                #if DV_DEBUG == 1
                    printf("Stats::NEW_PARAMETERS_NOT_VECTORIZED\n");
                #endif
            	return DV::NEW_PARAMETERS_NOT_VECTORIZED;
            }

            // Create new operation with the right parameters
            vector_map_table_entry_t *new_vrmt_entry = NULL;

            DV::DV_ERROR stats = vectorize(inst, &new_vrmt_entry, false); 

            if (stats == DV::NOT_ENOUGH_VR) {
	            /* Na hora de entrar no pipeline isso acontece
                register_rename_table[inst->write_regs[0]].vectorial = false;
	            register_rename_table[inst->write_regs[0]].offset = 0;
 	            register_rename_table[inst->write_regs[0]].correspondent_vectorial_reg = -1;
                */
                #if DV_DEBUG == 1
                    printf("Stats::NOT_ENOUGH_VR\n");
	            #endif
                return DV::NOT_ENOUGH_VR;

            }

            // Convert to validation
            this->convert_to_validation(inst, new_vrmt_entry, 0);
            #if DV_DEBUG == 1
                printf("Stats::NEW_PARAMETERS_REVECTORIZING\n");
            #endif
            return DV::NEW_PARAMETERS_REVECTORIZING;

        } else {
            // Set_U
            vectorizer->set_U(vrmt_entry->correspondent_VR, vrmt_entry->offset, true);

            // Increment offset
            vrmt_entry->offset++;

            // Convert to validation
            this->convert_to_validation(inst, vrmt_entry, vrmt_entry->offset - 1);
            #if DV_DEBUG == 1
                printf("Stats::SUCCESS\n");
            #endif
            return DV::SUCCESS;
        }


    }
    #if DV_DEBUG == 1
        printf("Stats::SUCCESS\n");
    #endif
    return DV::SUCCESS;
}

void vector_map_table_t::fill_vectorial_part (opcode_package_t *inst, bool is_load, int32_t vr_id, int32_t num_part) {
    // Fill opcode_assembly
    switch(inst->opcode_operation) {
	    case INSTRUCTION_OPERATION_INT_ALU:
	    	strcpy(inst->opcode_assembly, "VPADDQ_YMMqq_YMMqq_YMMqq+R256+R256+R256");
	    	break;
	    case INSTRUCTION_OPERATION_INT_MUL:
	    	strcpy(inst->opcode_assembly, "VPMULDQ_YMMqq_YMMqq_YMMqq+R256+R256+R256");
	    	break;
	    case INSTRUCTION_OPERATION_FP_ALU:
	    	strcpy(inst->opcode_assembly, "VADDPD_YMMqq_YMMqq_YMMqq+R256+R256+R256");
	    	break;
	    case INSTRUCTION_OPERATION_MEM_LOAD:
	    	break;
	    default:
	    	printf("Erro: Vetorização de operação não definida! Operação: %d - %s\n", inst->opcode_operation, inst->opcode_assembly);
	    	exit(1);
    }

    // Fill instruction_id
    auto &inst_id = orcs_engine.instruction_set->instructions_id;
    std::string op_asm = std::string(inst->opcode_assembly);

    if (inst_id.find(op_asm) != inst_id.end()) {
        inst->instruction_id = inst_id[op_asm];
    } else {
        // Last inst_id has 0 uops, used when instruction is not represented
        inst->instruction_id = inst_id.size() - 1;
    }

    // Fill opcode_operation
    if (is_load) {
	    inst->opcode_operation = INSTRUCTION_OPERATION_MEM_LOAD;
    } else {
	    inst->opcode_operation = INSTRUCTION_OPERATION_OTHER;
    }

    // Generic inst preparation
    inst->is_vectorial_part = num_part;
    inst->VR_id = vr_id;
    inst->is_validation = false;

}

DV::DV_ERROR vector_map_table_t::vectorize (opcode_package_t * inst, vector_map_table_entry_t **vrmt_entry, bool forward) {
    #if DV_DEBUG == 1
    if (forward) {
        printf("ACTION::Pré-vectorization\n");
    } else {
        printf("ACTION::Vectorization\n");
    }
    #endif

    // Invalida qualquer vrmt_entry existente
    vector_map_table_entry_t *vrmt_entry_temp = this->find_pc(inst->opcode_address);
    if (vrmt_entry_temp)
    {
    	this->invalidate(vrmt_entry_temp);
    }
    
    // Aloca um VR (vr_id)
    int32_t vr_id = vectorizer->allocate_VR(inst->write_regs[0]);

    if (vr_id == -1) 
    {   
        #if DV_DEBUG == 1
            printf("Stats::NOT_ENOUGH_VR\n");
        #endif
        return DV::NOT_ENOUGH_VR;
    }

    // Preenche VR
    VR_state_bits_t *state_VR = &vectorizer->vr_control_bits[vr_id];

    state_VR->MRBB = vectorizer->GMRBB;

    if (forward)
    {
    	state_VR->positions[0].F = false;
    	state_VR->positions[0].R = false;
    	state_VR->positions[0].U = false;
    	state_VR->positions[0].V = false;
    } else {
    	state_VR->positions[0].F = false;
    	state_VR->positions[0].R = false;
    	state_VR->positions[0].U = true;
    	state_VR->positions[0].V = false;
    }

    for (int32_t pos = 1; pos < VECTORIZATION_SIZE; ++pos) {
      	state_VR->positions[0].F = false;
    	state_VR->positions[0].R = false;
    	state_VR->positions[0].U = false;
    	state_VR->positions[0].V = false;
    }


    // Aloca uma vrmt_entry
    *vrmt_entry = &entries[this->allocate_entry()];

    // Preenche vrmt_entry
    (*vrmt_entry)->pc = inst->opcode_address;
    (*vrmt_entry)->offset = (forward) ? 0 : 1;
    //printf("Offset definido para: %d\n", (*vrmt_entry)->offset);
    (*vrmt_entry)->value = 0;
    (*vrmt_entry)->correspondent_VR = vr_id;
    (*vrmt_entry)->is_load = (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD);
    
    table_of_loads_entry_t *tl_entry = 0x0;
    // Define operandos vetoriais ou regiões de memória.
    if (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
        vectorizer->vectorized_loads++;
    	tl_entry = TL->find_pc (inst->opcode_address);
    	int32_t stride = tl_entry->stride;
        int32_t read_start;
        if (forward) {
            read_start = inst->read_address + tl_entry->stride;
        } else {
            read_start = inst->read_address;
        }
    	if (stride >= 0) {
    		(*vrmt_entry)->source_operand_1 = read_start;
    		(*vrmt_entry)->source_operand_2 = read_start + (VECTORIZATION_SIZE - 1) * stride + inst->read_size;

    	} else {
    		(*vrmt_entry)->source_operand_1 = read_start + (VECTORIZATION_SIZE - 1) * stride;
    		(*vrmt_entry)->source_operand_2 = read_start + inst->read_size;
    	}
    } else {

        vectorizer->vectorized_ops++;
    	(*vrmt_entry)->source_operand_1 = register_rename_table[inst->read_regs[0]].correspondent_vectorial_reg;
    	(*vrmt_entry)->source_operand_2 = register_rename_table[inst->read_regs[1]].correspondent_vectorial_reg;
    }
    if (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
        for (int32_t num_part = 0; num_part < 1; ++num_part)
        //for (int32_t num_part = 0; num_part < VECTORIZATION_SIZE; ++num_part) //ONE_LOAD
        {
            // Clona a uop
            opcode_package_t new_inst = (*inst);
            int32_t idx = (forward) ? num_part + 1 : num_part;
            new_inst.read_address += tl_entry->stride*idx;

            // Preenche com dados de parte vetorial
            fill_vectorial_part(&new_inst, true, vr_id, num_part);
            new_inst.is_pre_vectorization = forward;

            // Insere no pipeline
            inst_list->push_back(new_inst);
        }

    } else {
        // Clona a uop
        opcode_package_t new_inst = (*inst);

        // Preenche com dados de parte vetorial
        fill_vectorial_part(&new_inst, false, vr_id, 0);
        new_inst.is_pre_vectorization = forward;

        // Insere no pipeline
        inst_list->push_back(new_inst);
    }
    #if DV_DEBUG == 1
        printf("Stats::SUCCESS\n");
    #endif
    return DV::SUCCESS;

}

void vector_map_table_t::list_contents() {
    for (int32_t i = 0; i < this->entries_size; ++i) {
        if (this->entries[i].pc != 0x0) {
            printf("%lu: Offset: %d -- Source_1: %lu -- Source_2: %lu --> VR: %d\n",
                    this->entries[i].pc, this->entries[i].offset,
                    this->entries[i].source_operand_1,
                    this->entries[i].source_operand_2,
                    this->entries[i].correspondent_VR);
        }
    }
}