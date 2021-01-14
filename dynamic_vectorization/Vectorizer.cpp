#include "./../simulator.hpp"


int32_t Vectorizer_t::allocate_VR(int32_t logical_register) {
    for (int32_t i = 0; i < NUM_VR; ++i) {
        if (this->vr_state[i] == -1) {
            this->vr_state[i] = logical_register;
            this->vr_control_bits[i].MRBB = this->GMRBB;
            return i;
        }
    }
    return -1;
}

DV::DV_ERROR Vectorizer_t::new_commit (uop_package_t *inst) {
    // **************************************
    // Vectorizer_t:new_commit (!store)
    // **************************************

    if (inst->uop_operation != INSTRUCTION_OPERATION_MEM_STORE) {

        if (inst->is_vectorial_part >= 0) {
            // Set R
            int32_t vr_id = inst->VR_id;
            VR_state_bits_t *state_VR = &this->vr_control_bits[vr_id];
            state_VR->positions[inst->is_vectorial_part].R = true;

        }

        if (inst->is_validation) {
            // Set V
            // Reset U
            VR_state_bits_t * state_VR = &this->vr_control_bits[inst->VR_id];

            // Find next offset to validate
            int32_t offset = 0;
            for (offset = 0; offset < VECTORIZATION_SIZE; ++offset) {
            	if (state_VR->positions[offset].U) {
            		break;
            	}
            }
            state_VR->positions[offset].U = false;
            state_VR->positions[offset].V = true;

        }

        if (inst->will_free >= 0) {
            // Set F
            VR_state_bits_t *state_VR = &this->vr_control_bits[inst->will_free];
            state_VR->positions[inst->will_free_offset].F = true;

            // Tenta liberar
            // Liberou até a última posição
            if (inst->will_free_offset == VECTORIZATION_SIZE - 1) {
	            this->free_VR (inst->will_free);
            }

        }

        if (inst->is_BB == true) {
            this->GMRBB = inst->opcode_address;
            this->GMRBB_changed();
        }

        return DV::SUCCESS;
    }

    // **************************************
    // Vectorizer_t:new_commit (!store)
    // **************************************
    if (inst->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
        // Invalida entradas com dados desse endereço
        VRMT->new_store(inst);

        // Squash pipeline
        this->squash_pipeline();

    }
    return DV::SUCCESS;
}

DV::DV_ERROR Vectorizer_t::new_inst (uop_package_t *inst) {
    if (inst->is_vectorial_part >= 0) return DV::SUCCESS;

    // **************************************
    // Vectorizer_t::new_inst (qualquer) [first]
    // **************************************

    // Descobre se já possui alguma entrada na vrmt
    vector_map_table_entry_t * vrmt_entry = VRMT->find_pc(inst->opcode_address);

    // Marca para liberar
    // setar (F)
    reorder_buffer_line_t *destiny_reg = register_alias_table[inst->write_regs[0]];

    if (vrmt_entry == NULL) {
    	if (destiny_reg->vectorial) {
    		inst->will_free = destiny_reg->correspondent_vectorial_reg;
    		inst->will_free_offset = destiny_reg->offset;
    	}

    } else {
    	// Como para duas instruções diferentes não é alocado o mesmo VR, basta ver se são iguais
    	if (vrmt_entry->correspondent_VR != destiny_reg->correspondent_vectorial_reg) {
    		inst->will_free = destiny_reg->correspondent_vectorial_reg;
    		inst->will_free_offset = destiny_reg->offset;
    	}
    }

    // **************************************
    // Vectorizer_t::new_inst (load)
    // **************************************

    if (inst->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
        // Procura pelo load na TL
        table_of_loads_entry_t *tl_entry;
        tl_entry = TL->find_pc(inst->opcode_address);

        // Adiciona PC ou valida o stride
        if (tl_entry == NULL) {
	        tl_entry = TL->add_pc (inst->opcode_address, inst->memory_address);
        } else {
	        TL->update_stride (tl_entry, inst->memory_address);
        }

        // Descobre se já está vetorizado
        vector_map_table_entry_t *vrmt_entry;
        vrmt_entry = VRMT->find_pc (inst->opcode_address);

        if (tl_entry->confidence >= 2) {
            if (vrmt_entry != NULL) {
                // Faz a validação Ou seja, aloca o próximo se der
                DV::DV_ERROR stats = VRMT->validate(inst, vrmt_entry);
                return stats;
            } else {
                // Vetoriza o load
                DV::DV_ERROR stats = VRMT->vectorize (inst, &vrmt_entry, false);

                if (stats == DV::NOT_ENOUGH_VR) {
                	return stats;
                }

                // Converte em validação
                VRMT->convert_to_validation(inst, vrmt_entry);

                return DV::SUCCESS;
            }


        } else { // vrmt_entry->confidence < 2
            // Invalida qualquer entrada existente na VRMT
            if (vrmt_entry) {
	            VRMT->invalidate(vrmt_entry);
            }

            return DV::SUCCESS;

        }

    }

    // **************************************
    // Vectorizer_t::new_inst (operation)
    // **************************************
    if ((inst->uop_operation == INSTRUCTION_OPERATION_INT_ALU) ||
       (inst->uop_operation == INSTRUCTION_OPERATION_INT_MUL) ||
       (inst->uop_operation == INSTRUCTION_OPERATION_FP_ALU))    {

        // Search for PC in VRMT
        vector_map_table_entry_t *vrmt_entry;
        vrmt_entry = VRMT->find_pc(inst->opcode_address);

        if (vrmt_entry != NULL) {
            // Valida a entrada da VRMT convertendo em uma validação
            // (O validate converte a inst em validação)
            VRMT->validate(inst, vrmt_entry);

        } else { // vrmt_entry == NULL
            // Verifica se os operandos são vetoriais
            bool op_vectorial;
            op_vectorial = this->vectorial_operands(inst);

            if (op_vectorial) {
                // Vectorize operation
                VRMT->vectorize(inst, &vrmt_entry, false);

                // Convert to validation
                VRMT->convert_to_validation(inst, vrmt_entry);
            }

        }
        return DV::SUCCESS;

    }
    return DV::SUCCESS;
}

