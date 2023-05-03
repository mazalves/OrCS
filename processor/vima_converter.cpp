#include "./../simulator.hpp"

 vima_converter_t::vima_converter_t(){
    this->iteration = 0;
    this->state_machine = 0;
    this->vima_instructions_buffer.allocate(VIMA_INSTRUCTIONS_BUFFER);
    this->placeholders_buffer.allocate(VIMA_INSTRUCTIONS_BUFFER);


    // Write register control
    for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;

    // Conversions data
    this->next_unique_conversion_id = 1;
    this->current_conversion = NULL;
    this->current_conversions.allocate(CURRENT_CONVERSIONS_SIZE); // Greater than any ROB could support
    this->conversions_blacklist.allocate(CONVERSION_BLACKLIST_SIZE);

    // Prefetch
    this->contiguous_conversions = 0;
    this->prefetcher = new vima_prefetcher_t();

    // Statistics
    this->vima_instructions_launched = 0;
    this->placeholder_instructions_launched = 0;

    this->conversion_failed = 0;
    this->conversion_successful = 0;
    this->prefetched_vima_used = 0;
    this->prefetch_failed = 0;

    this->instructions_intercepted = 0;
    this->instructions_intercepted_until_commit = 0;
    this->instructions_reexecuted = 0;
    this->original_program_instructions = 0;

    this->conversion_entry_allocated = 0;
    this->not_enough_conversion_entries = 0;


    this->AGU_result_from_wrong_conversion = 0;
    this->AGU_result_from_current_conversion = 0;


    this->time_waiting_for_infos = 0;
    this->time_waiting_for_infos_start = 0;
    this->time_waiting_for_infos_stop = 0;

    this->VIMA_before_CPU = 0;
    this->CPU_before_VIMA = 0;

    this->blocked_until_conversion_complete = 0;
    this->unblocked_after_conversion_complete = 0;

    this->conversions_blacklisted = 0;
    this->avoided_by_the_blacklist = 0;

    this->flushed_instructions = 0;



    // *******************************
    // Create a new conversion manager
    // *****************************
    this->start_new_conversion();

}
        
void vima_converter_t::initialize(uint32_t mem_operation_latency, uint32_t mem_operation_wait_next, functional_unit_t *mem_operation_fu, uint32_t VIMA_SIZE, 
                                  uint32_t PREFETCH_BUFFER_SIZE, uint32_t CONTIGUOUS_CONVERSIONS_TO_PREFETCH, uint32_t PREFETCH_SIZE) {
            this->mem_operation_latency = mem_operation_latency;
            this->mem_operation_wait_next = mem_operation_wait_next;
            this->mem_operation_fu = mem_operation_fu;
            this->VIMA_SIZE = VIMA_SIZE;
            this->CONTIGUOUS_CONVERSIONS_TO_PREFETCH = CONTIGUOUS_CONVERSIONS_TO_PREFETCH;
            this->prefetcher->initialize(PREFETCH_BUFFER_SIZE, CONTIGUOUS_CONVERSIONS_TO_PREFETCH, PREFETCH_SIZE);

}

// TODO -> Definir mais especificamente
instruction_operation_t vima_converter_t::define_vima_operation(conversion_status_t *conversion_data) {
    // Dados retirados de:
    // instrinsics_extension.cpp:408 (verificar em vima_defines.h a qual lista pertence)
    switch(conversion_data->is_mov) {
        case true:
            return INSTRUCTION_OPERATION_VIMA_INT_ALU; //_vim64_icpyu
            break;
        case false:
            // Só definido pra soma ainda
            return INSTRUCTION_OPERATION_VIMA_INT_ALU; // _vim64_iadds
            break;
    }
    return INSTRUCTION_OPERATION_VIMA_FP_ALU;
}


