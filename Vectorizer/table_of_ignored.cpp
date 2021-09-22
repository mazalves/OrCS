#include "./../simulator.hpp"

void table_of_ignored_t::allocate(libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_operations_t *to, table_of_stores_t *ts, vectorizer_t *vectorizer) {
    this->max_entries = vectorizer_configs["TI_SIZE"];
    this->entries = new table_of_ignored_entry_t[this->max_entries];
    this->occupied_entries = 0;

    this->tl = tl;
    this->to = to;
    this->ts = ts;

    this->vectorizer = vectorizer;
}

bool table_of_ignored_t::insert (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry, uint8_t structural_id) {
    assert (this->occupied_entries < this->max_entries);

    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_free()) {
            this->entries[i].set(addr, uop_id, tv_entry, structural_id);
            ++this->occupied_entries;
            return true;
        }
    }
    // Algo deu errado nas contagens :p
    return false;
}

bool table_of_ignored_t::remove (uint64_t addr, uint8_t uop_id) {
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


bool table_of_ignored_t::is_ignored (uint64_t addr, uint8_t uop_id) {
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_equal(addr, uop_id)) {
            return true;
        }
    }
    return false;
}

table_of_vectorizations_entry_t * table_of_ignored_t::get_tv_entry (uint64_t addr, uint8_t uop_id, uint8_t *structural_id) {
    for (uint32_t i=0; i < this->max_entries; ++i) {
        if (this->entries[i].is_equal(addr, uop_id)) {
            return this->entries[i].get_tv_entry(structural_id);
        }
    }
    return NULL;
}

bool table_of_ignored_t::insert_vectorization (table_of_vectorizations_entry_t *entry) {
        table_of_loads_entry_t *tl_entries[2];
        table_of_operations_entry_t *to_entry;

        if (entry->ts_entry->is_mov) {
            tl_entries[0] = this->tl->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[1] = NULL;
            to_entry      = NULL;

            if (!this->insert(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id(), entry, 0)) return false;
            this->vectorizer->statistics_counters[VECTORIZER_ASSIGNED_TO_IGNORED_INST] += 2;
        } else {
            to_entry      = this->to->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[0] = to_entry->tl_entries[0];
            tl_entries[1] = to_entry->tl_entries[1];

            if (!this->insert(to_entry->get_pc(), to_entry->get_uop_id(), entry, 2)) return false;
            if (!this->insert(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id(), entry, 0)) return false;
            if (!this->insert(tl_entries[1]->get_pc(), tl_entries[1]->get_uop_id(), entry, 1)) return false;
            this->vectorizer->statistics_counters[VECTORIZER_ASSIGNED_TO_IGNORED_INST] += 4;

        }

        if (!this->insert(entry->ts_entry->get_pc(), entry->ts_entry->get_uop_id(), entry, 3)) return false;
        return true;

}

void table_of_ignored_t::remove_vectorization (table_of_vectorizations_entry_t *entry) {
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
            printf("%p\n", (void *) tl_entries[0]);
            this->remove(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id());

        } else {
            to_entry      = this->to->get_id(entry->ts_entry->tl_to_entry);
            tl_entries[0] = to_entry->tl_entries[0];
            tl_entries[1] = to_entry->tl_entries[1];
            printf("%p\n", (void *) tl_entries[0]);
            this->remove(to_entry->get_pc(), to_entry->get_uop_id());
            this->remove(tl_entries[0]->get_pc(), tl_entries[0]->get_uop_id());
            this->remove(tl_entries[1]->get_pc(), tl_entries[1]->get_uop_id());
        }

        this->remove(entry->ts_entry->get_pc(), entry->ts_entry->get_uop_id());

}