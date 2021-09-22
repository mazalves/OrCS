#include "./../simulator.hpp"

void registers_tracker_t::allocate (table_of_loads_t *tl, 
                                    table_of_operations_t *to, 
                                    table_of_stores_t *ts,
                                    table_of_vectorizations_t *tv,
                                    table_of_ignored_t *ti,
                                    table_of_pre_vectorization_t *tpv,
                                    vectorizer_t *vectorizer) {
    this->tl = tl;
    this->to = to;
    this->ts = ts;
    this->tv = tv;
    this->ti = ti;
    this->tpv = tpv;
    this->vectorizer = vectorizer;
    this->num_entries = 259; // Deve englobar o 258 para dependências, 
                             //visto que não dá pra executar só o começo de uma inst na memória :p
    this->entries = new registers_tracker_entry_t[this->num_entries];
    for (uint32_t i=0; i<this->num_entries; ++i) {
        this->entries[i].allocate();
    }
}
// #######################################################################################################


void registers_tracker_t::committed_ld (uop_package_t *uop) {
    bool active_debug = false;
    //if (uop->opcode_address == 94318318040695) active_debug = true;
    //if (uop->opcode_address == 94318318040700) active_debug = true;
    if (active_debug) printf("RT Op: %lu (LOAD)\n", uop->opcode_address);


    // Registradores de leitura
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->read_regs[i] != POSITION_FAIL); ++i) {
        registers_tracker_entry_t *entry = &this->entries[uop->read_regs[i]];
        // ****************
        // Faz invalidações
        // ****************
        // // Origem: Load
        if (entry->tl_pointer != NULL) {
            this->tl->start_invalidation(entry->tl_pointer);
        }
        // // Origem: Operação
        if (entry->to_pointer != NULL) {
            this->to->start_invalidation(entry->to_pointer);
        }
        // // Origem: Escrita (não escreve em regs)
        assert(entry->ts_pointer == NULL);
        // ******************
        // Atualiza ponteiros
        // ******************
        entry->tl_pointer = NULL;
        entry->to_pointer = NULL;
    }

    // Atualiza a tabela de loads
    // Como loads escrevem em um registrador, pode ser que aloque a mesma posição que um load
    // anterior estava linkado, assim precisamos alocar só depois das invalidações.
    table_of_loads_entry_t *tl_pointer = this->tl->new_ld(uop);


    // Registradores de escrita
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->write_regs[i] != POSITION_FAIL); ++i) {
        registers_tracker_entry_t *entry = &this->entries[uop->write_regs[i]];

        // ******************
        // Atualiza ponteiros
        // ******************
        entry->tl_pointer = tl_pointer;
        entry->to_pointer = NULL;
        entry->ts_pointer = NULL;
        //printf("LD escrevendo em reg %d\n", uop->write_regs[i]);
    }

}
// #######################################################################################################