uop_package_t vima_converter_t::create_placeholder_for_conversion(conversion_status_t *conversion_data) {
    // ******************************************
    // Cria um placeholder para a conversão
    // ******************************************
    opcode_package_t base_opcode;
    uop_package_t base_uop;
    base_opcode.package_clean();
    base_uop.package_clean();

    base_uop.opcode_to_uop(0, INSTRUCTION_OPERATION_NOP,
								0, 0, NULL,
								base_opcode, 0, false);

#if VIMA_CONVERSION_DEBUG
        printf("Generating Placeholder instruction with Conversion ID: %lu\n", conversion_data->unique_conversion_id);
#endif

		base_uop.is_hive = false;
		base_uop.hive_read1 = -1;
		base_uop.hive_read2 = -1;
		base_uop.hive_write = -1;

		base_uop.updatePackageWait(0);
		base_uop.born_cycle = orcs_engine.get_global_cycle();

		base_uop.unique_conversion_id = conversion_data->unique_conversion_id;
        base_uop.is_placeholder = true;

        return base_uop;

}

void vima_converter_t::generate_placeholder_instruction(conversion_status_t *conversion_data) {
    /* Conversion Placeholder */
    assert (!this->placeholders_buffer.is_full());
    this->placeholders_buffer.push_back(this->create_placeholder_for_conversion(conversion_data));

    this->placeholder_instructions_launched++;

#ifdef DEBUG_CONVERSION
						printf(" => Placeholder inserted on placeholders_buffer\n");
#endif
}

void vima_converter_t::generate_VIMA_instruction(conversion_status_t *conversion_data) {
        
        // ******************************************
        // Cria uma instrução para enviar para a VIMA
        // ******************************************

        opcode_package_t base_opcode;
        uop_package_t base_uop;
        base_opcode.package_clean();
        base_uop.package_clean();



		base_uop.opcode_to_uop(0, this->define_vima_operation(conversion_data),
								this->mem_operation_latency, this->mem_operation_wait_next, this->mem_operation_fu,
								base_opcode, 0, false);
		base_uop.add_memory_operation(0, 1);
#if VIMA_CONVERSION_DEBUG
        printf("Generating VIMA instruction with Conversion ID: %lu\n", conversion_data->unique_conversion_id);
#endif
		base_uop.unique_conversion_id = conversion_data->unique_conversion_id;
		base_uop.is_hive = false;
		base_uop.hive_read1 = -1;
		base_uop.hive_read2 = -1;
		base_uop.hive_write = -1;

		base_uop.is_vima = true;
        /* Começa a partir da que usou para identificar os addrs iniciais, por isso sem o + stride */
		base_uop.read_address = conversion_data->base_mem_addr[0];
		base_uop.read2_address = conversion_data->base_mem_addr[1];
		base_uop.write_address = conversion_data->base_mem_addr[3];
        assert (conversion_data->base_mem_addr[0] != 0x0 && (conversion_data->base_mem_addr[1] != 0x0 || conversion_data->is_mov) && conversion_data->base_mem_addr[3] != 0x0);

		base_uop.updatePackageWait(0);
		base_uop.born_cycle = orcs_engine.get_global_cycle();

        assert (!this->vima_instructions_buffer.is_full());
        this->vima_instructions_buffer.push_back(base_uop);

        this->vima_instructions_launched++;

#if VIMA_CONVERSION_DEBUG
        printf("Instrução VIMA inserida!\n");
#endif
}

