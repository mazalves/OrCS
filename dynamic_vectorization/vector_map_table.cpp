#include "./../simulator.hpp"
vector_map_table_t::vector_map_table_t  (int32_t num_entries, 
                                        register_rename_table_t *RRT,
                                        Vectorizer_t *vectorizer, 
                                        table_of_loads_t *TL, 
                                        circular_buffer_t <uop_package_t> *inst_list) {
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


bool vector_map_table_t::compare_registers (uop_package_t *inst, vector_map_table_entry_t *vrmt_entry) {
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

DV::DV_ERROR vector_map_table_t::convert_to_validation (uop_package_t *inst, vector_map_table_entry_t *vrmt_entry) {
    inst->is_vectorial_part = -1;
    inst->VR_id = vrmt_entry->correspondent_VR;
    inst->is_validation = true;

    // Set register as vectorial
    this->register_rename_table[inst->write_regs[0]].vectorial = true;
    this->register_rename_table[inst->write_regs[0]].offset = vrmt_entry->offset - 1;
    this->register_rename_table[inst->write_regs[0]].correspondent_vectorial_reg = vrmt_entry->correspondent_VR;

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

void vector_map_table_t::new_store (uop_package_t *inst) {
    for (int32_t i = 0; i < this->entries_size; ++i) {
	    if(this->entries[i].is_load) {
	    	if((entries[i].source_operand_1 <= inst->memory_address) &&
	    	  (entries[i].source_operand_2 > inst->memory_address)) {
                  this->invalidate(&this->entries[i]);
            }
        }
    }
}

DV::DV_ERROR vector_map_table_t::validate (uop_package_t *inst, vector_map_table_entry_t *vrmt_entry) {
    // **************************************
    // vector_map_table_t::validate (load)
    // **************************************
    if (inst->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) {

        // Set_U
        vectorizer->set_U (vrmt_entry->correspondent_VR,  vrmt_entry->offset, true);

        // Offset++
        vrmt_entry->offset++;

        // Convert to validation
        this->convert_to_validation(inst, vrmt_entry);

        if (vrmt_entry->offset >= 4) {
            // Pre-vectorize next part
            DV::DV_ERROR stats = vectorize(inst, &vrmt_entry, true);
            return stats;
        }

        return DV::SUCCESS;

    // **************************************
    // vector_map_table_t::validate (operation)
    // **************************************

    } else if ((inst->uop_operation != INSTRUCTION_OPERATION_MEM_LOAD) &&
               (inst->uop_operation != INSTRUCTION_OPERATION_MEM_STORE)  )
    {
        // Check if registers remain the same
        bool registers_persist = compare_registers(inst, vrmt_entry);

        if (registers_persist == false) {

            // Invalidate vrmt_entry
            this->invalidate(vrmt_entry);

            // Create new operation with the right parameters
            vector_map_table_entry_t *new_vrmt_entry = NULL;

            DV::DV_ERROR stats = vectorize(inst, &new_vrmt_entry, false); 

            if (stats == DV::NOT_ENOUGH_VR) {
	            register_rename_table[inst->write_regs[0]].vectorial = false;
	            register_rename_table[inst->write_regs[0]].offset = 0;
 	            register_rename_table[inst->write_regs[0]].correspondent_vectorial_reg = -1;
	            return DV::NOT_ENOUGH_VR;

            }

            // Convert to validation
            this->convert_to_validation(inst, new_vrmt_entry);

            return DV::NEW_PARAMETERS_REVECTORIZING;

        } else {
            // Set_U
            vectorizer->set_U(vrmt_entry->correspondent_VR, vrmt_entry->offset, true);

            // Increment offset
            vrmt_entry->offset++;

            // Convert to validation
            this->convert_to_validation(inst, vrmt_entry);

            if (vrmt_entry->offset >= 4) {

                // Pre-vectorize next part
                this->vectorize (inst, &vrmt_entry, true);

                return DV::VECTORIZE_OPERATION_FORWARD;

            }
            return DV::SUCCESS;
        }


    }
    return DV::SUCCESS;
}

void vector_map_table_t::fill_vectorial_part (uop_package_t *inst, char *signature, int32_t vr_id, int32_t num_part) {
    // inst preparation
    // Se existirem instruções vetoriais, também mudar uop_operation
    strcpy(inst->opcode_assembly, signature);
    inst->is_vectorial_part = num_part;
    inst->VR_id = vr_id;
    inst->is_validation = false;
}

DV::DV_ERROR vector_map_table_t::vectorize (uop_package_t * inst, vector_map_table_entry_t **vrmt_entry, bool forward) {


    // Aloca um VR (vr_id)
    int32_t vr_id = vectorizer->allocate_VR(inst->write_regs[0]);

    if (vr_id == -1) 
    {
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

    // Invalida qualquer vrmt_entry existente
    vector_map_table_entry_t *vrmt_entry_temp = this->find_pc(inst->opcode_address);
    if (vrmt_entry_temp)
    {
    	this->invalidate(vrmt_entry_temp);
    }

    // Aloca uma vrmt_entry
    *vrmt_entry = &entries[this->allocate_entry()];

    // Preenche vrmt_entry
    (*vrmt_entry)->pc = inst->opcode_address;
    (*vrmt_entry)->offset = (forward) ? 0 : 1;
    (*vrmt_entry)->value = 0;
    (*vrmt_entry)->correspondent_VR = vr_id;
    (*vrmt_entry)->is_load = (inst->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD);

    // Define operandos vetoriais ou regiões de memória.
    if (inst->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
    	table_of_loads_entry_t *tl_entry = TL->find_pc (inst->opcode_address);
    	int32_t stride = tl_entry->stride;

    	if (stride >= 0) {
    		(*vrmt_entry)->source_operand_1 = inst->memory_address;
    		(*vrmt_entry)->source_operand_2 = inst->memory_address + (VECTORIZATION_SIZE - 1) * stride + inst->memory_size;

    	} else {
    		(*vrmt_entry)->source_operand_1 = inst->memory_address + (VECTORIZATION_SIZE - 1) * stride;
    		(*vrmt_entry)->source_operand_2 = inst->memory_address + inst->memory_size;
    	}
    } else {
    	(*vrmt_entry)->source_operand_1 = register_rename_table[inst->read_regs[0]].correspondent_vectorial_reg;
    	(*vrmt_entry)->source_operand_2 = register_rename_table[inst->read_regs[1]].correspondent_vectorial_reg;
    }


    for (int32_t num_part = 0; num_part < VECTORIZATION_SIZE; ++num_part) {
        // Clona a uop
        uop_package_t new_inst = (*inst);

        // Preenche com dados de parte vetorial
        char signature[256];
        switch (inst->uop_operation) {
        	case INSTRUCTION_OPERATION_MEM_LOAD:
        		if (forward) {
                    strcpy(signature, "VEC_LOAD_F_PART");
        		} else {
        			strcpy(signature, "VEC_LOAD_PART");
        		}

        		break;
        	case INSTRUCTION_OPERATION_INT_ALU:
            case INSTRUCTION_OPERATION_INT_MUL:
            case INSTRUCTION_OPERATION_FP_ALU:
        		if (forward) {
                    strcpy(signature, "VEC_OP_F_PART");
        		} else {
        			strcpy(signature, "VEC_OP_PART");
        		}
        		break;

        	default:
        		printf("Error: VECTORIZATION_NOT_DEFINED\n");
        		exit(1);
        }

        fill_vectorial_part(&new_inst, signature, vr_id, num_part); 

        // Insere no pipeline
        inst_list->push_back(new_inst);

    }

    return DV::SUCCESS;

}

void vector_map_table_t::list_contents() {
    printf("VRMT Contents:\n");
    for (int32_t i = 0; i < VRMT_SIZE; ++i) {
        printf("%d: %lu\n", i, this->entries[i].pc);
    }
    
}