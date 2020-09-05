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

    this->cache_reads = 0;
    this->cache_writes = 0;
    this->cache_hits = 0;
    this->cache_misses = 0;
    this->cache_accesses = 0;
    this->cache_writebacks = 0;
    
    this->request_count = 0;
    this->total_wait = 0;

    this->VIMA_BUFFER = 0;
    this->VIMA_VECTOR_SIZE = 0;
    this->VIMA_CACHE_ASSOCIATIVITY = 0;
    this->VIMA_CACHE_LATENCY = 0;
    this->VIMA_CACHE_SIZE = 0;
    this->VIMA_UNBALANCED = 0;
    this->CORE_TO_BUS_CLOCK_RATIO = 0.0;
}

vima_controller_t::~vima_controller_t(){
    for (i = 0; i < sets; i++) delete[] this->cache[i];
    delete[] cache;
    delete[] vima_op_latencies;
}

void vima_controller_t::statistics(){
    if (this->get_cache_accesses() > 0){
        ORCS_PRINTF ("#==============VIMA Controller==========================================#\n")
        ORCS_PRINTF ("VIMA_Cache_Reads:                  %lu\n", get_cache_reads())
        ORCS_PRINTF ("VIMA_Cache_Writes:                 %lu\n", get_cache_writes())
        ORCS_PRINTF ("VIMA_Cache_Hits:                   %lu\n", get_cache_hits())
        ORCS_PRINTF ("VIMA_Cache_Misses:                 %lu\n", get_cache_misses())
        ORCS_PRINTF ("VIMA_Cache_Accesses:               %lu\n", get_cache_accesses())
        ORCS_PRINTF ("VIMA_Cache_Writebacks:             %lu\n", get_cache_writebacks())
        ORCS_PRINTF ("VIMA_Cache_Associativity:          %u\n", get_VIMA_CACHE_ASSOCIATIVITY())
        ORCS_PRINTF ("VIMA_Cache_Lines:                  %u\n", this->get_lines())
        ORCS_PRINTF ("VIMA_Cache_Sets:                   %u\n", this->get_sets())

        uint64_t total_fetch_latency = 0;
        uint64_t total_fetch_count = 0;
        uint64_t total_writeback_latency = 0;
        uint64_t total_writeback_count = 0;
        for (i = 0; i < sets; i++) {
            total_fetch_count += this->cache[i]->fetch_count;
            total_fetch_latency += this->cache[i]->fetch_latency_total;
            total_writeback_count += this->cache[i]->writeback_count;
            total_writeback_latency += this->cache[i]->writeback_latency_total;
        }

        if (total_fetch_count != 0) ORCS_PRINTF ("VIMA_Cache_Avg._Fetch_Latency:     %lu\n", total_fetch_latency/total_fetch_count)
        if (total_writeback_count != 0) ORCS_PRINTF ("VIMA_Cache_Avg._Writeback_Latency: %lu\n", total_writeback_latency/total_writeback_count)
        if (this->request_count > 0) ORCS_PRINTF ("VIMA_Controller_Avg._Wait:         %lu\n", this->total_wait/this->request_count)
        ORCS_PRINTF ("#========================================================================#\n")
    }
}

void vima_controller_t::print_vima_instructions(){
    ORCS_PRINTF ("=======VIMA INSTRUCTIONS=========\n")
    for (size_t i = 0; i < vima_buffer.size(); i++){
        ORCS_PRINTF ("uop %lu %s readyAt %lu status %s\n", vima_buffer[i]->uop_number, get_enum_memory_operation_char (vima_buffer[i]->memory_operation), vima_buffer[i]->readyAt, get_enum_package_state_char (vima_buffer[i]->status))
    }
    ORCS_PRINTF ("=================================\n")
}

void vima_controller_t::instruction_ready (size_t index){
    #if VIMA_DEBUG 
        ORCS_PRINTF ("%lu VIMA Controller clock(): instruction %lu, %s ready at cycle %lu.\n", orcs_engine.get_global_cycle(), vima_buffer[index]->uop_number, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->readyAt)
    #endif

    if (vima_buffer[index]->vima_write != 0) this->write_to_cache (index);

    //ORCS_PRINTF ("%lu VIMA request from processor %u READY\n", vima_buffer.size(), vima_buffer[index]->processor_id)
    vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[index]), vima_buffer.end());
}