void vima_converter_t::check_conversions() {
    for (uint32_t i=0; i<this->current_conversions.get_size(); ++i) {
        if( (this->current_conversions[i].vima_sent == true) && // Ao menos a conversão foi lançada
            (this->current_conversions[i].CPU_requirements_meet == false) && // Ela nunca completou os requisitos na CPU
            (this->current_conversions[i].infos_remaining == 0) && // Já obteve as informações das iterações iniciais
            (this->current_conversions[i].mem_addr_confirmations_remaining == 0) && // Já conseguiu calcular os endereços de acesso à memória (não verificamos isso ainda...)
            (this->current_conversions[i].wait_reg_deps_number == 0) && // As dependências foram resolvidas
            (orcs_engine.get_global_cycle() > this->current_conversions[i].deps_readyAt)) { // Já deu tempo da última dependência se resolver
                // Permitir o commit pela CPU
                //printf("Conversão %lu completada!\n", this->current_conversions[i].unique_conversion_id);
                this->current_conversions[i].CPU_requirements_meet = true;
                orcs_engine.vima_controller->confirm_transaction(1 /* Success */, this->current_conversions[i].unique_conversion_id);
                if (this->current_conversions[i].VIMA_requirements_meet && 
                    this->current_conversions[i].VIMA_requirements_meet_readyAt <= orcs_engine.get_global_cycle())
                    {
                        this->VIMA_before_CPU++;
                    } else {
                        this->CPU_before_VIMA++;
                    }
                
            }
    }
    /*if (this->current_conversions.get_size() > 0) {
        printf("Conversão mais antiga:\n");
        printf("    CPU_requirements_meet: %s\n    infos_remaining: %u\n    mem_addr_confirmations_remaining: %ld\n    wait_reg_deps_number: %d\n    deps_readyAt: %lu/%lu\n",
            this->current_conversions[0].CPU_requirements_meet ? "true" : "false",
            this->current_conversions[0].infos_remaining,
            this->current_conversions[0].mem_addr_confirmations_remaining,
            this->current_conversions[0].wait_reg_deps_number,
            this->current_conversions[0].deps_readyAt,
            orcs_engine.get_global_cycle());
    }*/
}

