#include "./../simulator.hpp"
#include <string>

vima_controller_t::vima_controller_t(){
    this->i = 0;
    this->current_index = 0;

    this->lines = 0;
    this->sets = 0;
    this->free_lines = 0;

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

void vima_controller_t::reset_statistics(){
    this->set_cache_reads(0);
    this->set_cache_writes(0);
    this->set_cache_hits(0);
    this->set_cache_misses(0);
    this->set_cache_accesses(0);
    this->set_cache_writebacks(0);
    
    for (i = 0; i < sets; i++) {
        this->cache[i]->fetch_count = 0;
        this->cache[i]->fetch_latency_total = 0;
        this->cache[i]->writeback_count = 0;
        this->cache[i]->writeback_latency_total = 0;
    }

    this->total_wait = 0;
    this->request_count = 0;
}

void vima_controller_t::print_vima_instructions(){
    ORCS_PRINTF ("=======VIMA INSTRUCTIONS=========\n")
    for (size_t i = vima_buffer_start; i < vima_buffer_count; i = (i + 1) % VIMA_BUFFER){
        ORCS_PRINTF ("uop %lu %s readyAt %lu status %s\n", vima_buffer[i]->uop_number, get_enum_memory_operation_char (vima_buffer[i]->memory_operation), vima_buffer[i]->readyAt, get_enum_package_state_char (vima_buffer[i]->status))
    }
    ORCS_PRINTF ("=================================\n")
}

void vima_controller_t::instruction_ready (size_t index){
    #if VIMA_DEBUG 
        ORCS_PRINTF ("%lu VIMA Controller clock(): instruction %lu, %s ready at cycle %lu.\n", orcs_engine.get_global_cycle(), vima_buffer[index]->uop_number, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->readyAt)
    #endif

    store_hash[(vima_buffer[index]->vima_write >> index_bits_shift) % 1024] = 0;

    vima_buffer_start = (vima_buffer_start + 1) % VIMA_BUFFER;
    vima_buffer_count--;
}

vima_vector_t* vima_controller_t::search_cache (uint64_t address, cache_status_t* result){
    vima_vector_t* ret = NULL;
    uint64_t lru_cycle = UINT64_MAX;
    int32_t lru_way = -1;
    int32_t index = 0;
    this->current_cache_access_latency += this->get_VIMA_CACHE_LATENCY();
    if (VIMA_CACHE_ASSOCIATIVITY == 1){
        index = get_index (address);
        for (uint32_t i = 0; i < get_lines(); i++){
	    if ((get_tag(cache[i][0].get_address()) == get_tag (address))) {
                *result = HIT;
                #if VIMA_DEBUG 
                    ORCS_PRINTF ("%lu VIMA Cache HIT! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
                #endif
                ret = &cache[i][0];
                break;
            } else if (cache[i][0].lru < lru_cycle && cache[i][0].assoc == NULL) {
                lru_cycle = cache[i][0].lru;
                lru_way = i;
            }
        }
    } else {
        index = get_index (address);
        for (i = 0; i < VIMA_CACHE_ASSOCIATIVITY; i++){
            if (get_tag(cache[index][i].get_address()) == get_tag (address)) {
                *result = HIT;
                #if VIMA_DEBUG 
                    ORCS_PRINTF ("%lu VIMA Cache HIT! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
                #endif
		        ret = &cache[index][i];
                break;
            }
            else if (cache[index][i].lru < lru_cycle && cache[i][i].assoc == NULL) {
                lru_cycle = cache[index][i].lru;
                lru_way = i;
            }
        }
    }
    if (ret == NULL && lru_way != -1){
        *result = MISS;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Cache MISS! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
        #endif
        if (VIMA_CACHE_ASSOCIATIVITY != 1) {
            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu VIMA Cache address %lu vector will be put on [%u][%u].\n", orcs_engine.get_global_cycle(), address, index, lru_way)
            #endif
            ret = &cache[index][lru_way];
        }
        else {
            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu VIMA Cache address %lu vector will be put on [%u][%u].\n", orcs_engine.get_global_cycle(), address, lru_way, 0)
            #endif
            ret = &cache[lru_way][0];
        }
    }
    return ret;
}

void vima_controller_t::check_completion (int index){
    if (vima_buffer[index]->vima_read1_vec != NULL){
        if (vima_buffer[index]->vima_read1_vec->status != PACKAGE_STATE_READY) return;
        vima_buffer[index]->vima_read1_vec->set_lru (orcs_engine.get_global_cycle());
        if (VIMA_UNBALANCED){
            if (vima_buffer[index]->vima_read1_vec_ub != NULL){
                if (vima_buffer[index]->vima_read1_vec_ub->status != PACKAGE_STATE_READY) return;
                vima_buffer[index]->vima_read1_vec_ub->set_lru (orcs_engine.get_global_cycle());
            }
        }
    }

    if (vima_buffer[index]->vima_read2_vec != NULL){
        if (vima_buffer[index]->vima_read2_vec->status != PACKAGE_STATE_READY) return;
        vima_buffer[index]->vima_read2_vec->set_lru (orcs_engine.get_global_cycle());
        if (VIMA_UNBALANCED){
            if (vima_buffer[index]->vima_read2_vec_ub != NULL){
                if (vima_buffer[index]->vima_read2_vec_ub->status != PACKAGE_STATE_READY) return;
                vima_buffer[index]->vima_read2_vec_ub->set_lru (orcs_engine.get_global_cycle());
            }
        }
    }

    if (vima_buffer[index]->vima_write_vec == NULL){
        cache_status_t result = MISS;
        vima_buffer[index]->vima_write_vec = search_cache (vima_buffer[index]->vima_write, &result);
        if (vima_buffer[index]->vima_write_vec == NULL) return;

        if (vima_buffer[index]->memory_operation == MEMORY_OPERATION_VIMA_SCATTER) result = MISS;
        if (result == MISS) {
            this->add_cache_misses();
            if (vima_buffer[index]->vima_write_vec->dirty){
                this->add_cache_writebacks();
                vima_buffer[index]->vima_write_vec->status = PACKAGE_STATE_TRANSMIT;
            }
        } else this->add_cache_hits();
        this->add_cache_writes();
        this->add_cache_accesses();
    } 

    vima_buffer[index]->vima_write_vec->set_next_address (vima_buffer[index]->vima_write);
    vima_buffer[index]->vima_write_vec->set_tag (get_tag (vima_buffer[index]->vima_write));
    vima_buffer[index]->vima_write_vec->set_lru (orcs_engine.get_global_cycle());
    vima_buffer[index]->vima_write_vec->dirty = true;

    if (VIMA_UNBALANCED){
        if (vima_buffer[index]->vima_write_vec_ub == NULL){
            cache_status_t result = MISS;
            vima_buffer[index]->vima_write_vec_ub = search_cache (vima_buffer[index]->vima_write + VIMA_VECTOR_SIZE, &result);
            if (vima_buffer[index]->vima_write_vec_ub == NULL) return;
            vima_buffer[index]->vima_write_vec_ub->assoc = vima_buffer[index];
                
            if (vima_buffer[index]->memory_operation == MEMORY_OPERATION_VIMA_SCATTER) result = MISS;
            if (result == MISS) {
                this->add_cache_misses();
                if (vima_buffer[index]->vima_write_vec_ub->dirty){
                    this->add_cache_writebacks();
                    vima_buffer[index]->vima_write_vec_ub->status = PACKAGE_STATE_TRANSMIT;
                }
            } else this->add_cache_hits();
            this->add_cache_writes();
            this->add_cache_accesses();
        } 

        vima_buffer[index]->vima_write_vec_ub->set_next_address (vima_buffer[index]->vima_write);
        vima_buffer[index]->vima_write_vec_ub->set_tag (get_tag (vima_buffer[index]->vima_write));
        vima_buffer[index]->vima_write_vec_ub->set_lru (orcs_engine.get_global_cycle());
        vima_buffer[index]->vima_write_vec_ub->dirty = true;
    }

    vima_buffer[index]->updatePackageWait (this->vima_op_latencies[vima_buffer[index]->memory_operation]);
    store_hash[(vima_buffer[index]->vima_write >> index_bits_shift) % 1024] = 0;

    #if VIMA_DEBUG
        ORCS_PRINTF ("%lu VIMA instruction %lu TRANSMIT -> WAIT.\n", orcs_engine.get_global_cycle(), vima_buffer[index]->uop_number)
    #endif
}

void vima_controller_t::process_instruction (uint32_t index){
    switch (vima_buffer[index]->status){
        case PACKAGE_STATE_WAIT:
            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu OUT VIMA %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->uop_number, vima_buffer[index]->processor_id)
                if (vima_buffer[index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[index]->vima_read1)
                if (vima_buffer[index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[index]->vima_read2)
                if (vima_buffer[index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[index]->vima_write)
                    ORCS_PRINTF ("\n")
            #endif
            this->instruction_ready (index);
            break;
        case PACKAGE_STATE_TRANSMIT:
            if (index == vima_buffer_start) this->check_completion (vima_buffer_start);
            break;
        case PACKAGE_STATE_VIMA:
            cache_status_t result_read1, result_read2;
            if (vima_buffer[index]->vima_read1 != 0 && vima_buffer[index]->vima_read1_vec == NULL) {
                if (store_hash[(vima_buffer[index]->vima_read1 >> index_bits_shift) % 1024] == 0) {
                    vima_buffer[index]->vima_read1_vec = search_cache (vima_buffer[index]->vima_read1, &result_read1);
                    if (vima_buffer[index]->vima_read1_vec == NULL) return;
                    vima_buffer[index]->vima_read1_vec->assoc = vima_buffer[index];
                }
            }

            if (vima_buffer[index]->vima_read2 != 0 && vima_buffer[index]->vima_read2_vec == NULL) {
                if (store_hash[(vima_buffer[index]->vima_read2 >> index_bits_shift) % 1024] == 0) {
                    vima_buffer[index]->vima_read2_vec = search_cache (vima_buffer[index]->vima_read2, &result_read2);
                    if (vima_buffer[index]->vima_read2_vec == NULL) return;
                    vima_buffer[index]->vima_read2_vec->assoc = vima_buffer[index];
                }
            }

	    if ((vima_buffer[index]->vima_read1 != 0 && vima_buffer[index]->vima_read1_vec == NULL) || (vima_buffer[index]->vima_read2 != 0 && vima_buffer[index]->vima_read2_vec == NULL)){
                if (vima_buffer[index]->vima_read1_vec != NULL) vima_buffer[index]->vima_read1_vec->assoc = NULL;
                if (vima_buffer[index]->vima_read2_vec != NULL) vima_buffer[index]->vima_read2_vec->assoc = NULL;
                vima_buffer[index]->vima_read1_vec = NULL;
                vima_buffer[index]->vima_read2_vec = NULL;
                return;
            }

            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu IN  VIMA %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->uop_number, vima_buffer[index]->processor_id)
                if (vima_buffer[index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[index]->vima_read1)
                if (vima_buffer[index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[index]->vima_read2)
                if (vima_buffer[index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[index]->vima_write)
                    ORCS_PRINTF ("\n")
            #endif

            if (vima_buffer[index]->vima_read1 != 0){
		if (vima_buffer[index]->vima_read1_vec == NULL) return;
                if (vima_buffer[index]->memory_operation == MEMORY_OPERATION_VIMA_GATHER) result_read1 = MISS;
                if (result_read1 == MISS) {
                    this->add_cache_misses();
                    if (vima_buffer[index]->vima_read1_vec->dirty){
                        this->add_cache_writebacks();
                        vima_buffer[index]->vima_read1_vec->status = PACKAGE_STATE_TRANSMIT;
                    } else vima_buffer[index]->vima_read1_vec->status = PACKAGE_STATE_WAIT;
                    vima_buffer[index]->vima_read1_vec->set_next_address (vima_buffer[index]->vima_read1);
                    vima_buffer[index]->vima_read1_vec->set_tag (get_tag (vima_buffer[index]->vima_read1));
                } else {
                    vima_buffer[index]->vima_read1_vec->status = PACKAGE_STATE_READY;
                    this->add_cache_hits();
                }
                this->add_cache_accesses();
                this->add_cache_reads();
            }

            if (vima_buffer[index]->vima_read2 != 0){
		if (vima_buffer[index]->vima_read2_vec == NULL) return;
                if (result_read2 == MISS) {
                    this->add_cache_misses();
                    if (vima_buffer[index]->vima_read2_vec->dirty){
                        this->add_cache_writebacks();
                        vima_buffer[index]->vima_read2_vec->status = PACKAGE_STATE_TRANSMIT;
                    } else vima_buffer[index]->vima_read2_vec->status = PACKAGE_STATE_WAIT;
                    vima_buffer[index]->vima_read2_vec->set_next_address (vima_buffer[index]->vima_read2);
                    vima_buffer[index]->vima_read2_vec->set_tag (get_tag (vima_buffer[index]->vima_read2));
                } else {
                    vima_buffer[index]->vima_read2_vec->status = PACKAGE_STATE_READY;
                    this->add_cache_hits();
                }
                this->add_cache_accesses();
                this->add_cache_reads();
            }

            store_hash[(vima_buffer[index]->vima_write >> index_bits_shift) % 1024] = 1;
            vima_buffer[index]->updatePackageTransmit(this->current_cache_access_latency);
            #if VIMA_DEBUG
                ORCS_PRINTF ("%lu VIMA instruction %lu VIMA -> TRANSMIT.\n", orcs_engine.get_global_cycle(), vima_buffer[index]->uop_number)
            #endif
            break;
        default:
            break;
    }
}

void vima_controller_t::clock(){
    this->free_lines = 0;
    for (uint32_t i = 0; i < sets; i++){
        for (size_t j = 0; j < VIMA_CACHE_ASSOCIATIVITY; j++) {
            this->cache[i][j].clock();
            if (this->cache[i][j].assoc == NULL) this->free_lines++;
        }
    }

    if (vima_buffer_count == 0) return;

    /*if (vima_buffer[vima_buffer_start]->vima_read1_vec == NULL || vima_buffer[vima_buffer_start]->vima_read2_vec == NULL || vima_buffer[vima_buffer_start]->vima_write_vec == NULL || (VIMA_UNBALANCED && (vima_buffer[vima_buffer_start]->vima_read1_vec_ub == NULL || vima_buffer[vima_buffer_start]->vima_read2_vec_ub == NULL || vima_buffer[vima_buffer_start]->vima_write_vec_ub == NULL))){
        this->process_instruction (vima_buffer_start);
        return;
    }*/

    for (uint32_t i = 0; i < vima_buffer_count; i++) {
        /*ORCS_PRINTF ("[%u] read1 -> %s, read2 -> %s, write -> %s.\n", i, 
            vima_buffer[(vima_buffer_start + i) % VIMA_BUFFER]->vima_read1_vec == NULL ? "YES" : "NO", 
            vima_buffer[(vima_buffer_start + i) % VIMA_BUFFER]->vima_read2_vec == NULL ? "YES" : "NO", 
            vima_buffer[(vima_buffer_start + i) % VIMA_BUFFER]->vima_write_vec == NULL ? "YES" : "NO"
        )*/
        this->process_instruction ((vima_buffer_start + i) % VIMA_BUFFER);
    }
}

void vima_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_vima = cfg_root["VIMA_CONTROLLER"];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_CORE_TO_BUS_CLOCK_RATIO (cfg_memory_ctrl["CORE_TO_BUS_CLOCK_RATIO"]);
    set_VIMA_CACHE_SIZE (cfg_vima["VIMA_CACHE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_vima["VIMA_VECTOR_SIZE"]);
    set_VIMA_CACHE_ASSOCIATIVITY (cfg_vima["VIMA_CACHE_ASSOCIATIVITY"]);
    set_VIMA_CACHE_LATENCY (cfg_vima["VIMA_CACHE_LATENCY"]);
    set_VIMA_UNBALANCED (cfg_vima["VIMA_UNBALANCED"]);
    set_VIMA_BUFFER (cfg_vima["VIMA_BUFFER"]);
    //set_VIMA_UNBALANCED (1);

    this->set_lines (this->get_VIMA_CACHE_SIZE()/this->get_VIMA_VECTOR_SIZE());
    this->set_sets (lines/this->get_VIMA_CACHE_ASSOCIATIVITY());

    this->set_VIMA_BUFFER (lines/3);
    if (VIMA_UNBALANCED) this->set_VIMA_BUFFER (lines/6);

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
    vima_op_latencies[MEMORY_OPERATION_VIMA_GATHER] = cfg_vima["VIMA_LATENCY_GATHER"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_GATHER] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_GATHER] * this->CORE_TO_BUS_CLOCK_RATIO);
    vima_op_latencies[MEMORY_OPERATION_VIMA_SCATTER] = cfg_vima["VIMA_LATENCY_SCATTER"];
    vima_op_latencies[MEMORY_OPERATION_VIMA_SCATTER] = ceil (this->vima_op_latencies[MEMORY_OPERATION_VIMA_SCATTER] * this->CORE_TO_BUS_CLOCK_RATIO);

    set_cache_accesses(0);
    set_cache_hits(0);
    set_cache_misses(0);

    vima_buffer_start = 0;
    vima_buffer_end = 0;
    vima_buffer_count = 0;

    vima_buffer = (memory_package_t**) malloc (VIMA_BUFFER * sizeof (memory_package_t*));
    store_hash = (uint16_t*) malloc (1024 * sizeof (uint16_t));

    free_lines = this->get_lines();
}

bool vima_controller_t::addRequest (memory_package_t* request){
    if (vima_buffer_count < this->VIMA_BUFFER) {
        uint32_t line_count = 0;
        if (request->vima_read1 != 0) {
		line_count++;
		if (VIMA_UNBALANCED) line_count++;
	}
        if (request->vima_read2 != 0) {
		line_count++;
		if (VIMA_UNBALANCED) line_count++;
	}
        if (request->vima_write != 0) {
		line_count++;
		if (VIMA_UNBALANCED) line_count++;
	}
        if (line_count > free_lines) return false;

        request->sent_to_ram = true;
        request->status = PACKAGE_STATE_VIMA;
        request->vima_cycle = orcs_engine.get_global_cycle();

        this->request_count++;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Controller addRequest(): NEW VIMA request from processor %u | count = %u | VIMA_BUFFER = %u\n", orcs_engine.get_global_cycle(), request->processor_id, vima_buffer_count, VIMA_BUFFER)
        #endif

        vima_buffer[vima_buffer_end] = request;
        vima_buffer_end = (vima_buffer_end + 1) % VIMA_BUFFER;
        vima_buffer_count++;
        return true;
    } else {
        request->sent_to_ram = false;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
        #endif
    }
    return false;
}
