#include "./../simulator.hpp"

// Deve ser chamado antes de qualquer execução
void table_of_operations_t::allocate (libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_stores_t *ts, table_of_vectorizations_t *tv, vectorizer_t *vectorizer) { 
       uint32_t size = vectorizer_configs["TO_SIZE"];
        assert (size > 0);

        this->entries = new table_of_operations_entry_t[size];
        this->max_entries = size;
        this->allocated_entries = 0;

        this->tl = tl;
        this->ts = ts;
        this->tv = tv;

        this->vectorizer = vectorizer;
    
}
        
void table_of_operations_t::new_op (uop_package_t *uop) {
    // Ajusta o LRU
    table_of_operations_entry_t *entry = this->find(uop);
    if (entry) {
        entry->lru = orcs_engine.get_global_cycle();
    }

    //this->print();

}

void table_of_operations_t::start_invalidation (table_of_operations_entry_t *entry){ 
    
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
            //printf("Operation invalidation\n");
            this->tv->start_invalidation(entry->tv_entry);
            this->vectorizer->statistics_counters[VECTORIZER_TO_STARTED_INVALIDATION]++;
            this->vectorizer->statistics_counters[VECTORIZER_TO_INVALIDATED_TRAINING]++;

        }
    } else  {
         // Invalida TL
        this->tl->invalidate(entry->tl_entries[0]);
        this->tl->invalidate(entry->tl_entries[1]);


        // Invalida TS
        if (entry->ts_entry) {
            this->ts->invalidate(entry->ts_entry);
        }

        // Se invalida
        this->invalidate(entry);
        this->vectorizer->statistics_counters[VECTORIZER_TO_STARTED_INVALIDATION]++;

    }

}

void table_of_operations_t::invalidate(table_of_operations_entry_t *entry){ 
    if (!entry) 
        return;

    // ***************
    // Limpa a entrada
    // ***************
    entry->clean();
    this->vectorizer->statistics_counters[VECTORIZER_INVALIDATION_TO]++;

}

// Busca na tabela de operações
table_of_operations_entry_t* table_of_operations_t::find (uop_package_t *uop) { 
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_entry(uop)) {
            return &this->entries[i];
        }
    }
     
    return NULL; 
}

 // Busca na tabela de operações e retorna o id, ou found == false.
 uint32_t table_of_operations_t::find_id (uop_package_t *uop, bool *found)  { 
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_entry(uop)) {
            *found = true;
            return i;
        }
    }
    *found = false;
    return 0;
}

 table_of_operations_entry_t* table_of_operations_t::get_id (uint32_t id) { 
    return &this->entries[id];
}


// Tenta criar uma nova entrada e retorna seu id, caso contrário informa que é inválida
uint32_t table_of_operations_t::new_entry_id (uop_package_t *uop, bool *valid) {
    this->allocated_entries++;
    // **********************************************************
    // Busca entrada livre ou LRU
    // Apenas seleciona aquelas sem vetorização ou em treinamento
    // **********************************************************
    table_of_operations_entry_t *choice = NULL;
    uint32_t choice_id = 0;
    *valid = false;
    for (uint32_t i=0; i < this->max_entries; ++i) {
        // Tenta pegar qualquer um sem vetorização ou em treinamento (qualquer possível)
        if (!choice) {
            if (this->entries[i].tv_entry == NULL) {
                choice = &this->entries[i];
                choice_id = i;
            } else if (this->entries[i].tv_entry->state < 4) {
                choice = &this->entries[i];
                choice_id = i;
            }
        } else { /* Se já tiver uma escolha */
            //  Tenta trocar uma  vetorizada por uma livre
            if ((choice->tv_entry) && (this->entries[i].tv_entry == NULL)) {
                choice = &this->entries[i];
                choice_id = i;
            } else if (choice->tv_entry) { /* Se não se encaixar nisso e ambos estiverem vetorizados, tenta avaliar o LRU */
                /* Só avalia o LRU se puder (a outra estiver treinando) */
                if (this->entries[i].tv_entry->state < 4) {
                    if (this->entries[i].lru < choice->lru) {
                        choice = &this->entries[i];
                        choice_id = i;

                    }
                }
            } else {
                /* Só avalia o LRU se ambos não forem vetorizados, já que pegar um vetorizado seria pior */
                if (this->entries[i].tv_entry == NULL) {
                    if (this->entries[i].lru < choice->lru) {
                        choice = &this->entries[i];
                        choice_id = i;
                    }
                }
            }
        }

        /* No melhor dos casos, pega uma entrada livre */
        if (this->entries[i].free) {
            choice = &this->entries[i];
            choice_id = i;
            break;
        }
    }

    // *************************************
    // Indica que uma entrada não foi obtida
    // *************************************
    if (choice == NULL) {
        *valid = false;
        this->vectorizer->statistics_counters[VECTORIZER_TO_NOT_ENOUGH_ENTRIES]++;
        return 0;
    } else if (choice->free == false) {
        this->start_invalidation(choice);
    }

    // ******************
    // Preenche a entrada
    // ******************
    choice->fill_entry(uop->opcode_address, uop->uop_id, uop->opcode_assembly,
                      NULL, NULL, NULL, NULL, orcs_engine.get_global_cycle());

    //printf("[ TO ] Alocando entrada %u\n", choice_id);
    *valid = true;

    assert (&this->entries[choice_id] == choice);
    return choice_id;
}

// Imprime tabela
void table_of_operations_t::print() {
    uint32_t n_vectorizable = 0;

    for (uint32_t i=0; i<this->max_entries; ++i) {
        if (this->entries[i].get_pc() != 0x0) n_vectorizable++;
    }

    if (n_vectorizable >= 1) {
        utils_t::largeSeparator();
        printf("TO contents (%u/%u):\n", n_vectorizable, this->max_entries);
        for (uint32_t i=0; i<this->max_entries; ++i) {
            if (this->entries[i].get_pc() != 0x0) {
                this->entries[i].print();
            }
        }
    }
}

// Return id from entry
uint32_t table_of_operations_t::entry_to_id(table_of_operations_entry_t *entry) {
    uint32_t id = ((uint64_t)entry - (uint64_t)this->entries) / sizeof(table_of_operations_entry_t);
    assert(&this->entries[id] == entry);
    return id;
}

void table_of_operations_t::check_for_nulls() {
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].free == false) {
            assert (this->entries[i].tl_entries[0]);
        }
    }
}