// Conversion address
bool vima_converter_t::AGU_result(uop_package_t *uop) {
    
    // Find correspondent conversion
    int32_t conversion_index = -1;
    for (uint32_t i=0; i<this->current_conversions.get_size(); ++i) {
        if (this->current_conversions[i].unique_conversion_id == uop->unique_conversion_id) {
            if (this->current_conversions[i].entry_can_be_removed == false) {
                conversion_index = i;
                break;
            }
        }
    }

    if (conversion_index >= 0) {
        // ****************************************
        // Data to start first conversion in a loop
        // ****************************************
        if (uop->linked_to_iteration < 0) {
            assert (this->current_conversions[conversion_index].infos_remaining);
            this->mem_addr[uop->linked_to_converter] = uop->memory_address[0]; // Current conversion addresses
            --this->current_conversions[conversion_index].infos_remaining;
            ++this->AGU_result_from_current_conversion;
#if VIMA_CONVERSION_DEBUG == 1
            printf("//*********************************\n");
            printf(" %lu - Received address from iteration %ld [%u Remaining]\n", uop->opcode_address, uop->linked_to_iteration, this->current_conversions[conversion_index].infos_remaining);
            printf("//*********************************\n");
#endif
        }


        if ((this->current_conversions[conversion_index].infos_remaining == 0) && (!this->current_conversions[conversion_index].vima_sent)) {
#if VIMA_CONVERSION_DEBUG
            printf("Required address data acquired\n");
#endif


            // ****************
            // Calculate stride
            // ****************
            int64_t stride;
            for (int32_t i=0; i<4; ++i) {
                if (i == 1) continue; // Operation
                if (this->current_conversions[conversion_index].is_mov && i == 2) continue;// Ld 2

                // Loads/store
                stride = this->mem_addr[7-i] - this->mem_addr[3-i];
                if (stride != uop->memory_size[0]) {
#if VIMA_CONVERSION_DEBUG
                    printf("Stride %ld ([%d] %lu - [%d] %lu) != %u ==>> Invalidating conversion\n", stride, 7-i, this->mem_addr[7-i], 3-i, this->mem_addr[3-i], uop->memory_size[0]);
#endif
                    this->invalidation_reason["FIRST TWO ITERATIONS NOT CONTIGUOUS"]++;
                    this->stop_conversion(&this->current_conversions[conversion_index]);
                    return false;
                }
            }

            // *****************************
            // Required offset for alignment
            // *****************************
            if(this->get_index_for_alignment(&this->current_conversions[conversion_index], uop->memory_size[0], this->mem_addr[3]) == false) {
#if VIMA_CONVERSION_DEBUG
                    printf("Could not align! Store address %ld Current iteration %u\n", this->mem_addr[3], this->iteration);
#endif
                this->invalidation_reason["NO ITERATION WITH STORE ALIGNED WITH VIMA CACHE LINES"]++;
                this->stop_conversion(&this->current_conversions[conversion_index]);
                return false;
            }
#if VIMA_CONVERSION_DEBUG == 1
            printf("//****************************\n");
            printf("Generating VIMA instruction...\n");
            printf("//****************************\n");
#endif
            // Set base memory addresses
            this->current_conversions[conversion_index].base_mem_addr[0] = this->mem_addr[0] + (uop->memory_size[0] * this->current_conversions[conversion_index].conversion_beginning);
            this->current_conversions[conversion_index].base_mem_addr[1] = (this->current_conversions[conversion_index].is_mov) ? 0x0 : this->mem_addr[1] + (uop->memory_size[0] * this->current_conversions[conversion_index].conversion_beginning);
            this->current_conversions[conversion_index].base_mem_addr[3] = this->mem_addr[3] + (uop->memory_size[0] * this->current_conversions[conversion_index].conversion_beginning);
#if VIMA_CONVERSION_DEBUG == 1
            printf("Base address[0]: %lu\n",  this->current_conversions[conversion_index].base_mem_addr[0]);
            printf("Base address[1]: %lu\n",  this->current_conversions[conversion_index].base_mem_addr[1]);
            printf("Base address[3]: %lu\n",  this->current_conversions[conversion_index].base_mem_addr[3]);
#endif

            // Verifica se a entrada acabou (então não tem as pŕoximas iterações para converter)
            if (orcs_engine.processor->traceIsOver &&
			    orcs_engine.processor->fetchBuffer.is_empty() &&
			    orcs_engine.processor->decodeBuffer.is_empty()) {
                    this->invalidation_reason["GENERATING INSTRUCTION BUT THE PROGRAM ALREADY FINISHED"]++;
                    this->stop_conversion(&this->current_conversions[conversion_index]);
                    return false;
                }



            // Create VIMA and insert into the pipeline
            this->generate_VIMA_instruction(&this->current_conversions[conversion_index]);

            // Create placeholder and insert into the pipeline
            this->generate_placeholder_instruction(&this->current_conversions[conversion_index]);

            this->current_conversions[conversion_index].vima_sent = true;

            CPU_to_VIMA_conversions[uop->memory_size[0]]++;
            this->current_conversions[conversion_index].mem_addr_confirmations_remaining = (this->current_conversions[conversion_index].is_mov) ? 2 * (VIMA_SIZE/uop->memory_size[0]) : 3 * (VIMA_SIZE/uop->memory_size[0]);

	        this->current_conversions[conversion_index].conversion_started = true;

                this->time_waiting_for_infos_stop = orcs_engine.get_global_cycle();
                this->time_waiting_for_infos += (this->time_waiting_for_infos_stop - this->time_waiting_for_infos_start);
        }
        /* Ignored instruction trying to confirm its stride */
        else if (this->current_conversions[conversion_index].vima_sent) {
            // **********************************
            // Check for the memory access stride
            // **********************************
            uint64_t expected = this->current_conversions[conversion_index].base_mem_addr[uop->linked_to_converter] + uop->memory_size[0] * (uop->linked_to_iteration - this->current_conversions[conversion_index].conversion_beginning);
            if (expected == uop->memory_address[0]) {
                this->current_conversions[conversion_index].mem_addr_confirmations_remaining--;
                ++this->AGU_result_from_current_conversion;
#if VIMA_CONVERSION_DEBUG == 1
                printf("//*****************************************************\n");
                printf("[%s]Confirmations remaining: %ld  [Conversion ID %lu]\n", (uop->uop_operation == INSTRUCTION_OPERATION_MEM_LOAD) ? "LOAD" : (uop->uop_operation == INSTRUCTION_OPERATION_MEM_STORE) ? "STORE": "???", this->current_conversions[conversion_index].mem_addr_confirmations_remaining, this->current_conversions[conversion_index].unique_conversion_id);
                printf("Received from %lu\n", uop->uop_number);
                printf("//*****************************************************\n");
#endif
                if (this->current_conversions[conversion_index].mem_addr_confirmations_remaining == 0) {
#if VIMA_CONVERSION_DEBUG == 1
                    printf("**********************************************\n");
                    printf("CPU requirements achieved! [Conversion ID %lu]\n", this->current_conversions[conversion_index].unique_conversion_id);
                    printf("*********************************************\n");
#endif
                    this->current_conversions[conversion_index].CPU_requirements_meet = true;
#if VIMA_CONVERSION_DEBUG == 1
                    printf("%lu CPU informing VIMA... [Conversion ID %lu]\n", orcs_engine.get_global_cycle(), this->current_conversions[conversion_index].unique_conversion_id);
#endif
                    orcs_engine.vima_controller->confirm_transaction(1 /* Success */, this->current_conversions[conversion_index].unique_conversion_id);
                    if (this->current_conversions[conversion_index].VIMA_requirements_meet && 
                        this->current_conversions[conversion_index].VIMA_requirements_meet_readyAt <= orcs_engine.get_global_cycle())
                        {
                            this->VIMA_before_CPU++;
                        } else {
                            this->CPU_before_VIMA++;
                        }
                
                }
            } else {
#if VIMA_CONVERSION_DEBUG == 1
                printf("Unexpected memory address (%lu != %lu): ", expected, uop->memory_address[0]);
#endif
                this->invalidation_reason["CONVERTED INSTRUCTION ADDRESS IS NOT THE EXPECTED"]++;
                orcs_engine.processor->conversion_mark_to_invalidate(this->current_conversions[conversion_index].unique_conversion_id);
                return false;
            }
        }
    }
    /* Is a result from an older conversion, already invalidated */
     else {
        ++this->AGU_result_from_wrong_conversion;
#if VIMA_CONVERSION_DEBUG == 1
        printf("AGU result from wrong conversion! (Received: %lu)\n", uop->unique_conversion_id);
#endif
    }
    return true;
}


