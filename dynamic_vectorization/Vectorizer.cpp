#include "./../simulator.hpp"


// Constants defined by configuration
int32_t VECTORIZATION_SIZE; // 4
int32_t NUM_VR; // 32
int32_t VRMT_SIZE; // 32 					// Acho que precisa ser maior ou igual ao VR. Deve ser 1 <-> 1
int32_t VRMT_ASSOCIATIVITY;
int32_t TL_SIZE; // 1
int32_t TL_ASSOCIATIVITY;
int32_t FU_VALIDATION_SIZE; // 100
int32_t FU_VALIDATION_WAIT_NEXT; // 1

int32_t FETCH_BUFFER_VECTORIZED; // 100 	// Tamanho do buffer auxiliar que passa instruções
                                    	 	// vetorizadas para o decode
int32_t DECODE_BUFFER_VECTORIZED; // 100  // Tamanho do buffer auxiliar que passa instruções
                                    	   // vetorizadas para o rename

uint32_t ROB_VECTORIAL_SIZE; // 100 	// Espaço adicional no ROB para instruções vetoriais
		                               			// Supostamente elas não entram no ROB, mas com esse espaço extra
		                               			// dedicado fica mais fácil gerenciar
int32_t VECTORIZATION_ENABLED;

std::map<std::string, std::string> vec_correspondent;

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

    if (inst->is_validation) {
        ERROR_ASSERT_PRINTF((inst->VR_id >= 0) && (inst->VR_id < NUM_VR) && (inst->will_validate_offset < VECTORIZATION_SIZE) && (inst->will_validate_offset >=0),
        "Erro, validação commitando registrador %d[%d]\n", inst->VR_id, inst->will_validate_offset);
        this->sub_V(inst->VR_id, inst->will_validate_offset, 1);
        this->sub_U(inst->VR_id, inst->will_validate_offset, 1);

    } else if (inst->is_vectorial_part >= 0) {
        ERROR_ASSERT_PRINTF((inst->VR_id >= 0) && (inst->VR_id < NUM_VR) && (inst->is_vectorial_part < VECTORIZATION_SIZE) && (inst->is_vectorial_part >=0),
        "Erro, parte vetorial commitando registrador %d[%d]\n", inst->VR_id, inst->is_vectorial_part);
        for (int32_t i=0; i < VECTORIZATION_SIZE; ++i) {
            this->sub_R(inst->VR_id, i, 1);
        }
    }

    if (inst->will_free >= 0) {
        ERROR_ASSERT_PRINTF((inst->will_free < NUM_VR) && (inst->will_free_offset < VECTORIZATION_SIZE) && (inst->will_free_offset >=0),
        "Erro, free de registrador %d[%d]\n", inst->will_free, inst->will_free_offset);
        this->sub_F(inst->will_free, inst->will_free_offset, 1);

        //=======================
        // Free if possible
        //=======================
        if (inst->will_free_offset == VECTORIZATION_SIZE-1)
        {
            VR_entry_state_t *vr_state = &this->vr_control_bits[inst->will_free].positions[VECTORIZATION_SIZE-1];
            if (vr_state->free && vr_state->F == 0) {
                this->free_VR(inst->will_free);
            }
        }
    
    }

    if (inst->is_BB == true) {
        this->GMRBB = inst->BB_addr;
        this->GMRBB_changed();
    }

        
    // **************************************
    // Vectorizer_t:new_commit (store)
    // **************************************
    if (inst->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) {
        // Resume pipeline
        if (inst->opcode_address == *store_squashing) {
	        this->resume_pipeline();
	        *store_squashing = 0;
        }

    }
    return DV::SUCCESS;
}