vima_vector_t* vima_controller_t::search_cache (uint64_t address, cache_status_t* result){
    uint64_t lru_cycle = UINT64_MAX;
    uint32_t lru_way = 0;
    uint32_t index = 0;
    this->current_cache_access_latency += this->get_VIMA_CACHE_LATENCY();
    if (VIMA_CACHE_ASSOCIATIVITY == 1){
        index = get_index (address);
        for (uint32_t i = 0; i < get_lines(); i++){
            if (get_index(cache[i][0].get_address()) == get_index (address) && get_tag(cache[i][0].get_address()) == get_tag (address)) {
                *result = HIT;
                #if VIMA_DEBUG 
                    ORCS_PRINTF ("%lu VIMA Cache HIT! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
                #endif
                return &cache[i][0];
            } else if (cache[i][0].lru < lru_cycle) {
                lru_cycle = cache[i][0].lru;
                lru_way = i;
            }
        }
    } else {
        index = get_index (address);
        for (i = 0; i < VIMA_CACHE_ASSOCIATIVITY; i++){
            if (get_tag(cache[index][i].get_address()) == get_tag (address) && get_tag(cache[index][i].get_address()) == get_tag (address)) {
                *result = HIT;
                #if VIMA_DEBUG 
                    ORCS_PRINTF ("%lu VIMA Cache HIT! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
                #endif
		        return &cache[index][i];
            }
            else if (cache[index][i].lru < lru_cycle) {
                lru_cycle = cache[index][i].lru;
                lru_way = i;
            }
        }
    }
    *result = MISS;
    #if VIMA_DEBUG 
        ORCS_PRINTF ("%lu VIMA Cache MISS! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
    #endif
    if (VIMA_CACHE_ASSOCIATIVITY != 1) return &cache[index][lru_way];
    else return &cache[lru_way][0];
}

void vima_controller_t::check_completion (int index){
    if (read1 != NULL){
        if (read1->status != PACKAGE_STATE_READY) return;
        if (VIMA_UNBALANCED && read1_unbalanced != NULL){
            if (read1_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }

    if (read2 != NULL){
        if (read2->status != PACKAGE_STATE_READY) return;
        if (VIMA_UNBALANCED && read2_unbalanced != NULL){
            if (read2_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }
    vima_buffer[index]->updatePackageWait (this->vima_op_latencies[vima_buffer[index]->memory_operation]);

    if (read1 != NULL) {
        read1->set_lru (orcs_engine.get_global_cycle());
    }
    if (read2 != NULL) {
        read2->set_lru (orcs_engine.get_global_cycle());
    }
}

void vima_controller_t::write_to_cache (int index) {
    write->set_next_address (vima_buffer[index]->vima_write);
    write->set_tag (get_tag (vima_buffer[index]->vima_write));
    write->status = PACKAGE_STATE_READY;
    write->dirty = true;
}

void vima_controller_t::check_cache (int index) {
    cache_status_t result_read1 = MISS;
    cache_status_t result_read2 = MISS;
    cache_status_t result_write = MISS;

    if (vima_buffer[index]->vima_read1 != 0) {
        this->add_cache_accesses();
        this->add_cache_reads();
    }
    if (vima_buffer[index]->vima_read2 != 0) {
        this->add_cache_accesses();
        this->add_cache_reads();
    }
    if (vima_buffer[index]->vima_write != 0) {
        this->add_cache_accesses();
        this->add_cache_writes();
    }

    if (vima_buffer[index]->vima_read1 != vima_buffer[index]->vima_read2 && vima_buffer[index]->vima_read2 != 0) {
        read1 = search_cache (vima_buffer[index]->vima_read1, &result_read1);
        read1->set_lru (orcs_engine.get_global_cycle());
        read2 = search_cache (vima_buffer[index]->vima_read2, &result_read2);
        read2->set_lru (orcs_engine.get_global_cycle());
    } else {
        read1 = search_cache (vima_buffer[index]->vima_read1, &result_read1);
        read1->set_lru (orcs_engine.get_global_cycle());
        read2 = NULL;
    }
    
    if (vima_buffer[index]->vima_write != 0){
        if (vima_buffer[index]->vima_read1 != vima_buffer[index]->vima_write && vima_buffer[index]->vima_read2 != vima_buffer[index]->vima_write){
            write = search_cache (vima_buffer[index]->vima_write, &result_write);
        } else {
            if (vima_buffer[index]->vima_write == vima_buffer[index]->vima_read1) write = read1;
            else if (vima_buffer[index]->vima_write == vima_buffer[index]->vima_read2) write = read2;
        }
    } else write = NULL;

    if (read1 != NULL && result_read1 == MISS){
        this->add_cache_misses();
        if (read1->status == PACKAGE_STATE_READY){
            this->add_cache_writebacks();
            read1->status = PACKAGE_STATE_TRANSMIT;
        } else read1->status = PACKAGE_STATE_WAIT;
        read1->set_next_address (vima_buffer[index]->vima_read1);
        read1->set_tag (get_tag (vima_buffer[index]->vima_read1));
    } else if (vima_buffer[index]->vima_read1 != 0) this->add_cache_hits();

    if (read2 != NULL && result_read2 == MISS){
        this->add_cache_misses();
        if (read2->status == PACKAGE_STATE_READY){
            this->add_cache_writebacks();
            read2->status = PACKAGE_STATE_TRANSMIT;
        } else read2->status = PACKAGE_STATE_WAIT;
        read2->set_next_address (vima_buffer[index]->vima_read2);
        read2->set_tag (get_tag (vima_buffer[index]->vima_read2));
    } else if (vima_buffer[index]->vima_read2 != 0) this->add_cache_hits();

    if (write != NULL && write != read1 && write != read2 && result_write == MISS){
        this->add_cache_misses();
        if (write->status == PACKAGE_STATE_READY){
            this->add_cache_writebacks();
            write->status = PACKAGE_STATE_TRANSMIT;
        }
    } else if (vima_buffer[index]->vima_write != 0) this->add_cache_hits();

    vima_buffer[index]->updatePackageTransmit(this->current_cache_access_latency);
    this->current_cache_access_latency = 0;
}

void vima_controller_t::clock(){
    for (uint32_t i = 0; i < sets; i++){
        for (size_t j = 0; j < VIMA_CACHE_ASSOCIATIVITY; j++) {
            if (this->cache[i][j].status != PACKAGE_STATE_READY) this->cache[i][j].clock();
        }
    }
    
    if (vima_buffer.size() <= 0) return;
    switch (vima_buffer[current_index]->status){
        case PACKAGE_STATE_WAIT:
            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu OUT VIMA %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[current_index]->memory_operation), vima_buffer[current_index]->uop_number, vima_buffer[current_index]->processor_id)
                if (vima_buffer[current_index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[current_index]->vima_read1)
                if (vima_buffer[current_index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[current_index]->vima_read2)
                if (vima_buffer[current_index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[current_index]->vima_write)
                ORCS_PRINTF ("\n")
            #endif
            this->instruction_ready (0);
            break;
        case PACKAGE_STATE_TRANSMIT:
            this->check_completion(0);
            break;
        case PACKAGE_STATE_VIMA:
            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu IN  VIMA %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[current_index]->memory_operation), vima_buffer[current_index]->uop_number, vima_buffer[current_index]->processor_id)
                if (vima_buffer[current_index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[current_index]->vima_read1)
                if (vima_buffer[current_index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[current_index]->vima_read2)
                if (vima_buffer[current_index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[current_index]->vima_write)
                ORCS_PRINTF ("\n")
            #endif
            this->total_wait += (orcs_engine.get_global_cycle() - vima_buffer[0]->vima_cycle);
            this->check_cache(0);
            break;
        default:
            ///ORCS_PRINTF ("%lu %s -> \n", vima_buffer[current_index]->uop_number, get_enum_package_state_char (vima_buffer[current_index]->status))
            vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[current_index]), vima_buffer.end());
            vima_buffer.shrink_to_fit();
            break;
    }
}

void vima_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_vima = cfg_root["VIMA_CONTROLLER"];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_VIMA_BUFFER (cfg_vima["VIMA_BUFFER"]);
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    set_VIMA_CACHE_SIZE (cfg_vima["VIMA_CACHE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_vima["VIMA_VECTOR_SIZE"]);
    set_VIMA_CACHE_ASSOCIATIVITY (cfg_vima["VIMA_CACHE_ASSOCIATIVITY"]);
    set_VIMA_CACHE_LATENCY (cfg_vima["VIMA_CACHE_LATENCY"]);
    set_VIMA_UNBALANCED (cfg_vima["VIMA_UNBALANCED"]);

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
        for (size_t j = 0; j < VIMA_CACHE_ASSOCIATIVITY; j++) {
            this->cache[i][j].allocate();
            this->cache[i][j].set_set(i);
            this->cache[i][j].set_column(j);
        }
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
    
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = cfg_vima["VIMA_LATENCY_INT_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = cfg_vima["VIMA_LATENCY_INT_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = cfg_vima["VIMA_LATENCY_INT_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = cfg_vima["VIMA_LATENCY_FP_ALU"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_ALU] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = cfg_vima["VIMA_LATENCY_FP_DIV"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_DIV] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = cfg_vima["VIMA_LATENCY_FP_MUL"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MUL] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] = cfg_vima["VIMA_LATENCY_INT_MLA"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_INT_MLA] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] = cfg_vima["VIMA_LATENCY_FP_MLA"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_FP_MLA] * this->CORE_TO_BUS_CLOCK_RATIO);

    set_cache_accesses(0);
    set_cache_hits(0);
    set_cache_misses(0);
}

bool vima_controller_t::addRequest (memory_package_t* request){
    if (vima_buffer.size() < this->VIMA_BUFFER) {
        //ORCS_PRINTF ("%lu requests inside the VIMA controller\n", vima_buffer.size())
        request->sent_to_ram = true;
        request->status = PACKAGE_STATE_VIMA;
        request->vima_cycle = orcs_engine.get_global_cycle();
        vima_buffer.push_back (request);
        vima_buffer.shrink_to_fit();

        this->request_count++;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Controller addRequest(): NEW VIMA request from processor %u\n", orcs_engine.get_global_cycle(), request->processor_id)
        #endif
        return true;
    } else {
        request->sent_to_ram = false;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
        #endif
    }
    return false;
}