void registers_tracker_t::committed_op (uop_package_t *uop) {
    bool active_debug = false;
    //if (uop->opcode_address == 94318318040704) active_debug = true;
    if (active_debug) printf("Op: %lu\n", uop->opcode_address);


    bool vectorizable_origins = true;
    // *****************************
    // Busca entrada existente na TO
    // *****************************
    bool valid_to_entry = false;

    
    uint32_t to_entry = to->find_id(uop, &valid_to_entry);
    table_of_operations_entry_t *to_pointer = (valid_to_entry) ? to->get_id(to_entry) : NULL;
    // ************************
    // Registradores de leitura
    // ************************
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->read_regs[i] != POSITION_FAIL); ++i) {
        registers_tracker_entry_t *entry = &this->entries[uop->read_regs[i]];
        //if (active_debug) printf (" [%d]", uop->read_regs[i]);

        // *************************************************
        // Faz invalidações && verifica origens vetorizáveis
        // *************************************************
        // // Origem: Load
        if (entry->tl_pointer == NULL) {
            if (active_debug) printf(">> Load não vetorizável\n");
            vectorizable_origins = false;
        } else {
            // ++++++++++++++++++++++++
            // Não possui entrada na TO
            // ++++++++++++++++++++++++
            if (!valid_to_entry) {
                if (entry->tl_pointer->linked_to_ts == true) {
                    vectorizable_origins = false;

                    this->tl->start_invalidation(entry->tl_pointer);
                    entry->tl_pointer = NULL;
                    if (active_debug) printf(">> Load ligado a outro\n");

                }
            } else {
                if (entry->tl_pointer->linked_to_ts == false) {
                    vectorizable_origins = false;
                    if (active_debug) printf(">> Op ligada a outros loads\n");

                } else if ((entry->tl_pointer->is_mov == true)      ||
                    (entry->tl_pointer->to_ts_entry != to_entry)) {
                    vectorizable_origins = false;

                    this->tl->start_invalidation(entry->tl_pointer);
                    entry->tl_pointer = NULL;
                    if (active_debug) printf(">> Load ligado a store\n");

                }
            }
            // +++++++++++++++++++++++++++++++
            // O load não foi confirmado ainda
            // +++++++++++++++++++++++++++++++
            if (entry->tl_pointer->vectorizable == false) {
                vectorizable_origins = false;
                if (active_debug) printf(">> Load não confirmado como vetorizável\n");

            }
        }
        // // Origem: Operação
        if (entry->to_pointer != NULL) {
            vectorizable_origins = false;
            this->to->start_invalidation(entry->to_pointer);
            entry->to_pointer = NULL;
            if (active_debug) printf(">> Origem: operação\n");

        }
        // // Origem: Escrita (não escreve em regs)
        assert(entry->ts_pointer == NULL);
    }

    // *******************************************
    // Não possui nem dois registradores de origem
    // *******************************************
    if (uop->read_regs[0] == POSITION_FAIL || uop->read_regs[1] == POSITION_FAIL) {
        vectorizable_origins = false;
        if (active_debug) printf("** Sem dois registrdores de origem\n");

    }
    // **********************
    // Marca como vetorizável
    // **********************
    if (vectorizable_origins) {
        // ++++++++++++++++++++++++++++++++++++
        // Vincula aos registradores de leitura
        // ++++++++++++++++++++++++++++++++++++
        // // Tenta alocar/encontrar entrada na/da TO
        if (!valid_to_entry) {
            to_entry = to->new_entry_id(uop, &valid_to_entry);
            to_pointer = to->get_id(to_entry);
        }
        if (valid_to_entry == true) {
            for (int32_t i=0; (i < MAX_REGISTERS) && (uop->read_regs[i] != POSITION_FAIL); ++i) {
                registers_tracker_entry_t *entry = &this->entries[uop->read_regs[i]];
                entry->tl_pointer->linked_to_ts = true;
                entry->tl_pointer->is_mov = false;
                entry->tl_pointer->to_ts_entry = to_entry;
                to_pointer->tl_entries[i] = entry->tl_pointer;
                assert(i < 2);
            }
            if (active_debug) printf(" >> Operação (%lu) vinculada a LOADs\n", uop->opcode_address);

            if (active_debug) this->to->print();
        }
    } else /* Se não for vetorizável */ {
        // +++++++++++++++++++++++++++++++++++++++++++++++++
        // Invalida vetorizações e entradas na TO existentes
        // +++++++++++++++++++++++++++++++++++++++++++++++++
        if (valid_to_entry) {
            to->start_invalidation(to_pointer);
            valid_to_entry = false;
            to_entry = 0;
            to_pointer = NULL;
        }
    }

    // Atualiza a tabela de operações (basicamente o LRU da entrada, se existir)
    this->to->new_op(uop);

    // Registradores de escrita
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->write_regs[i] != POSITION_FAIL); ++i) {
        registers_tracker_entry_t *entry = &this->entries[uop->write_regs[i]];

        // ******************
        // Atualiza ponteiros
        // ******************
        entry->tl_pointer = NULL;
        entry->to_pointer = to_pointer;
        entry->ts_pointer = NULL;
    
        if (active_debug) printf(" --> Vinculado a %d\n", uop->write_regs[i]);
        assert (entry->to_pointer == NULL || entry->to_pointer->free == false);
    }
}
// #######################################################################################################

