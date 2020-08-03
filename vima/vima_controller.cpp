#include "./../simulator.hpp"
#include <string>

vima_controller_t::vima_controller_t(){
    this->i = 0;
    this->current_index = 0;

    this->read1 = NULL;
    this->read2 = NULL;
    this->write = NULL;

    this->lines = 0;
    this->sets = 0;

    this->cache = NULL;
    this->vima_op_latencies = NULL;
    this->current_cache_access_latency = 0;

    this->index_bits_mask = 0;
    this->index_bits_shift = 0;

    this->tag_bits_mask = 0;
    this->tag_bits_shift = 0;

    this->cache_hits = 0;
    this->cache_misses = 0;
    this->cache_accesses = 0;
    this->cache_writebacks = 0;

    this->VIMA_BUFFER = 0;
    this->VIMA_VECTOR_SIZE = 0;
    this->VIMA_DEBUG = 0;
    this->VIMA_CACHE_ASSOCIATIVITY = 0;
    this->VIMA_CACHE_LATENCY = 0;
    this->VIMA_CACHE_SIZE = 0;
    this->VIMA_UNBALANCED = 0;
    this->CORE_TO_BUS_CLOCK_RATIO = 0.0;
}

vima_controller_t::~vima_controller_t(){
    ORCS_PRINTF ("#========================================================================#\n")
    ORCS_PRINTF ("VIMA Cache Hits: %lu\n", get_cache_hits())
    ORCS_PRINTF ("VIMA Cache Misses: %lu\n", get_cache_misses())
    ORCS_PRINTF ("VIMA Cache Accesses: %lu\n", get_cache_accesses())
    ORCS_PRINTF ("VIMA Cache Writebacks: %lu\n", get_cache_writebacks())
    ORCS_PRINTF ("VIMA Cache Associativity: %u\n", get_VIMA_CACHE_ASSOCIATIVITY())
    ORCS_PRINTF ("VIMA Cache Lines: %u\n", this->get_lines())
    ORCS_PRINTF ("VIMA Cache Sets: %u\n", this->get_sets())
    ORCS_PRINTF ("#========================================================================#\n")

    for (i = 0; i < sets; i++) delete[] this->cache[i];
    delete[] cache;
    delete[] vima_op_latencies;
}

void vima_controller_t::print_vima_instructions(){
    ORCS_PRINTF ("=======VIMA INSTRUCTIONS=========\n")
    for (size_t i = 0; i < vima_buffer.size(); i++){
        ORCS_PRINTF ("uop %lu %s readyAt %lu status %s\n", vima_buffer[i]->uop_number, get_enum_memory_operation_char (vima_buffer[i]->memory_operation), vima_buffer[i]->readyAt, get_enum_package_state_char (vima_buffer[i]->status))
    }
    ORCS_PRINTF ("=================================\n")
}

void vima_controller_t::instruction_ready (size_t index){
    if (VIMA_DEBUG) {
        ORCS_PRINTF ("VIMA Controller clock(): instruction %lu, %s ready at cycle %lu.\n", vima_buffer[index]->uop_number, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->readyAt)
    }

    if (vima_buffer[index]->vima_write != 0) this->write_to_cache (index);

    //ORCS_PRINTF ("%lu VIMA request from processor %u READY\n", vima_buffer.size(), vima_buffer[index]->processor_id)
    vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[index]), vima_buffer.end());
}

