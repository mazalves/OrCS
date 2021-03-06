#include "./../simulator.hpp"
extern std::map<std::string, std::string> vec_correspondent;

vector_map_table_t::vector_map_table_t (int32_t num_entries,
                                        int32_t associativity,
                                        register_rename_table_t *RRT,
                                        Vectorizer_t *vectorizer, 
                                        table_of_loads_t *TL, 
                                        circular_buffer_t <opcode_package_t> *inst_list) {
    // Inicializações
    this->entries.allocate(num_entries, associativity, 0);
    this->TL = TL;
    this->vectorizer = vectorizer;
    this->register_rename_table = RRT;
    this->inst_list = inst_list;

    // Instruções e correspondentes vetoriais
    vec_correspondent[std::string("ADD")] = std::string("VPADDQ_YMMqq_YMMqq_YMMqq+R256+R256+R256");
    vec_correspondent[std::string("SUB")] = std::string("VPSUBB_YMMqq_YMMqq_YMMqq+R256+R256+R256");
    vec_correspondent[std::string("SHL")] = std::string("VPSLLW_XMMdq_XMMdq_IMMb+R128+R128+I8");
    vec_correspondent[std::string("SHR")] = std::string("VPSRLW_YMMqq_YMMqq_IMMb+R256+R256+I8");
    vec_correspondent[std::string("DIV")] = std::string("VDIVPD_YMMqq_YMMqq_YMMqq+R256+R256+R256");
    vec_correspondent[std::string("MUL")] = std::string("VMULPD_YMMqq_YMMqq_YMMqq+R256+R256+R256");
    vec_correspondent[std::string("NEG")] = std::string("PSIGNB_XMMdq_XMMdq+R128+R128");
    vec_correspondent[std::string("AND")] = std::string("VANDPD_YMMqq_YMMqq_YMMqq+R256+R256+R256");
    vec_correspondent[std::string("OR")] = std::string("VORPD_YMMqq_YMMqq_YMMqq+R256+R256+R256");

}
        
vector_map_table_t::~vector_map_table_t () {
    //list_contents();
}

vector_map_table_entry_t* vector_map_table_t::allocate_entry (uint64_t pc) {
    //=========================
    // Find oldest entry
    //=========================
    // Aloca
    uint32_t oldest_last_use = 0;
    uint32_t oldest_id = 0;

    vector_map_table_entry_t *possible_entries = this->entries.map_set(pc);
    oldest_last_use = possible_entries[0].last_use;
    oldest_id = 0;

    for (uint32_t i=1; i < this->entries.get_associativity(); ++i) {
        if (oldest_last_use > possible_entries[i].last_use) {
            oldest_last_use = possible_entries[i].last_use;
            oldest_id = i;
        }
    }

    possible_entries[oldest_id].last_use = orcs_engine.get_global_cycle();
    return &possible_entries[oldest_id];
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
    vector_map_table_entry_t* possible_entries = this->entries.map_set(pc);

    for (uint32_t i=0; i < this->entries.get_associativity(); ++i) {
        if(possible_entries[i].pc == pc)
            return &possible_entries[i];
    }
    return NULL;
}

void vector_map_table_t::invalidate (vector_map_table_entry_t *vrmt_entry) {
    vrmt_entry->pc = 0;
    vectorizer->vr_control_bits[vrmt_entry->correspondent_VR].associated_entry = 0x0;
}

