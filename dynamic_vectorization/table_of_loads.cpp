#include "./../simulator.hpp"

table_of_loads_entry_t *table_of_loads_t::add_pc (uint64_t pc, uint64_t addr) {
    //=========================
    // Find oldest entry
    //=========================
    // Aloca
    uint32_t oldest_last_use = 0;
    uint32_t oldest_id = 0;

    table_of_loads_entry_t *possible_entries = this->entries.map_set(pc);
    oldest_last_use = possible_entries[0].last_use;
    oldest_id = 0;

    for (uint32_t i=1; i < this->entries.get_associativity(); ++i) {
        if (oldest_last_use > possible_entries[i].last_use) {
            oldest_last_use = possible_entries[i].last_use;
            oldest_id = i;
        }
    }

    //=========================
    //Fill entry
    //=========================

    table_of_loads_entry_t *tl_entry = &possible_entries[oldest_id];

    // Preenche
    tl_entry->pc = pc;
    tl_entry->last_address = addr;
    tl_entry->stride = 0;
    tl_entry->confidence = 0;
    tl_entry->last_use = orcs_engine.get_global_cycle();

    return tl_entry;

}

table_of_loads_entry_t *table_of_loads_t::find_pc (uint64_t pc) {

    //=========================
    // Get set
    //=========================
    table_of_loads_entry_t* possible_entries = this->entries.map_set(pc);


    //=========================
    // Search for the element
    //=========================
    for (uint32_t i=0; i < this->entries.get_associativity(); ++i) {
        if (possible_entries[i].pc == pc) {
            return &possible_entries[i];
        }
    }

    //=========================
    // Element not found
    //=========================
    return NULL;

}

void table_of_loads_t::update_stride (table_of_loads_entry_t *tl_entry, uint64_t addr) {
    if (tl_entry->confidence == 0) {
	    tl_entry->stride = addr - tl_entry->last_address;
        // Verifica se stride ultrapassa o máximo
        if (MAX_LOAD_STRIDE > -1 && abs(tl_entry->stride) > MAX_LOAD_STRIDE) {
            vectorizer->stride_greater_than_max++;
            tl_entry->confidence = 0;
        } else {
            tl_entry->confidence = 1;
        }
	    
    } else {
    	int32_t new_stride = addr - tl_entry->last_address;
    	if (new_stride != tl_entry->stride) {
            vectorizer->stride_changed++;
            tl_entry->stride = new_stride;
            
    		// Verifica se novo stride ultrapassa o máximo
            if (MAX_LOAD_STRIDE > -1 && abs(tl_entry->stride) > MAX_LOAD_STRIDE) {
                vectorizer->stride_greater_than_max++;
                tl_entry->confidence = 0;
            } else {
                tl_entry->confidence = 1;
            }
    	} else {
            vectorizer->stride_confirmed++;
            tl_entry->confidence = 2;
        }
    }

    if (VECTORIZATION_ENABLED == 0) {
        tl_entry->confidence = 0;
    }

    if (VECTORIZE_AFTER > orcs_engine.get_global_cycle()) {
        tl_entry->confidence = 0;
    }
    
    tl_entry->last_address = addr;
}

table_of_loads_t::table_of_loads_t (uint32_t num_entries, uint32_t associativity, Vectorizer_t *vectorizer) {
    printf("table_of_loads_t::constructor\n");
    this->vectorizer = vectorizer;
    this->entries.allocate(num_entries, associativity, 0);

}

table_of_loads_t::~table_of_loads_t  () {
   //this->list_contents();
}

void table_of_loads_t::list_contents() {
    printf("TL contents:\n");
    uint32_t n_sets = this->entries.get_num_sets();
    uint32_t associativity = this->entries.get_associativity();

    for (uint32_t i=0; i < n_sets; ++i) {
        printf("Set %u: ", i);
        for (uint32_t j=0; j < associativity; ++j) {
            printf(" %lu[%lu] ", this->entries[i][j].pc, this->entries[i][j].last_use);
        }
        printf("\n");
    }
}
