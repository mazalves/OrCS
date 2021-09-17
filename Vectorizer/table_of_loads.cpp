#include "./../simulator.hpp"

// Deve ser chamado antes de qualquer execução
    void table_of_loads_t::allocate (libconfig::Setting &vectorizer_configs, table_of_operations_t *to, table_of_stores_t *ts, table_of_vectorizations_t *tv) {
        uint32_t size = vectorizer_configs["TL_SIZE"];
        assert (size > 0);

        this->entries = new table_of_loads_entry_t[size];
        for (uint32_t i=0; i<size; ++i) {
            this->entries[i].allocate(vectorizer_configs);
        }

        this->num_entries = 0;
        this->max_entries = size;

        this->to = to;
        this->ts = ts;
        this->tv = tv;
    }


table_of_loads_entry_t* table_of_loads_t::new_ld (uop_package_t *uop) {
    bool active_debug = false;
    //if (uop->opcode_address == 94318318040695) active_debug = true;
    //if (uop->opcode_address == 94318318040700) active_debug = true;
    if (active_debug) printf("Op: %lu (LOAD)\n", uop->opcode_address);


    // ********************************
    // Não tenta vetorizar gathers
    // ********************************
    if (uop->num_mem_operations > 1) {
        if (active_debug) printf(" Return by gather\n");
        return NULL;
    }

    // ********************************
    // Procura na tabela
    // ********************************
    table_of_loads_entry_t *entry = NULL;
    entry = this->find(uop);

    // ********************************
    // Se não encontrar, adiciona na TL
    // ********************************
    if (entry == NULL) {
        entry = this->add_tl (uop);
        if (active_debug)  {
            if (entry) printf(" Added to TL\n");
            else printf("Não foi possível alocar entrada na TL!\n");
        }
    } else {
        // *************
        // Valida o load
        // *************
        if (entry->confidence_counter == 0) {
            entry->stride = uop->memory_address[0] - entry->last_address;
            entry->last_address = uop->memory_address[0];
            entry->inc_confidence_counter(orcs_engine.get_global_cycle()); // Característica do 1 é ter um stride
            if (active_debug) printf(" Setting first stride\n");

        } else {
            int32_t stride = uop->memory_address[0] - entry->last_address;
            // -----
            // Falha
            // -----
            if (stride != entry->stride) {
            if (active_debug) printf(" Invalid stride (Old: %d; New: %d)\n", entry->stride, stride);

                // +++++++++++++++++++++++++++++++++++++++++
                // Invalida vetorização, ou link, se existir
                // +++++++++++++++++++++++++++++++++++++++++
                this->start_invalidation(entry);

                // +++++++++++++++++++
                // Reinicia o Tracking
                // +++++++++++++++++++
                entry->fill_entry(uop->opcode_address, uop->uop_id, uop->memory_address[0], 
                        0, 0, 0, 0, false, NULL, orcs_engine.get_global_cycle(), false, false);
                entry->reset_confidence_counter(orcs_engine.get_global_cycle());
                entry->vectorizable = false;
                entry->stride = stride;
                entry->inc_confidence_counter(orcs_engine.get_global_cycle());
            }
            // -------
            // Sucesso
            // -------
            else {
                if (active_debug) printf(" Valid stride\n");
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

    return entry;
}




// Adiciona na tabela de loads
table_of_loads_entry_t* table_of_loads_t::add_tl (uop_package_t *uop) {
    // **********************************************************
    // Busca entrada livre ou LRU
    // Apenas seleciona aquelas sem vetorização ou em treinamento
    // **********************************************************
    table_of_loads_entry_t *choice = NULL;
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
    } else if (choice->free) {
        this->num_entries++;
    } else {
        this->start_invalidation(choice);
    }

    // ******************
    // Preenche a entrada
    // ******************
    choice->fill_entry(uop->opcode_address, uop->uop_id, uop->memory_address[0], 
                        0, 0, 0, 0, false, NULL, orcs_engine.get_global_cycle(), false, false);
    

    return choice;

}

// Busca na tabela de loads
table_of_loads_entry_t* table_of_loads_t::find (uop_package_t *uop) {

    table_of_loads_entry_t *entry = NULL;
    for (uint32_t i = 0; i < this->max_entries; ++i) {
        if (this->entries[i].is_entry(uop)) {
            entry = &this->entries[i];
        }
    }
    return entry;
}

table_of_loads_entry_t* table_of_loads_t::get_id(uint32_t id) {
    return &this->entries[id];
}


// Chama invalidação para todos os envolvidos
void table_of_loads_t::start_invalidation (table_of_loads_entry_t *entry) {
    
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
            printf("Load invalidation\n");
            this->tv->start_invalidation(entry->tv_entry);
        }
    } else  {
        if (entry->linked_to_ts) {
            if (entry->is_mov) {
                // Invalida na TS
                table_of_stores_entry_t *ts_pointer = ts->get_id(entry->to_ts_entry);
                ts->invalidate(ts_pointer);

            } else {
                table_of_operations_entry_t *to_pointer = to->get_id(entry->to_ts_entry);

                // Invalida na TS
                if (to_pointer->ts_entry) {
                    this->ts->invalidate(to_pointer->ts_entry);
                }

                // Invalida na TO
                to->invalidate(to_pointer);
            }
        }

        // Se invalida
        this->invalidate(entry);
    }

}

void table_of_loads_t::invalidate(table_of_loads_entry_t *entry){
    
    if (!entry)
        return;

    // ***************
    // Limpa a entrada
    // ***************
    entry->clean();

}

// Imprime tabela
void table_of_loads_t::print() {
        printf("TL contents (%u/%u):\n", this->num_entries, this->max_entries);
        for (uint32_t i=0; i<this->max_entries; ++i) {
                this->entries[i].print();
    }
}

// Return id from entry
uint32_t table_of_loads_t::entry_to_id(table_of_loads_entry_t *entry) {
    uint32_t id = ((uint64_t)entry - (uint64_t)this->entries) / sizeof(table_of_loads_entry_t);
    assert(&this->entries[id] == entry);
    return id;
}