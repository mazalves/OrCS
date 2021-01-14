#include "./../simulator.hpp"

table_of_loads_entry_t *table_of_loads_t::add_pc (uint64_t pc, uint64_t addr) {
    // Aloca
    int32_t tl_id = this->next_replacement;
    this->next_replacement += 1;
    if(this->next_replacement == entries_size)
    {
    	this->next_replacement = 0;
    }

    table_of_loads_entry_t *tl_entry = &this->entries[tl_id];

    // Preenche
    tl_entry->pc = pc;
    tl_entry->last_address = addr;
    tl_entry->stride = 0;
    tl_entry->confidence = 0;

    return tl_entry;

}

table_of_loads_entry_t *table_of_loads_t::find_pc (uint64_t pc) {
    for (int32_t i = 0; i < this->entries_size; ++i) {
        if (this->entries[i].pc == pc)
            return &this->entries[i];

    }
    return NULL;

}

void table_of_loads_t::update_stride (table_of_loads_entry_t *tl_entry, uint64_t addr) {
    if (tl_entry->confidence == 0) {
	    tl_entry->stride = addr - tl_entry->last_address;
	    tl_entry->confidence = 1;
    } else {
    	int32_t new_stride =  addr - tl_entry->last_address;
    	if (new_stride != tl_entry->stride) {
    		tl_entry->confidence = 1;
    		tl_entry->stride = new_stride;
    	} else {
            tl_entry->confidence = 2;
        }
    }

    tl_entry->last_address = addr;

}

table_of_loads_t::table_of_loads_t  (int32_t num_entries) {
    this->entries_size = num_entries;
    this->entries = new table_of_loads_entry_t [this->entries_size];
    this->next_replacement = 0;

}

table_of_loads_t::~table_of_loads_t  () {
    delete[] this->entries;

}