#include "./../simulator.hpp"

void table_of_pre_vectorization_t::allocate(libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_operations_t *to, table_of_stores_t *ts, vectorizer_t *vectorizer) {
    this->max_entries = vectorizer_configs["TPV_SIZE"];
    this->entries = new table_of_pre_vectorization_entry_t[this->max_entries];
    this->occupied_entries = 0;

    this->tl = tl;
    this->to = to;
    this->ts = ts;

    this->vectorizer = vectorizer;
}

bool table_of_pre_vectorization_t::insert (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry) {
    assert (this->occupied_entries < this->max_entries);
    
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_free()) {
            this->entries[i].set(addr, uop_id, tv_entry);
            ++this->occupied_entries;
            return true;
        }
    }
    // Algo deu errado nas contagens :p
    return false;
}

bool table_of_pre_vectorization_t::remove (uint64_t addr, uint8_t uop_id) {
    if (this->occupied_entries == 0)
       return false;

    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_equal(addr, uop_id)) {
            this->entries[i].clean();
            assert (this->occupied_entries > 0);
            --this->occupied_entries;
            return true;
        }
    }
    // Algo deu errado nas contagens :p
    return false;
        
}


table_of_vectorizations_entry_t* table_of_pre_vectorization_t::get_tv_entry (uint64_t addr, uint8_t uop_id) {
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_equal(addr, uop_id)) {
            return this->entries[i].get_tv();
        }
    }
    return NULL;
}


// Retira as instruções correpondentes à vetorização dessa tabela
// Um indício de que terminou o treinamento, tivemos uma invalidação no meio dele :p
void table_of_pre_vectorization_t::remove_vectorization (table_of_vectorizations_entry_t *entry) {
    table_of_loads_entry_t *tl_entries[2];
    table_of_operations_entry_t *to_entry;

    if (entry->ts_entry == NULL) {
        assert (entry->discard_results);
        return;
    }

    if (entry->ts_entry->is_mov) {
        tl_entries[0] = this->tl->get_id(entry->ts_entry->tl_to_entry);
        tl_entries[1] = NULL;
        to_entry      = NULL;

        this->remove(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id());
    } else {
        to_entry      = this->to->get_id(entry->ts_entry->tl_to_entry);
        tl_entries[0] = to_entry->tl_entries[0];
        tl_entries[1] = to_entry->tl_entries[1];

        this->remove(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id());
        this->remove(tl_entries[1]->get_pc(), tl_entries[1]->get_uop_id());
    }

    this->remove(entry->ts_entry->get_pc(), entry->ts_entry->get_uop_id());

}
// ############################################################################################################

void table_of_pre_vectorization_t::print() {
    utils_t::largeSeparator(stdout);
    printf("Table of pre-vectorizations\n");
    utils_t::largeSeparator(stdout);
    for (uint32_t i=0; i < this->max_entries; ++i) {
        this->entries[i].print();
    }
}