vima_vector_t* vima_controller_t::search_cache (uint64_t address){
    add_cache_accesses();
    uint64_t lru_cycle = UINT64_MAX;
    uint32_t lru_way = 0;
    uint32_t index = 0;
    this->current_cache_access_latency += this->get_VIMA_CACHE_LATENCY();
    if (VIMA_CACHE_ASSOCIATIVITY == 1){
        for (uint32_t i = 0; i < get_lines(); i++){
            if (get_index(cache[i][0].get_address()) == get_index (address) && get_tag(cache[i][0].get_address()) == get_tag (address)) {
                add_cache_hits();
                //ORCS_PRINTF ("VIMA CACHE HIT! Address %lu was found.\n", address)
                return &cache[i][0];
            }
            else if (cache[i][0].lru < lru_cycle) {
                lru_cycle = cache[i][0].lru;
                lru_way = i;
            }
        }
    } else {
        index = get_index (address);
        for (i = 0; i < VIMA_CACHE_ASSOCIATIVITY; i++){
            if (get_tag(cache[index][i].get_address()) == get_tag (address)) {
		        add_cache_hits();
                //ORCS_PRINTF ("VIMA CACHE HIT! Address %lu was found.\n", address)
                return &cache[index][i];
            }
            else if (cache[index][i].lru < lru_cycle) {
                lru_cycle = cache[index][i].lru;
                lru_way = i;
            }
        }
    }
    add_cache_misses();
    //if (lru_cycle != 0) ORCS_PRINTF ("EVICTION! ")
    //ORCS_PRINTF ("VIMA CACHE MISS! Address %lu will be fetched.\n", address)
    if (VIMA_CACHE_ASSOCIATIVITY != 1) return &cache[index][lru_way];
    else return &cache[lru_way][0];
}