DV::DV_ERROR Vectorizer_t::new_inst (opcode_package_t *inst) {

    if (inst->is_vectorial_part >= 0) return DV::SUCCESS;
    if (inst->is_read && inst->is_read2) return DV::SUCCESS;    

    // **************************************
    // Vectorizer_t::new_inst (load)
    // **************************************

    if (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_LOAD) {
        // Procura pelo load na TL
        table_of_loads_entry_t *tl_entry;
        tl_entry = TL->find_pc(inst->opcode_address);

        // Adiciona PC ou valida o stride
        if (tl_entry == NULL) {
	        tl_entry = TL->add_pc (inst->opcode_address, inst->read_address);
        } else {
	        TL->update_stride (tl_entry, inst->read_address);
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
                VRMT->convert_to_validation(inst, vrmt_entry, 0);

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
    if ((inst->opcode_operation == INSTRUCTION_OPERATION_INT_ALU) ||
       (inst->opcode_operation == INSTRUCTION_OPERATION_INT_MUL) ||
       (inst->opcode_operation == INSTRUCTION_OPERATION_FP_ALU))    {
           //printf("%s - %lu\n", inst->opcode_assembly, vec_correspondent.count(std::string(inst->opcode_assembly)));
            if (vec_correspondent.count(std::string(inst->opcode_assembly)) == 1)
            {
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
                    DV::DV_ERROR stats = VRMT->vectorize(inst, &vrmt_entry, false);

                    // Convert to validation
                    if (stats == DV::SUCCESS) { 
                        VRMT->convert_to_validation(inst, vrmt_entry, 0);
                    }
                }

            }
            return DV::SUCCESS;

        }
    }

    // **************************************
    // Vectorizer_t::new_inst (store)
    // **************************************
    if (inst->opcode_operation == INSTRUCTION_OPERATION_MEM_STORE) {
        
        // Invalida entradas com dados desse endereço
        bool found = VRMT->new_store(inst);

        // Squash pipeline
        if (found) {
	        *this->store_squashing = inst->opcode_address;
	        this->squash_pipeline();
        }

    }

    return DV::SUCCESS;
}



bool Vectorizer_t::vectorial_operands (opcode_package_t *inst) {
    if ((inst->read_regs[0] == -1) || (inst->read_regs[1] == -1)) {
        return false;
    }
    register_rename_table_t *op_1 = &register_rename_table[inst->read_regs[0]];
    register_rename_table_t *op_2 = &register_rename_table[inst->read_regs[1]];
    if (op_1->vectorial && op_2->vectorial) 
    {
        return true;
    }

    return false;
}

DV::DV_ERROR Vectorizer_t::enter_pipeline (opcode_package_t *inst) {
    /*
    printf("$$$$$$$$$$$$$$$$$$$\n");
    printf("enter_pipeline\n");
    if(inst->is_validation) {
        printf(" --> Validação\n");
    } else if(inst->is_vectorial_part >= 0) {
        printf(" --> Parte %d\n", inst->is_vectorial_part);

    }
    printf("$$$$$$$$$$$$$$$$$$$\n");
    */
    // Descobre se já possui alguma entrada na vrmt
    vector_map_table_entry_t * vrmt_entry = NULL;
    if (inst->is_vectorial_part >= 0) {
        vrmt_entry = VRMT->find_pc(inst->opcode_address);
    }

    // Marca para liberar
    // setar (F)
    if (inst->write_regs[0] == -1) return DV::NOT_WRITING;
    if (inst->write_regs[0] >= MAX_REGISTER_NUMBER) return DV::REGISTER_GREATER_THAN_MAX;

    register_rename_table_t *destiny_reg = &register_rename_table[inst->write_regs[0]];


    // A validação vai liberar (ela passa antes no pipeline)
   // ONE_LOAD
    if (destiny_reg->vectorial && inst->is_vectorial_part < 0) {
    	inst->will_free = destiny_reg->correspondent_vectorial_reg;
    	inst->will_free_offset = destiny_reg->offset;
    }
    //printf("Got to free: %d[%d]\n", inst->will_free, inst->will_free_offset);


    if (inst->VR_id >= 0 && inst->is_pre_vectorization == false) {
        // Check for vrmt_entry

        if (inst->is_vectorial_part >= 0 && vrmt_entry == NULL)
        {
        	printf("Vectorizer_t::enter_pipeline\n");
        	printf("Error: VRMT entry (%lu) not found\n", inst->opcode_address);
            VRMT->list_contents();
        	exit(1);
        }

        // Adjust RAT
        register_rename_table[inst->write_regs[0]].vectorial = true;
        register_rename_table[inst->write_regs[0]].correspondent_vectorial_reg = inst->VR_id;

        // Uma instrução acessa o que já foi validado
        // Vectorial parts de forward vão acabar deixando em -1.
        if (inst->is_validation) {
            register_rename_table[inst->write_regs[0]].offset = inst->will_validate_offset;
        } else if (vrmt_entry->offset == 0) { // Pré-vetorização
            register_rename_table[inst->write_regs[0]].offset = 3;
            //printf("Pre");
        } else {
            register_rename_table[inst->write_regs[0]].offset = vrmt_entry->offset - 1;
        }

        //printf("Vectorization: %d -- VR: %d -- Offset: %d\n", inst->write_regs[0], inst->VR_id,
        //                                    register_rename_table[inst->write_regs[0]].offset);


    } else if (inst->VR_id == -1) { // 
        // Set RAT as scalar register
        if ((inst->write_regs[0] != -1) && (inst->write_regs[0] < MAX_REGISTER_NUMBER)) {
            register_rename_table[inst->write_regs[0]].vectorial = false;
            //printf("Scalarization: %d\n", inst->write_regs[0]);

        }

    }
    if (inst->is_validation){
        // Remove registers dependency
        inst->read_regs[0] = -1;
    }

    // Register on VR
    // Vectorial parts are added when created
    if (inst->VR_id >= 0 && inst->is_vectorial_part < 0) {
		this->vr_control_bits[inst->VR_id].associated_not_decoded++;
	}

    //printf("%linstu libera %d [%d]\n", inst->opcode_address, inst->will_free, inst->will_free_offset);
    return DV::SUCCESS;
}