void registers_tracker_t::committed_st (uop_package_t *uop) {
    bool active_debug = false;
    //if (uop->opcode_address == 94318318040707) active_debug = true;
    if (active_debug) printf("Op: %lu (STORE)\n", uop->opcode_address);

    bool vectorizable_origin = true;


    // Atualiza a tabela de stores
    // Como stores não escrevem, não deveria ter o problema de alocar a entrada do anterior.
    this->ts->new_st(uop);

    // *****************************
    // Busca entrada existente na TS
    // *****************************
    bool valid_ts_entry = false;
    uint32_t ts_entry = ts->find_id(uop, &valid_ts_entry);
    table_of_stores_entry_t *ts_pointer = (valid_ts_entry) ? ts->get_id(ts_entry) : NULL;
    // Registrador de leitura (apenas o último é a origem dos dados, o restante são endereços)
    int32_t source_reg = -1;
    for (source_reg=0; (source_reg < MAX_REGISTERS) && (uop->read_regs[source_reg] != POSITION_FAIL); ++source_reg);
    if (source_reg < MAX_REGISTERS) source_reg = uop->read_regs[source_reg-1]; 
    else source_reg = -1;
    if (source_reg != -1) {
        registers_tracker_entry_t *entry = &this->entries[source_reg];
        if (active_debug) printf (" [%d] ", source_reg);
        // ***************************************************
        // Verifica se a operação anterior está linkada a essa
        // ***************************************************
        // // Se for um load
        if (entry->tl_pointer != NULL) {
            
            if (valid_ts_entry) {
                if (entry->tl_pointer->linked_to_ts == false) {
                    if (ts_pointer->tv_entry != NULL) {

                        this->tl->start_invalidation(entry->tl_pointer);
                        entry->tl_pointer = NULL;
                        vectorizable_origin = false;
                        if (active_debug) printf(">> Ligado a outra vetorização\n");
                    }
                    
                } else if ((entry->tl_pointer->is_mov == false) ||
                           (entry->tl_pointer->to_ts_entry != ts_entry))
                {

                    this->tl->start_invalidation(entry->tl_pointer);
                    entry->tl_pointer = NULL;
                    vectorizable_origin = false;
                    if (active_debug) printf(">> Load ligado a uma operação\n");
                }
            } else {
                if (entry->tl_pointer->linked_to_ts == true) {

                    this->tl->start_invalidation(entry->tl_pointer);
                    entry->tl_pointer = NULL;
                    vectorizable_origin = false;
                    if (active_debug) printf(">> Load ligado a outro store/operação\n");
                }
            }
        }
        // // Se for uma operação
        if (entry->to_pointer != NULL) {
             if (valid_ts_entry) {
                 /* Se operação está vinculada a outro store */
                if (entry->to_pointer->ts_entry != NULL && entry->to_pointer->ts_entry != ts_pointer) {
                    this->to->start_invalidation(entry->to_pointer);
                    entry->to_pointer = NULL;
                    vectorizable_origin = false;
                    if (active_debug) printf(">> Operação está vinculada a outro store\n");
                }
                /* Se store estiver vinculado a outra operação */
                else if ((entry->to_pointer->ts_entry == NULL) && (ts_pointer->tv_entry != NULL)) {
                    vectorizable_origin = false;
                    this->to->start_invalidation(entry->to_pointer);

                    entry->to_pointer = NULL;
                    vectorizable_origin = false;
                    if (active_debug) printf(">> Store estiver vinculado a outra operação\n");
                }
            } else {
                if (entry->to_pointer->ts_entry != NULL) {
                    this->to->start_invalidation(entry->to_pointer);
                    entry->to_pointer = NULL;
                    vectorizable_origin = false;
                    if (active_debug) printf(">> Operação ligada a outro store\n");
                }
            }
        }
        // // Se for um store (deveria ser impossível :p)
        assert (entry->ts_pointer == NULL);


        // Se for outra coisa
        if (entry->tl_pointer == NULL && entry->to_pointer == NULL && entry->ts_pointer == NULL) {
            vectorizable_origin = false;
            if (active_debug) printf(">> Nem veio de um load/op válido\n");
        }
    }


    // ++++++++++++++++++++++++++++++++
    // Faz a vetorização a partir da TS 
    // ++++++++++++++++++++++++++++++++
    if (vectorizable_origin) {
        //this->ts->print();

        /* Aponta gerador das leituras para a TS */
        if (valid_ts_entry == true && ts_pointer->vectorizable && ts_pointer->tv_entry == NULL) {

            if (source_reg != -1) {
                registers_tracker_entry_t *entry = &this->entries[source_reg];
                assert (entry->tl_pointer != NULL || entry->to_pointer != NULL);
                assert ((entry->tl_pointer != NULL && entry->to_pointer == NULL) || 
                        (entry->tl_pointer == NULL && entry->to_pointer != NULL));

                if (entry->tl_pointer && entry->tl_pointer->vectorizable) {
                    entry->tl_pointer->is_mov = true;
                    entry->tl_pointer->to_ts_entry = ts_entry;
                    entry->tl_pointer->linked_to_ts = true;
                    ts_pointer->is_mov = true;
                    ts_pointer->linked_tl_to = true;
                    ts_pointer->tl_to_entry = tl->entry_to_id(entry->tl_pointer);
                    if (active_debug) printf(" >> Ligando store a load pelo reg %d (tl_entry: %d)\n", source_reg, this->tl->entry_to_id(entry->tl_pointer));
                    /* Vetoriza */
                    this->ts->vectorize(ts_pointer);

                } else if (entry->to_pointer) {

                    entry->to_pointer->ts_entry = ts_pointer;
                    ts_pointer->is_mov = false;
                    ts_pointer->linked_tl_to = true;
                    ts_pointer->tl_to_entry = to->entry_to_id(entry->to_pointer);
                    if (active_debug) printf(" >> Ligando store a operação pelo reg %d (to_entry: %d)\n", source_reg, this->to->entry_to_id(entry->to_pointer));
                    /* Vetoriza */
                    this->ts->vectorize(ts_pointer);
                }
            }
            
            
        }
    } else {

        if (valid_ts_entry && ts_pointer->linked_tl_to) {
            if (active_debug) printf(" >> Invalidando cadeia a partir de store (%lu)\n", uop->opcode_address);
            this->ts->start_invalidation(ts_pointer);
            valid_ts_entry = false;
            ts_entry = 0;
            ts_pointer = NULL;
        }
        
    }
    // Registradores de escrita
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->write_regs[i] != POSITION_FAIL); ++i) {
        registers_tracker_entry_t *entry = &this->entries[uop->write_regs[i]];
        // ******************
        // Atualiza ponteiros
        // ******************
        entry->tl_pointer = NULL;
        entry->to_pointer = NULL;
        entry->ts_pointer = NULL;
        //entry->ts_pointer = ts_pointer; // Se escrever não pode vetorizar :p
        if (active_debug) printf(" >> Invalidando por possuir reg de escrita (%d)\n", uop->write_regs[i]);
        this->ts->start_invalidation(ts_pointer);
    }

}
// #######################################################################################################


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Rename
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Verifica se está lendo de alguma vetorização que não é sua própria
// Se estiver deve invalidar a ambas as vetorizações
void registers_tracker_t::check_read_registers(uop_package_t *uop, table_of_vectorizations_entry_t *tv_entry) {
    
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->read_regs[i] != POSITION_FAIL); ++i) {
        registers_tracker_entry_t *entry = &this->entries[uop->read_regs[i]];
        
        if ((entry->tv_pointer) && (entry->tv_pointer != tv_entry)) {
            this->tv->start_invalidation(entry->tv_pointer);
            if (tv_entry) {
                this->tv->start_invalidation(tv_entry);
            }
        }
    }
}