void Vectorizer_t::set_U (int32_t vr_id, int32_t index, bool value) {
    this->vr_control_bits[vr_id].positions[index].U = value;
}


bool Vectorizer_t::vectorial_operands (uop_package_t *inst) {
    reorder_buffer_line_t *op_1 = register_alias_table[inst->read_regs[0]];
    reorder_buffer_line_t *op_2 = register_alias_table[inst->read_regs[1]];
    if (op_1->vectorial && op_2->vectorial) return true;
    return false;
}

void Vectorizer_t::enter_pipeline (uop_package_t *inst) {
    if (inst->VR_id >= 0) {
        // Find vrmt_entry
        vector_map_table_entry_t *vrmt_entry = VRMT->find_pc (inst->opcode_address);

        if (vrmt_entry == NULL) 
        {
        	printf("Vectorizer_t::enter_pipeline\n");
        	printf("Error: VRMT entry not found\n");
        	exit(1);
        }

        // Adjust RAT
        register_alias_table[inst->write_regs[0]]->vectorial = true;
        register_alias_table[inst->write_regs[0]]->correspondent_vectorial_reg = inst->VR_id;

        if (inst->is_validation) 
        {
        	register_alias_table[inst->write_regs[0]]->offset = vrmt_entry->offset;
        } else {
        	// Uma instrução acessa o que já foi validado
            // Vectorial parts de forward vão acabar deixando em -1.
        	register_alias_table[inst->write_regs[0]]->offset = vrmt_entry->offset - 1; 
        }

    } else { // inst->VR_id == -1
        // Set RAT as scalar register
        register_alias_table[inst->write_regs[0]]->vectorial = false;

    }

}


void Vectorizer_t::GMRBB_changed () {

    // Try to free VR
    for (int32_t i = 0; i < NUM_VR; ++i) {

	    if(vr_state[i] == -1) continue;

	    if (this->vr_control_bits[i].MRBB != GMRBB) 
	    {
		    bool can_free = true;

		    for (int32_t pos = 0; pos < VECTORIZATION_SIZE; ++pos) {
			    if (this->vr_control_bits[pos].positions[pos].R == false) {
				    can_free = false;
				    break;
			    }

			    if (this->vr_control_bits[pos].positions[pos].U == true) {
				    can_free = false;
				    break;
			    }

			    if (this->vr_control_bits[pos].positions[pos].V == true) {
				    if (this->vr_control_bits[pos].positions[pos].F == false) {
					    can_free = false;
					    break;
				    }
			    }

		    }

		    if (can_free) {
			    this->free_VR(i);
		    }
	    }
    }
}

void Vectorizer_t::free_VR (int32_t vr_id) {
    int32_t PR = this->vr_state[vr_id];

    // Libera registrador lógico
    if  (register_alias_table[PR]->correspondent_vectorial_reg == vr_id) {
    	register_alias_table[PR]->vectorial = false;
    	register_alias_table[PR]->offset = -1;
    	register_alias_table[PR]->correspondent_vectorial_reg = -1;
    }

    // Libera registrador vetorial
    this->vr_state[vr_id] = -1;

}


void Vectorizer_t::squash_pipeline() {
//<error>

}

Vectorizer_t::Vectorizer_t(reorder_buffer_line_t **RAT, circular_buffer_t <uop_package_t> *inst_list) 
{
    GMRBB = 0;
    // TL
    this->TL = new table_of_loads_t (TL_SIZE);

    // VR data
    this->vr_control_bits = new VR_state_bits_t [NUM_VR];
    for (int32_t i = 0; i < NUM_VR; ++i) {
        this->vr_control_bits[i].MRBB = 0;
        this->vr_control_bits[i].positions = new VR_entry_state_t [VECTORIZATION_SIZE];
    }

    this->vr_state = std::vector<int32_t>(NUM_VR, -1);

    // VRMT
    this->VRMT = new vector_map_table_t (VRMT_SIZE, 
                             RAT,
                             this, 
                             this->TL, 
                             inst_list);


    // RAT
    this->register_alias_table = RAT;

}

Vectorizer_t::~Vectorizer_t() 
{
    // TL
    delete this->TL;

    // VR data
    for (int32_t i = 0; i < NUM_VR; ++i) {
        delete[] this->vr_control_bits[i].positions;
    }
    delete[] this->vr_control_bits;

    // VRMT
    delete this->VRMT;

}