void Vectorizer_t::GMRBB_changed () {

    // Try to free VR
    for (int32_t i = 0; i < NUM_VR; ++i) {

	    if(vr_state[i] == -1) continue;

	    if (this->vr_control_bits[i].MRBB != GMRBB) 
	    {
		    bool can_free = true;
            
            if (this->vr_control_bits[i].associated_not_decoded == 0) {
		        for (int32_t pos = 0; pos < VECTORIZATION_SIZE; ++pos) {
                    // Vectorial part on pipeline
			        if (this->vr_control_bits[i].positions[pos].R != 0 || this->vr_control_bits[i].positions[pos].executed == false) {
			    	    can_free = false;
			    	    break;
			        }
                    // Validation on pipeline
			        if (this->vr_control_bits[i].positions[pos].U != 0) {
                        can_free = false;
			    	    break;
			        }

                    // Validated and not free
			        if (this->vr_control_bits[i].positions[pos].V == 0 && this->vr_control_bits[i].positions[pos].sent) {
			    	    if (this->vr_control_bits[i].positions[pos].F != 0 || this->vr_control_bits[i].positions[pos].free == false) {
			    		    can_free = false;
			    		    break;
			    	    }
			        }

		        } 
            } else {
                can_free = false;
            }

		    if (can_free) {
			    this->free_VR(i);
		    }
	    }
    }
}

void Vectorizer_t::free_VR (int32_t vr_id) {

    // Check if have dependencies
    if (this->vr_control_bits[vr_id].associated_not_decoded != 0) {
        return;
    }

    // Check if it was executed
    for (int32_t i=0; i < VECTORIZATION_SIZE; ++i) {
        if (this->vr_control_bits[vr_id].positions[i].executed != 0 || this->vr_control_bits[vr_id].positions[i].executed == false) {
            return;
        }
    }

    // Limpa o registrador
    VR_state_bits_t* state = &this->vr_control_bits[vr_id];
    for (int32_t i=0; i < VECTORIZATION_SIZE; ++i) {
        state->positions[i].sent = false;
        state->positions[i].executed = false;
        state->positions[i].free = false;

        state->positions[i].V = 0;
        state->positions[i].R = 0;
        state->positions[i].U = 0;
        state->positions[i].F = 0;

    }

    // Impede re-liberação pelo GMRBB
    this->vr_control_bits[vr_id].MRBB = 0;

    // Libera para próximas instruções
    this->vr_control_bits[vr_id].associated_not_decoded = 0;

    // Remove entrada associada da VRMT
    if (this->vr_control_bits[vr_id].associated_entry != NULL &&
        this->vr_control_bits[vr_id].associated_entry->correspondent_VR == vr_id) {
        this->vr_control_bits[vr_id].associated_entry->pc = 0x0;
    }
    this->vr_control_bits[vr_id].associated_entry = NULL;

    // Desvincula ao registrador lógico
    int32_t PR = this->vr_state[vr_id];



    // Já está liberado
    if (PR == -1) {
        return;
    }

    // Libera registrador lógico
    if  (register_rename_table[PR].correspondent_vectorial_reg == vr_id) {
    	register_rename_table[PR].vectorial = false;
    	register_rename_table[PR].offset = -1;
    	register_rename_table[PR].correspondent_vectorial_reg = -1;
    }

    // Libera registrador vetorial
    this->vr_state[vr_id] = -1;

}