void registers_tracker_t::register_overwritten(uop_package_t *uop, table_of_vectorizations_entry_t *tv_entry) {
    // Anota caso sobrescreva algum registrador (Completar validação final)
    uint8_t num_regs = 0;
    for (int32_t i=0; (i < MAX_REGISTERS) && (uop->write_regs[i] != POSITION_FAIL); ++i) {
        ++num_regs;
        registers_tracker_entry_t *entry = &this->entries[uop->write_regs[i]];
        //printf("Wrote in register %d\n", i);
        //printf("From write Regs[i] = %d\n", uop->write_regs[i]);
        //printf("From entry: %p[%d]/%p[%d]\n", (void *)&this->entries[uop->write_regs[i]], uop->write_regs[i], (void *)&this->entries[this->num_entries], this->num_entries);
        if(entry->tv_pointer) {
            //printf("[%d] Sobrescrita para o %p pela uop %lu (%s)\n", uop->write_regs[i], (void *) entry->tv_pointer, uop->opcode_address, uop->opcode_assembly);
            this->tv->register_overwritten(entry->tv_pointer);
        }

        // Novo valor
        //printf("[%d] Definindo como %p pela uop: %lu (%s)\n", uop->write_regs[i], (void *) tv_entry, uop->opcode_address, uop->opcode_assembly);

        entry->set_rename_pointers(tv_entry);
    }
    // Marca quantos novos foram escritos
    if (tv_entry) tv_entry->register_add(num_regs);
}


