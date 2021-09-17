#include "./../simulator.hpp"

 // Deve ser chamado antes de qualquer execução
void table_of_stores_t::allocate (libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_operations_t *to, table_of_vectorizations_t *tv)  { 
        uint32_t size = vectorizer_configs["TS_SIZE"];
        assert (size > 0);

        this->entries = new table_of_stores_entry_t[size];
        for (uint32_t i=0; i<size; ++i) {
            this->entries[i].allocate(vectorizer_configs);
        }

        this->max_entries = size;

        this->tl = tl;
        this->to = to;
        this->tv = tv;

 }

void table_of_stores_t::new_st (uop_package_t *uop) {
    bool active_debug = false;
    //if (uop->opcode_address == 94318318040707) active_debug = true;
    // ********************************
    // Não tenta vetorizar scatters
    // ********************************
    if (uop->num_mem_operations > 1) {
        return;
    }

    // ********************************
    // Procura na tabela
    // ********************************
    table_of_stores_entry_t *entry = NULL;
    entry = this->find(uop);

    // ***************************************
    // Se não encontrar, tenta adicionar na TL
    // ***************************************
    if (entry == NULL) {
        entry = this->add_ts (uop);
    } else {
        // **************
        // Valida o store
        // **************
        if (entry->confidence_counter == 0) {
            entry->stride = uop->memory_address[0] - entry->last_address;
            entry->last_address = uop->memory_address[0];
            entry->inc_confidence_counter(orcs_engine.get_global_cycle()); // Característica do 1 é ter um stride

        } else {
            int32_t stride = uop->memory_address[0] - entry->last_address;
            // -----
            // Falha
            // -----
            if (stride != entry->stride) {
                // +++++++++++++++++++++++++++++++++++++++++
                // Invalida vetorização, ou link, se existir
                // +++++++++++++++++++++++++++++++++++++++++
                this->start_invalidation(entry);

                // +++++++++++++++++++
                // Reinicia o Tracking
                // +++++++++++++++++++
                entry->reset_confidence_counter(orcs_engine.get_global_cycle());
                entry->vectorizable = false;
                entry->stride = stride;
                entry->inc_confidence_counter(orcs_engine.get_global_cycle());
            }
            // -------
            // Sucesso
            // -------
            else {
                entry->inc_confidence_counter(orcs_engine.get_global_cycle());
                entry->check_vectorizable(uop);
            }
            // -----------------------------------------
            // Salva o endereço para a próxima validação
            // -----------------------------------------
            entry->last_address = uop->memory_address[0];

        }
    }
    if (active_debug) this->print();

}


table_of_stores_entry_t* table_of_stores_t::add_ts (uop_package_t *uop) {
// **********************************************************
    // Busca entrada livre ou LRU
    // Apenas seleciona aquelas sem vetorização ou em treinamento
    // **********************************************************
    table_of_stores_entry_t *choice = NULL;
    for (uint32_t i=0; i < this->max_entries; ++i) {
        // Tenta pegar qualquer um sem vetorização ou em treinamento (qualquer possível)
        if (!choice) {
            if (this->entries[i].tv_entry == NULL) {
                choice = &this->entries[i];
            } else if (this->entries[i].tv_entry->state < 4) {
                choice = &this->entries[i];
            }
        } else { /* Se já tiver uma escolha */
            //  Tenta trocar uma  vetorizada por uma livre
            if ((choice->tv_entry) && (this->entries[i].tv_entry == NULL)) {
                choice = &this->entries[i];
            } else if (choice->tv_entry) { /* Se não se encaixar nisso e ambos estiverem vetorizados, tenta avaliar o LRU */
                /* Só avalia o LRU se puder (a outra estiver treinando) */
                if (this->entries[i].tv_entry->state < 4) {
                    if (this->entries[i].lru < choice->lru) {
                        choice = &this->entries[i];
                    }
                }
            } else {
                /* Só avalia o LRU se ambos não forem vetorizados, já que pegar um vetorizado seria pior */
                if (this->entries[i].tv_entry == NULL) {
                    if (this->entries[i].lru < choice->lru) {
                        choice = &this->entries[i];
                    }
                }
            }
        }

        /* No melhor dos casos, pega uma entrada livre */
        if (this->entries[i].free) {
            choice = &this->entries[i];
            break;
        }
    }

    // *********************************
    // Indica que uma entrada foi obtida
    // *********************************
    if (choice == NULL) {
        return NULL;
    } else if (choice->free == false) {
        this->start_invalidation(choice);
    }

    // ******************
    // Preenche a entrada
    // ******************
    choice->fill_entry(uop->opcode_address, uop->uop_id, uop->memory_address[0], 0, 0, 
                                        false, NULL, 0, false, orcs_engine.get_global_cycle(), false);
    

    return choice;

}


// Chama invalidação para todos os envolvidos
void table_of_stores_t::start_invalidation (table_of_stores_entry_t *entry) {

    if (!entry)
        return;
        


    // ***********************
    // Invalida outras tabelas
    // ***********************
    // Invalida TV
    if (entry->tv_entry) {
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        // Apenas invalida se estiver pre-vetorizando (indica que travou,
        // tipo com o laço tendo acabado dentro do ROB mesmo :p)
        // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        if (entry->tv_entry->state < 4) {
            printf("Store invalidation\n");
            this->tv->start_invalidation(entry->tv_entry);
        }
    } else  {
        if (entry->linked_tl_to) {
            if (entry->is_mov) {
                // Invalida TL
                this->tl->invalidate(this->tl->get_id(entry->tl_to_entry));
            } else {
                table_of_operations_entry_t *to_pointer = this->to->get_id(entry->tl_to_entry);
                // Invalida TL
                this->tl->invalidate(to_pointer->tl_entries[0]);
                this->tl->invalidate(to_pointer->tl_entries[1]);
    
                // Invalida TO
                this->to->invalidate(to_pointer);
            }
        }

        // Se invalida
        this->invalidate(entry);
    }


}

void table_of_stores_t::invalidate(table_of_stores_entry_t *entry) {
    if (!entry) return;

    // ***************
    // Limpa a entrada
    // ***************
    entry->clean();
}

// Busca na tabela de operações
table_of_stores_entry_t* table_of_stores_t::find (uop_package_t *uop) {
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_entry(uop)) {
            return &this->entries[i];
        }
    }
    return NULL;
}

// Busca na tabela de operações e retorna o id, ou found == false.
uint32_t table_of_stores_t::find_id (uop_package_t *uop, bool *found) { 
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_entry(uop)) {
            *found = true;
            return i;
        }
    }
    *found = false;
    return 0;
}

// Obtém entrada pelo id
table_of_stores_entry_t *table_of_stores_t::get_id (uint32_t id) {
    return &this->entries[id];
}


// Vetoriza a cadeia ou valida vetorização existente
// (A verificação foi feita ao construir, na função anterior)
void table_of_stores_t::vectorize (table_of_stores_entry_t *entry) {
    if (entry->tv_entry) {
        // Caso dê errado, é porque alguma invalidação esqueceu de apagar uma das duas :p
        assert(entry->tv_entry->ts_entry == entry);
        //this->tv->sequence_completed(entry->tv_entry); // Deveria ser chamado só no Rename
    } else {
        this->tv->new_pre_vectorization(entry);
    }

}

// Imprime tabela
void table_of_stores_t::print() {
        utils_t::largeSeparator();
        printf("TS contents (%u):\n", this->max_entries);
        for (uint32_t i=0; i<this->max_entries; ++i) {
                this->entries[i].print();
        }
}