bool vector_map_table_t::new_store (opcode_package_t *inst) {
    bool r = false;
    for (uint32_t i = 0; i < this->entries.get_num_sets(); ++i) {
        for (uint32_t j = 0; j < this->entries.get_associativity(); ++j) {
	        if(this->entries[i][j].is_load) {
	        	if((entries[i][j].source_operand_1 <= inst->write_address) &&
	        	  (entries[i][j].source_operand_2 > inst->write_address))
                {
                    this->invalidate(&this->entries[i][j]);
		            r = true;
                }
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

        // Set that was sent to pipeline
        // // Is made on decode
        //vectorizer->set_sent(vrmt_entry->correspondent_VR, vrmt_entry->offset, true);
        //vectorizer->set_U (vrmt_entry->correspondent_VR,  vrmt_entry->offset, true);

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
            // Set_sent
            // // Is made on decode
            //vectorizer->set_sent(vrmt_entry->correspondent_VR, vrmt_entry->offset, true);
            //vectorizer->set_U(vrmt_entry->correspondent_VR, vrmt_entry->offset, true);

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
    /*
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
    */
    if (inst->opcode_operation != INSTRUCTION_OPERATION_MEM_LOAD){
     strcpy(inst->opcode_assembly, vec_correspondent[std::string(inst->opcode_assembly)].c_str());
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
    *vrmt_entry = this->allocate_entry(inst->opcode_address);

    // Preenche vrmt_entry
    (*vrmt_entry)->pc = inst->opcode_address;
    (*vrmt_entry)->offset = (forward) ? 0 : 1;
    //printf("Offset definido para: %d\n", (*vrmt_entry)->offset);
    (*vrmt_entry)->value = 0;
    (*vrmt_entry)->correspondent_VR = vr_id;
    (*vrmt_entry)->is_load = (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD);
    
    // Associa ao VR
    vectorizer->vr_control_bits[vr_id].associated_entry = (*vrmt_entry);


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
        //for (int32_t num_part = 0; num_part < 1; ++num_part) //ONE_LOAD
        uint64_t last_read = 0;
        for (int32_t num_part = 0; num_part < VECTORIZATION_SIZE; ++num_part)
        {
            // Clona a uop
            opcode_package_t new_inst = (*inst);
            int32_t idx = (forward) ? num_part + 1 : num_part;
            new_inst.read_address += tl_entry->stride*idx;
            if (!this->same_block(new_inst.read_address, last_read)) {
                // Preenche com dados de parte vetorial
                fill_vectorial_part(&new_inst, true, vr_id, num_part);
                new_inst.is_pre_vectorization = forward;

                // Insere no pipeline
                inst_list->push_back(new_inst);

                // Marca para vetor esperar ela
                state_VR->associated_not_decoded += 1;
            }
            last_read = new_inst.read_address;
        }

    } else {
        // Clona a uop
        opcode_package_t new_inst = (*inst);

        // Preenche com dados de parte vetorial
        fill_vectorial_part(&new_inst, false, vr_id, 0);
        new_inst.is_pre_vectorization = forward;

        // Insere no pipeline
        inst_list->push_back(new_inst);

        // Marca para vetor esperar ela
        state_VR->associated_not_decoded += 1;
    }
    #if DV_DEBUG == 1
        printf("Stats::SUCCESS\n");
    #endif
    return DV::SUCCESS;

}

void vector_map_table_t::list_contents() {
    for (uint32_t i=0; i < this->entries.get_num_sets(); ++i) {
        printf("#########\n");
        printf("Set %u\n", i);
        printf("#########\n");
        for (uint32_t j=0; j < this->entries.get_associativity(); ++j) {
            if (this->entries[i][j].pc != 0x0) {
                printf("%lu: Offset: %d -- Source_1: %lu -- Source_2: %lu --> VR: %d\n",
                        this->entries[i][j].pc, this->entries[i][j].offset,
                        this->entries[i][j].source_operand_1,
                        this->entries[i][j].source_operand_2,
                        this->entries[i][j].correspondent_VR);
            }
        }
        printf("########################\n");
    }
    printf("All content: \n");
    for (uint32_t i=0; i < this->entries.get_num_sets(); ++i) {
        printf("Set %u: ", i);
        for (uint32_t j=0; j < this->entries.get_associativity(); ++j) {
                printf("%lu[%lu] ",
                        this->entries[i][j].pc, this->entries[i][j].last_use);
        }
        printf("\n");
        printf("########################\n");
    }
}