void Vectorizer_t::squash_pipeline() {
    *this->pipeline_squashed = true;

}

void Vectorizer_t::resume_pipeline() {
    *this->pipeline_squashed = false;
}

Vectorizer_t::Vectorizer_t(circular_buffer_t <opcode_package_t> *inst_list,
                           bool *pipeline_squashed, uint64_t *store_squashing)
{
    GMRBB = 0;

    // Squash do pipeline
    this->pipeline_squashed = pipeline_squashed;
    this->store_squashing = store_squashing;

    // TL
    this->TL = new table_of_loads_t (TL_SIZE, TL_ASSOCIATIVITY);

    // RRT
    this->register_rename_table = new register_rename_table_t[MAX_REGISTER_NUMBER];

    // VR data
    this->vr_control_bits = new VR_state_bits_t [NUM_VR];
    for (int32_t i = 0; i < NUM_VR; ++i) {
        this->vr_control_bits[i].MRBB = 0;
        this->vr_control_bits[i].positions = new VR_entry_state_t [VECTORIZATION_SIZE];
    }

    this->vr_state = std::vector<int32_t>(NUM_VR, -1);

    // VRMT
    this->VRMT = new vector_map_table_t (VRMT_SIZE,
                             VRMT_ASSOCIATIVITY,
                             this->register_rename_table,
                             this, 
                             this->TL, 
                             inst_list);

    // Statistics
    vectorized_loads = 0;
    vectorized_ops = 0;
    

}

Vectorizer_t::~Vectorizer_t() 
{
    // Print vectors state:
    printf("Vectors: ");
    for (int32_t i=0; i<NUM_VR; ++i) {
        printf("%d ", this->vr_state[i]);
    }
    printf("VR status:");
    for (int32_t i=0; i<NUM_VR; ++i) {
        printf("%d (Linked entry: %lu): ", i, (uint64_t) ((void *)this->vr_control_bits[i].associated_entry));
        for (int32_t j=0; j<VECTORIZATION_SIZE; ++j) {
            printf("[%d %d %d %d S: %s E: %s F: %s] ", this->vr_control_bits[i].positions[j].V
                                   , this->vr_control_bits[i].positions[j].R
                                   , this->vr_control_bits[i].positions[j].U
                                   , this->vr_control_bits[i].positions[j].F
                                   , this->vr_control_bits[i].positions[j].sent ? "true" : "false"
                                   , this->vr_control_bits[i].positions[j].executed ? "true" : "false"
                                   , this->vr_control_bits[i].positions[j].free ? "true" : "false");
        }
        printf("{%lu}\n", this->vr_control_bits[i].MRBB);
        printf(" -> Associated not decoded: %u\n", this->vr_control_bits[i].associated_not_decoded);
    }


    // TL
    delete this->TL;

    // VR data
    delete[] this->vr_control_bits;

    // VRMT
    delete this->VRMT;

}

void Vectorizer_t::statistics() {
    bool close = false;
	FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
		close=true;
	}
	if (output != NULL){
			utils_t::largestSeparator(output);
            fprintf(output,"Vectorizer\n");
            fprintf(output, "Vectorized loads: %lu\n", this->vectorized_loads);
            fprintf(output, "Vectorized operations: %lu\n", this->vectorized_ops);
            utils_t::largestSeparator(output);
        }
        if(close) fclose(output);

}

void Vectorizer_t::debug() {
    printf(" ********* Table of loads (TL) ********* \n");
    TL->list_contents();
    printf(" ********* Vector Register Map Table (VRMT) ********* \n");
    VRMT->list_contents();
    printf("\n");
    printf("Registers: ");
    for (int32_t i = 0; i < NUM_VR; ++i) {
        printf("%d ", this->vr_state[i]);
    }
    printf("\n");
}