void vima_controller_t::check_completion (int index){
    if (vima_buffer[index]->vima_read1 != 0){
        if (read1->status != PACKAGE_STATE_READY) return;
        if (VIMA_UNBALANCED && read1_unbalanced != NULL){
            if (read1_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }

    if (vima_buffer[index]->vima_read2 != 0){
        if (read2->status != PACKAGE_STATE_READY) return;
        if (VIMA_UNBALANCED && read2_unbalanced != NULL){
            if (read2_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }
    vima_buffer[index]->updatePackageWait (this->vima_op_latencies[vima_buffer[index]->memory_operation]);

}

void vima_controller_t::write_to_cache (int index) {
    write = search_cache (vima_buffer[index]->vima_write);
    if (write->status == PACKAGE_STATE_FREE) write->status = PACKAGE_STATE_WAIT;
    else {
        add_cache_writebacks();
        write->status = PACKAGE_STATE_TRANSMIT;
    }
    write->set_address (vima_buffer[index]->vima_write);
    write->set_tag (get_tag (vima_buffer[index]->vima_write));    
    write->set_lru (orcs_engine.get_global_cycle());
    write->dirty = true;
    working_vectors.push_back (write);
    if (VIMA_UNBALANCED && (get_index(vima_buffer[index]->vima_write) != get_index(vima_buffer[index]->vima_write + VIMA_VECTOR_SIZE -1))) {
        write_unbalanced = search_cache (vima_buffer[index]->vima_write + VIMA_VECTOR_SIZE -1);
        if (write_unbalanced->status == PACKAGE_STATE_FREE) write_unbalanced->status = PACKAGE_STATE_WAIT;
        else {
            add_cache_writebacks();
            write_unbalanced->status = PACKAGE_STATE_TRANSMIT;
        }
        write_unbalanced->set_address (vima_buffer[index]->vima_write + VIMA_VECTOR_SIZE -1);
        write_unbalanced->set_tag (get_tag (vima_buffer[index]->vima_write + VIMA_VECTOR_SIZE -1));    
        write_unbalanced->set_lru (orcs_engine.get_global_cycle());
        write_unbalanced->dirty = true;
        working_vectors.push_back (write_unbalanced);
    }
}

void vima_controller_t::check_cache (int index) {
    if (vima_buffer[index]->vima_read1 != 0) {
        read1 = search_cache (vima_buffer[index]->vima_read1);
        if (read1->status == PACKAGE_STATE_FREE) read1->status = PACKAGE_STATE_WAIT;
        else {
            add_cache_writebacks();
            read1->status = PACKAGE_STATE_TRANSMIT;
        }
        read1->set_address (vima_buffer[index]->vima_read1);
        read1->set_tag (get_tag (vima_buffer[index]->vima_read1));    
        read1->set_lru (orcs_engine.get_global_cycle());
        working_vectors.push_back (read1);
        if (VIMA_UNBALANCED && (get_index(vima_buffer[index]->vima_read1) != get_index(vima_buffer[index]->vima_read1 + VIMA_VECTOR_SIZE -1))) {
            read1_unbalanced = search_cache (vima_buffer[index]->vima_read1 + VIMA_VECTOR_SIZE -1);
            if (read1_unbalanced->status == PACKAGE_STATE_FREE) read1_unbalanced->status = PACKAGE_STATE_WAIT;
            else {
                add_cache_writebacks();
                read1_unbalanced->status = PACKAGE_STATE_TRANSMIT;
            }
            read1_unbalanced->set_address (vima_buffer[index]->vima_read1 + VIMA_VECTOR_SIZE -1);
            read1_unbalanced->set_tag (get_tag (vima_buffer[index]->vima_read1 + VIMA_VECTOR_SIZE -1));    
            read1_unbalanced->set_lru (orcs_engine.get_global_cycle());
            working_vectors.push_back (read1_unbalanced);
        }
    }
    if (vima_buffer[index]->vima_read2 != 0) {
        read2 = search_cache (vima_buffer[index]->vima_read2);
        if (read2->status == PACKAGE_STATE_FREE) read2->status = PACKAGE_STATE_WAIT;
        else {
            add_cache_writebacks();
            read2->status = PACKAGE_STATE_TRANSMIT;
        }
        read2->set_address (vima_buffer[index]->vima_read2);
        read2->set_tag (get_tag (vima_buffer[index]->vima_read2));    
        read2->set_lru (orcs_engine.get_global_cycle());
        working_vectors.push_back (read2);
        if (VIMA_UNBALANCED && (get_index(vima_buffer[index]->vima_read2) != get_index(vima_buffer[index]->vima_read2 + VIMA_VECTOR_SIZE - 1))) {
            read2_unbalanced = search_cache (vima_buffer[index]->vima_read2 + VIMA_VECTOR_SIZE -1);
            if (read2_unbalanced->status == PACKAGE_STATE_FREE) read2_unbalanced->status = PACKAGE_STATE_WAIT;
            else {
                add_cache_writebacks();
                read2_unbalanced->status = PACKAGE_STATE_TRANSMIT;
            }
            read2_unbalanced->set_address (vima_buffer[index]->vima_read2 + VIMA_VECTOR_SIZE - 1);
            read2_unbalanced->set_tag (get_tag (vima_buffer[index]->vima_read2 + VIMA_VECTOR_SIZE - 1));    
            read2_unbalanced->set_lru (orcs_engine.get_global_cycle());
            working_vectors.push_back (read2_unbalanced);
        }
    }

    vima_buffer[index]->updatePackageTransmit(this->current_cache_access_latency);
    this->current_cache_access_latency = 0;
}

void vima_controller_t::clock(){
    for (size_t i = 0; i < working_vectors.size(); i++){
        if (working_vectors[i]->status != PACKAGE_STATE_READY) working_vectors[i]->clock();
        else working_vectors.erase (std::remove (working_vectors.begin(), working_vectors.end(), working_vectors[i]), working_vectors.end());
    }

    if (vima_buffer.size() <= 0) return;
    switch (vima_buffer[current_index]->status){
        case PACKAGE_STATE_WAIT:
            //ORCS_PRINTF ("OUT VIMA %lu %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[current_index]->memory_operation), vima_buffer[current_index]->uop_number, vima_buffer[current_index]->processor_id)
            //if (vima_buffer[current_index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[current_index]->vima_read1)
            //if (vima_buffer[current_index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[current_index]->vima_read2)
            //if (vima_buffer[current_index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[current_index]->vima_write)
            //ORCS_PRINTF ("\n")
            this->instruction_ready (0);
            break;
        case PACKAGE_STATE_TRANSMIT:
            this->check_completion(0);
            break;
        case PACKAGE_STATE_VIMA:
            //ORCS_PRINTF ("IN  VIMA %lu %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[current_index]->memory_operation), vima_buffer[current_index]->uop_number, vima_buffer[current_index]->processor_id)
            //if (vima_buffer[current_index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[current_index]->vima_read1)
            //if (vima_buffer[current_index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[current_index]->vima_read2)
            //if (vima_buffer[current_index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[current_index]->vima_write)
            //ORCS_PRINTF ("\n")
            this->check_cache(0);
            break;
        default:
            ///ORCS_PRINTF ("%lu %s -> \n", vima_buffer[current_index]->uop_number, get_enum_package_state_char (vima_buffer[current_index]->status))
            vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[current_index]), vima_buffer.end());
            break;
    }
}

void vima_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["VIMA_CONTROLLER"];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_VIMA_BUFFER (cfg_processor["VIMA_BUFFER"]);
    set_VIMA_DEBUG (cfg_processor["VIMA_DEBUG"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    set_VIMA_CACHE_SIZE (cfg_processor["VIMA_CACHE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_processor["VIMA_VECTOR_SIZE"]);
    set_VIMA_CACHE_ASSOCIATIVITY (cfg_processor["VIMA_CACHE_ASSOCIATIVITY"]);
    set_VIMA_CACHE_LATENCY (cfg_processor["VIMA_CACHE_LATENCY"]);
    set_VIMA_UNBALANCED (cfg_processor["VIMA_UNBALANCED"]);

    this->set_lines (this->get_VIMA_CACHE_SIZE()/this->get_VIMA_VECTOR_SIZE());
    this->set_sets (lines/this->get_VIMA_CACHE_ASSOCIATIVITY());

    read1 = NULL;
    read1_unbalanced = NULL;
    read2 = NULL;
    read2_unbalanced = NULL;
    write = NULL;
    write_unbalanced = NULL;

    this->cache = new vima_vector_t*[sets]();
    for (uint32_t i = 0; i < sets; i++){
        this->cache[i] = new vima_vector_t[VIMA_CACHE_ASSOCIATIVITY]();
        for (size_t j = 0; j < VIMA_CACHE_ASSOCIATIVITY; j++) this->cache[i][j].allocate();
    }
    
    this->index_bits_shift = utils_t::get_power_of_two(this->get_VIMA_VECTOR_SIZE());
    this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(sets);

    uint64_t i;
    /// INDEX MASK
    for (i = 0; i < utils_t::get_power_of_two(sets); i++) {
        this->index_bits_mask |= 1 << (i + index_bits_shift);
    }

    /// TAG MASK
    for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->tag_bits_mask |= 1 << i;
    }

    vima_op_latencies = new uint32_t[MEMORY_OPERATION_LAST]();
    
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = cfg_processor["VIMA_LATENCY_INT_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = cfg_processor["VIMA_LATENCY_INT_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = cfg_processor["VIMA_LATENCY_INT_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = cfg_processor["VIMA_LATENCY_FP_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = cfg_processor["VIMA_LATENCY_FP_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = cfg_processor["VIMA_LATENCY_FP_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] = cfg_processor["VIMA_LATENCY_INT_MLA"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] = cfg_processor["VIMA_LATENCY_FP_MLA"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] * this->CORE_TO_BUS_CLOCK_RATIO);

    set_cache_accesses(0);
    set_cache_hits(0);
    set_cache_misses(0);
}

bool vima_controller_t::addRequest (memory_package_t* request){
    if (vima_buffer.size() < this->VIMA_BUFFER) {
        request->sent_to_ram = true;
        request->status = PACKAGE_STATE_VIMA;
        vima_buffer.push_back (request);
        //ORCS_PRINTF ("%lu %lu NEW VIMA request from processor %u\n", orcs_engine.get_global_cycle(), vima_buffer.size(), request->processor_id)
        return true;
    } else {
        request->sent_to_cache = false;
        request->sent_to_ram = false;
        if (VIMA_DEBUG) ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
    }
    return false;
}