// Based in the memory addresses accessed by each load and store and the last iteration inside
// ROB, this procedure defines the first and last vector indexes to be converted into a VIMA instruction
// Return true if could align the store, return false otherwise
bool vima_converter_t::get_index_for_alignment(conversion_status_t * current_conversion, uint32_t access_size, uint64_t base_store_address) {
    // Tenta encontrar o próximo índice com os stores alinhados ao início da linha de cache da vima
    // Pra evitar de duas conversões tentarem escrever sobre o mesmo bloco e a segunda ter que esperar a primeira confirmar :p
    
    uint64_t aligned_i = 0;
    for (uint64_t i=1; i < this->VIMA_SIZE; ++i) {
        uint64_t inicio = this->iteration + i;
        uint64_t tmp = base_store_address + (access_size * inicio);

        if (tmp % this->VIMA_SIZE == 0) {
            aligned_i = i;
            break;
        }
    }

    if (aligned_i == 0) {
        return false;
    }

    current_conversion->conversion_beginning = this->iteration + aligned_i;
    
    current_conversion->conversion_ending = this->iteration + aligned_i + (VIMA_SIZE/access_size) - 1;    

    
//#if VIMA_CONVERSION_DEBUG == 1
    printf("**** Definição de limites para conversão ****\n");
    printf("Store Inicial: %lu (VIMA_SIZE: %lu)\n", base_store_address + (access_size * current_conversion->conversion_beginning), this->VIMA_SIZE);
    printf("Iteração de início da conversão: %lu\n", current_conversion->conversion_beginning);
    printf("Iteração de fim da conversão:    %lu\n", current_conversion->conversion_ending);
    printf("*********************************************\n");
//#endif
    return true;
}

void vima_converter_t::confirm_mem_addr(uop_package_t *uop) {
    // Find correspondent conversion
    int32_t conversion_index = -1;
    for (uint32_t i=0; i<this->current_conversions.get_size(); ++i) {
        if (this->current_conversions[i].unique_conversion_id == uop->unique_conversion_id) {
            if (this->current_conversions[i].entry_can_be_removed == false) {
                conversion_index = i;
                break;
            }
        }
    }
    if (conversion_index >= 0) {
        this->current_conversions[conversion_index].mem_addr_confirmations_remaining--;
    }
}