void registers_tracker_t::renamed_ld (uop_package_t *uop) {
    //printf("Load %lu\n", uop->opcode_address);

    // Tracking
    table_of_vectorizations_entry_t *tv_pointer = NULL;

    // ************************************
    // Get Address and change state machine
    // ************************************
    tv_pointer = this->tpv->get_tv_entry(uop->opcode_address, uop->uop_id);

    if (tv_pointer) {
        // +++++++++++++++++++++++++
        // Descobre qual dos loads é
        // +++++++++++++++++++++++++
        table_of_loads_entry_t *load_entry[2];
        if (tv_pointer->ts_entry->is_mov) {
            load_entry[0] = this->tl->get_id(tv_pointer->ts_entry->tl_to_entry);
            load_entry[1] = NULL;
        } else {
            table_of_operations_entry_t *to_pointer = this->to->get_id(tv_pointer->ts_entry->tl_to_entry);
            // Invalida TL
            load_entry[0] = to_pointer->tl_entries[0];
            load_entry[1] = to_pointer->tl_entries[1];
        }

        //printf("Preenchendo tv com loads\n");
        switch(tv_pointer->state) {
            case 0:
                if ((load_entry[0]->get_pc() == uop->opcode_address) && (load_entry[0]->get_uop_id() == uop->uop_id)) {
                    tv_pointer->addr_mem_load[0] = uop->memory_address[0];
                    tv_pointer->after_last_byte_mem_load[0] = uop->memory_address[0] + tv_pointer->load_stride[0] * tv_pointer->num_elements;
                    
                    //printf("Definindo primeiro load: %lu <--> %lu\n", uop->memory_address[0], tv_pointer->after_last_byte_mem_load[0]);
                    this->vectorizer->statistics_counters[VECTORIZER_LOAD_1_DEFINED]++;

                    tv_pointer->set_state(1);

                    // ++++++++++++++++++++++++++++++++++++++++++++++++ 
                    // Insere na lista de ignoradas
                    // ++++++++++++++++++++++++++++++++++++++++++++++++
                    if (this->vectorizer->ti->has_n_vacancies((tv_pointer->ts_entry->is_mov) ? 2 : 4) == false) {
                        //printf("Sem vagas na lista de ignoradas...\n");
                        this->vectorizer->statistics_counters[VECTORIZER_TI_NOT_ENOUGH_ENTRIES]++;
                        
                        tv_pointer->set_state(0);            
                        break;
                    }

                    this->ti->insert_vectorization(tv_pointer);
                }
                break;
            case 1:
                assert (load_entry[1] != NULL);
                if ((load_entry[1]->get_pc() == uop->opcode_address) && (load_entry[1]->get_uop_id() == uop->uop_id)) {
                    tv_pointer->addr_mem_load[1] = uop->memory_address[0];
                    tv_pointer->after_last_byte_mem_load[1] = uop->memory_address[0] + tv_pointer->load_stride[1] * tv_pointer->num_elements;
                    
                    //printf("Definindo segundo load: %lu <--> %lu\n", uop->memory_address[0], tv_pointer->after_last_byte_mem_load[1]);
                    this->vectorizer->statistics_counters[VECTORIZER_LOAD_2_DEFINED]++;
                    
                    tv_pointer->set_state(2);
                }
                break;
            default:
                printf("Estado inesperado! Recebi load no rename com estado %d!\n", tv_pointer->state);
                exit(1);
                break;
        }
    } else {
        uint8_t structural_id = 0;
        tv_pointer = this->ti->get_tv_entry(uop->opcode_address, uop->uop_id, &structural_id);
    }

    // Tracking com a entrada correspondente
    this->check_read_registers(uop, tv_pointer);
    this->register_overwritten(uop, tv_pointer);

}

void registers_tracker_t::renamed_op (uop_package_t *uop) {
    //printf("Operation %lu\n", uop->opcode_address);

    // Tracking
    table_of_vectorizations_entry_t *tv_pointer = NULL;
    uint8_t structural_id = 0;
    tv_pointer = this->ti->get_tv_entry(uop->opcode_address, uop->uop_id, &structural_id);
    this->check_read_registers(uop, tv_pointer);
    this->register_overwritten(uop, tv_pointer);


}