// TODO
// Get data from VIMA and set registers with it before meet the requirements
void vima_converter_t::vima_execution_completed(memory_package_t *vima_package, uint64_t readyAt) {


    // ***************
    // Find conversion
    // ***************
    int32_t conversion_index = -1;
    for (uint32_t i=0; i < this->current_conversions.get_size(); ++i) {
        if (this->current_conversions[i].unique_conversion_id == vima_package->unique_conversion_id) {
            conversion_index = i;
            break;
        }
    }

    if (conversion_index >= 0) {
#if VIMA_CONVERSION_DEBUG == 1
    printf("******************************************************\n");
    printf("VIMA requirements achieved! [Conversion ID %lu] in %lu\n", vima_package->unique_conversion_id, readyAt);
    printf("******************************************************\n");
#endif
        this->current_conversions[conversion_index].VIMA_requirements_meet = true;
        this->current_conversions[conversion_index].VIMA_requirements_meet_readyAt = readyAt;
    } else {
        this->prefetcher->vima_execution_completed(vima_package, readyAt);
    }
}

void vima_converter_t::stop_conversion(conversion_status_t *invalidated_conversion) {
#if VIMA_CONVERSION_DEBUG == 1
    printf("Conversão %lu interrompida antes de iniciar!\n", invalidated_conversion->unique_conversion_id);
#endif

    // ***********************
    // Store in the black list
    // ***********************

    // Just the conversions that fail before even being converted once are blacklisted
    if (invalidated_conversion->first_conversion) {
        if (this->conversions_blacklist.is_full()) {
            this->conversions_blacklist.pop_front();
        }
        back_list_entry_t new_black_list_entry;
        new_black_list_entry.package_clean();
        new_black_list_entry.pc = invalidated_conversion->base_addr[0]; // Primeiro load da conversão que deu errado
        new_black_list_entry.uop_id = invalidated_conversion->base_uop_id[0];
        this->conversions_blacklist.push_back(new_black_list_entry);
        ++this->conversions_blacklisted;
    }

    // ****************************************
    // Libera a entrada da conversão invalidada
    // ****************************************
    invalidated_conversion->entry_can_be_removed = true;


    // *************************
	// Remove conversões antigas
	// *************************
	while(this->current_conversions.get_size() > 0 && this->current_conversions[0].entry_can_be_removed)
	{
		this->current_conversions.pop_front();
	}


    // ******************************
    // Libera para futuras conversões
    // ******************************
    this->start_new_conversion(); 
}

void vima_converter_t::invalidate_conversion(conversion_status_t *invalidated_conversion) {
#if VIMA_CONVERSION_DEBUG == 1
    printf("Conversão %lu invalidada!\n", invalidated_conversion->unique_conversion_id);
#endif

    // ***********************
    // Store in the black list
    // ***********************

    // Just the conversions that fail before even being converted once are blacklisted
    if (invalidated_conversion->first_conversion) {
        if (this->conversions_blacklist.is_full()) {
            this->conversions_blacklist.pop_front();
        }
        back_list_entry_t new_black_list_entry;
        new_black_list_entry.package_clean();
        new_black_list_entry.pc = invalidated_conversion->base_addr[0]; // Primeiro load da conversão que deu errado
        new_black_list_entry.uop_id = invalidated_conversion->base_uop_id[0];
        this->conversions_blacklist.push_back(new_black_list_entry);
        ++this->conversions_blacklisted;
    }

    // ******************************************************
    // Faz flush do pipeline
    // ******************************************************
    orcs_engine.processor->conversion_flush(invalidated_conversion);


    // *****************************************
    // Invalida cada uma das próximas conversões
    // *****************************************
    for (uint32_t i=0; i < this->current_conversions.get_size(); ++i)
    {
        if (this->current_conversions[i].entry_can_be_removed) {
			continue;
		}

        if (this->current_conversions[i].unique_conversion_id >= invalidated_conversion->unique_conversion_id) {

            // **********
            // Statistics
            // **********
            ++this->conversion_failed;
            
            // *************************************
            // Avisa VIMA para descartar o resultado
            // *************************************
            orcs_engine.vima_controller->confirm_transaction(2 /* Failure */, this->current_conversions[i].unique_conversion_id);

            // ***************************
            // Remove dependências com ela
            // ***************************
            orcs_engine.processor->resolve_registers_to(&this->current_conversions[i]);

            // ****************************************
            // Libera a entrada da conversão invalidada
            // ****************************************
            this->current_conversions[i].entry_can_be_removed = true;
#if VIMA_CONVERSION_DEBUG == 1
            printf("%lu Marcada para invalidação!\n", this->current_conversions[i].unique_conversion_id);
#endif
        }
    }

    // *********************
    // Invalidate prefetches
    // *********************
    this->contiguous_conversions = 0;
    conversion_status_t *prefetch;
    while((prefetch = this->prefetcher->get_prefetch())) {
        // **********
        // Statistics
        // **********
        ++this->prefetch_failed;

        // *************************************
        // Avisa VIMA para descartar o resultado
        // *************************************
        orcs_engine.vima_controller->confirm_transaction(2 /* Failure */, prefetch->unique_conversion_id);

        // **********************
        // Remove from prefetcher
        // **********************
        this->prefetcher->pop_prefetch();
    }

    // *************************
	// Remove conversões antigas
	// *************************
	while(this->current_conversions.get_size() > 0 && this->current_conversions[0].entry_can_be_removed)
	{
		this->current_conversions.pop_front();
	}


    // ******************************
    // Libera para futuras conversões
    // ******************************
    this->start_new_conversion(); 

}