void registers_tracker_t::renamed_st (uop_package_t *uop) {
    //printf("Store %lu\n", uop->opcode_address);
    // Tracking
    table_of_vectorizations_entry_t *tv_pointer = NULL;
    uint8_t structural_id = 0;
    tv_pointer = this->ti->get_tv_entry(uop->opcode_address, uop->uop_id, &structural_id);

    /* Como o store define a pŕoxima vetorização, precisa sobrescrever os registradores antes */
    this->check_read_registers(uop, tv_pointer);
    this->register_overwritten(uop, tv_pointer);

    if (tv_pointer) {
        // *************************************************
        // Avisa sobre o fim de uma iteração, descobrindo se
        // deve parar de ignorar
        // *************************************************
        assert (tv_pointer->state < 5);
        tv_pointer->increment_validation();
        if (tv_pointer->has_ended()) {
            // +++++++++++++++++++++++++++++++++++++++++++++++
            // Todas as instruções da vetorização entraram no
            // pipeline. Assim realiza a vetorização da
            // próxima parte.
            // +++++++++++++++++++++++++++++++++++++++++++++++

            bool status = this->tv->new_pre_vectorization(tv_pointer->ts_entry);
            if (status) {
                printf("%p substituido por %p na tabela de ignorados!\n", (void *)tv_pointer, (void *)tv_pointer->ts_entry->tv_entry);
                /* Remove ponteiro antigo das waiting */ // O novo vai entrar no primeiro load
                this->ti->remove_vectorization(tv_pointer);
                tv_pointer->ts_entry = NULL;
            } else {
                printf("Não conseguiu substituto para %p na tabela de ignorados!\n", (void *)tv_pointer);
                this->tv->unbind(tv_pointer);
                this->ti->remove_vectorization(tv_pointer);
            }
            /* Marca para ainda ignorar esse store e o vincula à TV_entry anterior: */
            uop->waiting = true;
            uop->tv_pointer = tv_pointer;
            uop->structural_id = 3; // Store

            /* Faz apenas esperar commits */
            tv_pointer->set_state(5);
        }
    }

    // ************************************
    // Get Address and change state machine
    // ************************************
    table_of_vectorizations_entry_t *tpv_tv_pointer = NULL;
    if ((tpv_tv_pointer = this->tpv->get_tv_entry(uop->opcode_address, uop->uop_id))) {

        switch(tpv_tv_pointer->state) {
            case 1:
                assert (tpv_tv_pointer->ts_entry->is_mov);
                tpv_tv_pointer->addr_mem_store = uop->memory_address[0];
                tv_pointer->after_last_byte_mem_store = uop->memory_address[0] + tv_pointer->store_stride * tv_pointer->num_elements;
                
                //printf("Definindo store em mov: %lu <--> %lu\n", uop->memory_address[0], tv_pointer->after_last_byte_mem_store);
                this->vectorizer->statistics_counters[VECTORIZER_STORE_DEFINED_MOV]++;
                
                tpv_tv_pointer->set_state(3);
                break;
            case 2:
                tpv_tv_pointer->addr_mem_store = uop->memory_address[0];
                tv_pointer->after_last_byte_mem_store = uop->memory_address[0] + tv_pointer->store_stride * tv_pointer->num_elements;
                
                //printf("Definindo store em operação: %lu <--> %lu\n", uop->memory_address[0], tv_pointer->after_last_byte_mem_store);
                this->vectorizer->statistics_counters[VECTORIZER_STORE_DEFINED_OP]++;
                
                
                tpv_tv_pointer->set_state(3);
                break;
        }
        switch(tpv_tv_pointer->state) {
            case 0: // Deve esperar pelos loads
                break;
            case 3:
            // ******************************************
            // Launch VIMA instruction and start ignoring
            // ******************************************
                // ++++++++++++++++++++++++++++++++++++++++++++++++        
                // Verifica se pode adicionar a instrução ao buffer
                // ++++++++++++++++++++++++++++++++++++++++++++++++ 
                if (this->vectorizer->vectorizations_to_execute.is_full()) {
                    tpv_tv_pointer->set_state(0);
                    return;
                }


                // ++++++++++++++++++++++++++++++++++++++++++++++++ 
                // Insere no pipeline
                // ++++++++++++++++++++++++++++++++++++++++++++++++ 
                this->tv->generate_VIMA_instruction(tpv_tv_pointer);


                // ++++++++++++++++++++++++++++++++++++++++++++++++
                // Remove da lista de pré-vetorizações
                // ++++++++++++++++++++++++++++++++++++++++++++++++
                this->tpv->remove_vectorization(tpv_tv_pointer);

                // ++++++++++++++++++++++++++++++++++++++++++++++++
                // Evita fazer o processo novamente
                // ++++++++++++++++++++++++++++++++++++++++++++++++
                tpv_tv_pointer->set_state(4);
                break;

            default:
                printf("Estado inesperado! Recebi store no rename com estado %d!\n", tpv_tv_pointer->state);
                exit(1);
                break;
        }
    }
}