void vima_converter_t::continue_conversion(conversion_status_t *prev_conversion) {
#if VIMA_CONVERSION_DEBUG == 1
    printf("***************************************************\n");
    printf("Creating a subsequent conversion...\n");
    printf("***************************************************\n");

    printf("Trying to allocate a new converter entry...\n");
#endif
    // Prefetch counter
    this->contiguous_conversions++;

    // Conversion generation
    conversion_status_t new_conversion;
    new_conversion.package_clean();
    int32_t entry = this->current_conversions.push_back(new_conversion);
    if (entry != -1) {
#if VIMA_CONVERSION_DEBUG == 1
        printf("Entry allocated!\n");
#endif
        ++this->conversion_entry_allocated;
        this->current_conversion = &this->current_conversions[entry];

        this->iteration = 2;
        this->state_machine = 0;

        // Write register control
        for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


#if VIMA_CONVERSION_DEBUG == 1
        printf("Continue with conversion id: %lu\n", next_unique_conversion_id);
#endif
        this->current_conversion->is_mov = prev_conversion->is_mov;
        this->current_conversion->unique_conversion_id = next_unique_conversion_id++;

        this->current_conversion->conversion_started = true;
        this->current_conversion->conversion_beginning = 2;

        this->current_conversion->infos_remaining = 0;


        CPU_to_VIMA_conversions[prev_conversion->mem_size]++;
        this->current_conversion->conversion_ending = this->current_conversion->conversion_beginning + (VIMA_SIZE/prev_conversion->mem_size) - 1;
        this->current_conversion->mem_addr_confirmations_remaining = (prev_conversion->is_mov) ? 2 * (VIMA_SIZE/prev_conversion->mem_size) : 3 * (VIMA_SIZE/prev_conversion->mem_size);


        this->current_conversion->vima_sent = false;
        this->current_conversion->CPU_requirements_meet = false;
        this->current_conversion->VIMA_requirements_meet = false;
        this->current_conversion->VIMA_requirements_meet_readyAt = 0;

    
    this->current_conversion->base_mem_addr[0] = prev_conversion->base_mem_addr[0] + (prev_conversion->mem_size * (VIMA_SIZE/prev_conversion->mem_size));
    this->current_conversion->base_mem_addr[1] = (prev_conversion->is_mov) ? 0x0 : prev_conversion->base_mem_addr[1] + (prev_conversion->mem_size * (VIMA_SIZE/prev_conversion->mem_size));
    this->current_conversion->base_mem_addr[3] = prev_conversion->base_mem_addr[3] + (prev_conversion->mem_size * (VIMA_SIZE/prev_conversion->mem_size));
    this->current_conversion->mem_size = prev_conversion->mem_size;

        // Send VIMA speculatively
        // Check for prefetches
        conversion_status_t *prefetch = this->prefetcher->get_prefetch();
        if (prefetch && this->current_conversion->IsEqual(prefetch)) {
#if VIMA_CONVERSION_DEBUG == 1
        printf("//**********************************\n");
        printf("Using prefetched VIMA instruction...\n");
        printf("//**********************************\n");
        printf("Base address[0]: %lu\n",  this->current_conversion->base_mem_addr[0]);
        printf("Base address[1]: %lu\n",  this->current_conversion->base_mem_addr[1]);
        printf("Base address[3]: %lu\n",  this->current_conversion->base_mem_addr[3]);
        printf("VIMA_requirements_meet: %s\n",  prefetch->VIMA_requirements_meet ? "true" : "false");
        printf("VIMA_requirements_meet_readyAt: %lu\n",  prefetch->VIMA_requirements_meet_readyAt);
#endif
            this->current_conversion->VIMA_requirements_meet = prefetch->VIMA_requirements_meet;
            this->current_conversion->VIMA_requirements_meet_readyAt = prefetch->VIMA_requirements_meet_readyAt;
            this->current_conversion->vima_sent = true;
            this->prefetcher->pop_prefetch();
            this->prefetched_vima_used++;
        } else {
            assert(prefetch == NULL);
#if VIMA_CONVERSION_DEBUG == 1
        printf("//****************************\n");
        printf("Generating VIMA instruction...\n");
        printf("//****************************\n");
        printf("Base address[0]: %lu\n",  this->current_conversion->base_mem_addr[0]);
        printf("Base address[1]: %lu\n",  this->current_conversion->base_mem_addr[1]);
        printf("Base address[3]: %lu\n",  this->current_conversion->base_mem_addr[3]);
#endif
        this->generate_VIMA_instruction(this->current_conversion);
        this->current_conversion->vima_sent = true;

        }
        if (this->contiguous_conversions >= this->CONTIGUOUS_CONVERSIONS_TO_PREFETCH) {
            this->prefetcher->make_prefetch(this->current_conversion);
        }

        //  Placeholder
        this->generate_placeholder_instruction(this->current_conversion);
    }
    // *******************************
    // An entry could not be allocated
    // *******************************
    else {
#if VIMA_CONVERSION_DEBUG == 1
        printf("Entry could not be allocated!\n");
#endif
        if (this->current_conversion != 0x0) {
            ++this->not_enough_conversion_entries;
        }
        printf("ALERT: Could not allocate!\n");
        this->current_conversion = 0x0;

        this->iteration = UINT32_MAX;
        this->state_machine = UINT8_MAX;

        // Write register control
        for (uint32_t i=0; i < 259; ++i) regs_list[i] = false;


        // Conversion data
        for (uint32_t i=0; i < 8; ++i) {
            this->addr[i] = 0;
            this->uop_id[i] = 0;
            this->mem_addr[i] = 0;
        }

    }
}

void vima_converter_t::clock() {
    this->check_conversions();
}

bool vima_converter_t::is_blacklisted(uop_package_t *uop) {
    for (uint32_t i=0; i < this->conversions_blacklist.get_size(); ++i) {
        if (this->conversions_blacklist[i].pc == uop->opcode_address &&
            this->conversions_blacklist[i].uop_id == uop->uop_id) {
#if VIMA_CONVERSION_DEBUG == 1
            printf("**********************************************\n");
            printf("Blacklisted load found! Addr: %lu uop_id: %u\n", uop->opcode_address, uop->uop_id);
            printf("**********************************************\n");
#endif
                ++this->avoided_by_the_blacklist;
                return true;
            }
    }
    return false;
}

conversion_status_t *vima_converter_t::get_conversion(uint64_t conversion_unique_id) {
    for (uint32_t i=0; i < this->current_conversions.get_size(); ++i) {
        if (this->current_conversions[i].unique_conversion_id == conversion_unique_id) {
            return &this->current_conversions[i];
        }
    }
    return 0x0;
}