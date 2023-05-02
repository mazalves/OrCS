#include "./../simulator.hpp"
#include <string>

vima_controller_t::vima_controller_t(){
    this->i = 0;

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
        ORCS_PRINTF ("Remaining VIMA instructions (this ready counts): %u.\n", vima_buffer_count)
    #endif

    if (vima_buffer[index]->vima_write != 0) store_hash[(vima_buffer[index]->vima_write >> index_bits_shift) % 1024] = 0;

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
            if (get_index(cache[i][0].get_address()) == get_index (address) && get_tag(cache[i][0].get_address()) == get_tag (address)) {
                *result = HIT;
                #if VIMA_DEBUG 
                    ORCS_PRINTF ("%lu VIMA Cache HIT! address %lu, tag = %lu, index = %lu.\n", orcs_engine.get_global_cycle(), address, get_tag (address), get_index (address))
                #endif
                ret = &cache[i][0];
                break;
            } else if (cache[i][0].lru < lru_cycle && !cache[i][0].taken) {
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
		        ret = &cache[index][i];
                break;
            }
            else if (cache[index][i].lru < lru_cycle && !cache[i][i].taken) {
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
        else {
            vima_buffer[index]->vima_read1_vec->set_lru (orcs_engine.get_global_cycle());
            vima_buffer[index]->vima_read1_vec->taken = false;
        }
        if (VIMA_UNBALANCED && read1_unbalanced != NULL){
            if (read1_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }

    if (vima_buffer[index]->vima_read2_vec != NULL){
        if (vima_buffer[index]->vima_read2_vec->status != PACKAGE_STATE_READY) return;
        else {
            vima_buffer[index]->vima_read2_vec->set_lru (orcs_engine.get_global_cycle());
            vima_buffer[index]->vima_read2_vec->taken = false;
        }
        if (VIMA_UNBALANCED && read2_unbalanced != NULL){
            if (read2_unbalanced->status != PACKAGE_STATE_READY) return;
        }
    }

    if (vima_buffer[index]->vima_write_vec == NULL){
        cache_status_t result = HIT;
        vima_buffer[index]->vima_write_vec = search_cache (vima_buffer[index]->vima_write, &result);
        if (vima_buffer[index]->vima_write_vec == NULL) return;
            
        if (vima_buffer[index]->memory_operation == MEMORY_OPERATION_VIMA_SCATTER) result = MISS;
        if (result == MISS) {
            this->add_cache_misses();
            if (vima_buffer[index]->vima_write_vec->dirty){
                this->add_cache_writebacks();
                vima_buffer[index]->vima_write_vec->status = PACKAGE_STATE_TRANSMIT;
            } else vima_buffer[index]->vima_write_vec->status = PACKAGE_STATE_WAIT;
        } else this->add_cache_hits();
    }

    if (vima_buffer[index]->vima_write_vec->dirty){
        this->add_cache_writebacks();
        vima_buffer[index]->vima_write_vec->status = PACKAGE_STATE_TRANSMIT;
    } else vima_buffer[index]->vima_write_vec->status = PACKAGE_STATE_WAIT;

    vima_buffer[index]->vima_write_vec->set_next_address (vima_buffer[index]->vima_write);
    vima_buffer[index]->vima_write_vec->set_tag (get_tag (vima_buffer[index]->vima_write));
    vima_buffer[index]->vima_write_vec->set_lru (orcs_engine.get_global_cycle());
    vima_buffer[index]->vima_write_vec->dirty = true;
    vima_buffer[index]->vima_write_vec->taken = false;

    vima_buffer[index]->updatePackageTransactional (this->vima_op_latencies[vima_buffer[index]->memory_operation]);
}

/*
Transmit -> TRANSACTIONAL -> CONFIRM ->  -> WAIT
TRANSACTIONAL: Espera a operação completar com sucesso e dá um readyAt para avisar a CPU após k ciclos.
CONFIRM: Avisa a CPU Espera uma resposta da CPU confirmando (readyAt). Envia dados e vai para WAIT após K *3 ciclos (Enviar Ld 1 + ld 2 + store)
*/
void vima_controller_t::clock(){
    for (uint32_t i = 0; i < sets; i++){
        for (size_t j = 0; j < VIMA_CACHE_ASSOCIATIVITY; j++) {
            this->cache[i][j].clock();
        }
    }
    
    if (vima_buffer_count == 0) return;


    // ******************************
    // Remove old transactions status
    // ******************************

    while (this->CPU_transactions_status.get_size() > 0 && this->CPU_transactions_status[0].unique_conversion_id < vima_buffer[vima_buffer_start]->unique_conversion_id) {
        #if VIMA_DEBUG
        printf("Popping id: %lu\n", this->CPU_transactions_status[0].unique_conversion_id);
        #endif
        this->CPU_transactions_status.pop_front();
    }


    // Get CPU conversion status
    int32_t transaction_index = -1;
    for (uint32_t i=0; i < this->CPU_transactions_status.get_size(); ++i) {
        if (this->CPU_transactions_status[i].unique_conversion_id == vima_buffer[vima_buffer_start]->unique_conversion_id) {
            transaction_index = i;
            break;
        }
    }


    // TODO -> falta garantir que a VIMA comite apenas depois da confirmação das CPU
    // mesmo com isso devemos poder executar outras VIMAs enquanto isso
    uint32_t index = 0;
    for (uint32_t i = 0; i < vima_buffer_count; i++){
        index = (vima_buffer_start + i) % VIMA_BUFFER;

        switch (vima_buffer[index]->status){
            case PACKAGE_STATE_WAIT:
                #if VIMA_DEBUG
                    ORCS_PRINTF ("%lu OUT VIMA [Conversion ID %lu] %s -> %lu | processor: %u", orcs_engine.get_global_cycle(), vima_buffer[index]->unique_conversion_id, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->uop_number, vima_buffer[index]->processor_id)
                    if (vima_buffer[index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[index]->vima_read1)
                    if (vima_buffer[index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[index]->vima_read2)
                    if (vima_buffer[index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[index]->vima_write)
                ORCS_PRINTF ("\n")
                #endif
                this->instruction_ready (index);
                break;
            case PACKAGE_STATE_TRANSACTIONAL:
            #if VIMA_DEBUG
                    ORCS_PRINTF ("%lu TRANSACTIONAL  VIMA [Conversion ID %lu] %s -> %lu readyAt: %lu/%lu", orcs_engine.get_global_cycle(), vima_buffer[index]->unique_conversion_id, get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->uop_number, vima_buffer[index]->readyAt, orcs_engine.get_global_cycle())
                    if (vima_buffer[index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[index]->vima_read1)
                    if (vima_buffer[index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[index]->vima_read2)
                    if (vima_buffer[index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[index]->vima_write)
                    ORCS_PRINTF ("\n")
                #endif
                // If VIMA operation was already completed
                if (vima_buffer[index]->readyAt <= orcs_engine.get_global_cycle()) {

                    // ***************************************************
                    // If before completion the conversion was invalidated 
                    // ***************************************************

                        if ((transaction_index >= 0) && 
                            (this->CPU_transactions_status[transaction_index].status == 2 /* Failure */ && 
                             this->CPU_transactions_status[transaction_index].readyAt < orcs_engine.get_global_cycle()))
                        {
    #if VIMA_CONVERSION_DEBUG == 1
                            printf("%lu VIMA discarding results before inform CPU %lu [Conversion ID %lu]\n", orcs_engine.get_global_cycle(), orcs_engine.get_global_cycle() + 0, vima_buffer[index]->unique_conversion_id);
    #endif
                            vima_buffer[index]->updatePackageWait(1); // VIMA discard results
                        }
                        // *********
                        // Otherwise
                        // *********
                        else {
    #if VIMA_CONVERSION_DEBUG == 1
                            printf("%lu Transactional [Conversion ID %lu] until %lu [Conversion ID %lu]\n", orcs_engine.get_global_cycle(), vima_buffer[index]->unique_conversion_id, orcs_engine.get_global_cycle() + this->latency_burst, vima_buffer[index]->unique_conversion_id);
    #endif
    #if VIMA_CONVERSION_DEBUG == 1
                        printf("%lu VIMA informing CPU... [Conversion ID %lu]\n", orcs_engine.get_global_cycle(), vima_buffer[index]->unique_conversion_id);
    #endif
                            orcs_engine.processor->vima_converter.vima_execution_completed(vima_buffer[index], orcs_engine.get_global_cycle() + this->latency_burst);
                            vima_buffer[index]->cpu_informed = true;

                            vima_buffer[index]->updatePackageWait(1); // VIMA data sent to CPU -> Considering that was sent with the confirmation to the CPU
                        }
                }
                break;
            case PACKAGE_STATE_TRANSMIT:
                if (index == vima_buffer_start) this->check_completion (vima_buffer_start);
                break;
            case PACKAGE_STATE_VIMA:
                read1_d = false;
                read2_d = false;
                write_d = false;

                if (vima_buffer[index]->vima_read1 != 0 && vima_buffer[index]->vima_read1_vec == NULL) {
                    if (store_hash[(vima_buffer[index]->vima_read1 >> index_bits_shift) % 1024] == 0) {
                        cache_status_t result = MISS;
                        vima_buffer[index]->vima_read1_vec = search_cache (vima_buffer[index]->vima_read1, &result);
                        if (vima_buffer[index]->vima_read1_vec == NULL) return;
                        vima_buffer[index]->vima_read1_vec->taken = true;

                        #if VIMA_DEBUG
                            ORCS_PRINTF ("%lu IN  VIMA %s -> %lu | processor: %u [conversion ID: %lu]", orcs_engine.get_global_cycle(), get_enum_memory_operation_char (vima_buffer[index]->memory_operation), vima_buffer[index]->uop_number, vima_buffer[index]->processor_id, vima_buffer[index]->unique_conversion_id)
                            if (vima_buffer[index]->vima_read1 != 0) ORCS_PRINTF (" | READ1: [%lu]", vima_buffer[index]->vima_read1)
                            if (vima_buffer[index]->vima_read2 != 0) ORCS_PRINTF (" | READ2: [%lu]", vima_buffer[index]->vima_read2)
                            if (vima_buffer[index]->vima_write != 0) ORCS_PRINTF (" | WRITE: [%lu]", vima_buffer[index]->vima_write)
                            ORCS_PRINTF ("\n")
                        #endif

                        this->add_cache_accesses();
                        this->add_cache_reads();

                        if (vima_buffer[index]->memory_operation == MEMORY_OPERATION_VIMA_GATHER) result = MISS;

                        if (result == MISS) {
                            this->add_cache_misses();
                            if (vima_buffer[index]->vima_read1_vec->dirty){
                                this->add_cache_writebacks();
                                vima_buffer[index]->vima_read1_vec->status = PACKAGE_STATE_TRANSMIT;
                            } else vima_buffer[index]->vima_read1_vec->status = PACKAGE_STATE_WAIT;
                        } else this->add_cache_hits();
                        vima_buffer[index]->vima_read1_vec->set_next_address (vima_buffer[index]->vima_read1);
                        vima_buffer[index]->vima_read1_vec->set_tag (get_tag (vima_buffer[index]->vima_read1));
                    }
                }
                if (vima_buffer[index]->vima_read2 != 0 && vima_buffer[index]->vima_read2_vec == NULL) {
                    if (store_hash[(vima_buffer[index]->vima_read2 >> index_bits_shift) % 1024] == 0) {
                        cache_status_t result = MISS;
                        vima_buffer[index]->vima_read2_vec = search_cache (vima_buffer[index]->vima_read2, &result);
                        if (vima_buffer[index]->vima_read2_vec == NULL) return;
                        vima_buffer[index]->vima_read2_vec->taken = true;

                        this->add_cache_accesses();
                        this->add_cache_reads();

                        if (result == MISS) {
                            this->add_cache_misses();
                            if (vima_buffer[index]->vima_read2_vec->dirty){
                                this->add_cache_writebacks();
                                vima_buffer[index]->vima_read2_vec->status = PACKAGE_STATE_TRANSMIT;
                            } else vima_buffer[index]->vima_read2_vec->status = PACKAGE_STATE_WAIT;
                        } else this->add_cache_hits();
                        vima_buffer[index]->vima_read2_vec->set_next_address (vima_buffer[index]->vima_read2);
                        vima_buffer[index]->vima_read2_vec->set_tag (get_tag (vima_buffer[index]->vima_read2));
                    }
                }
                if (vima_buffer[index]->vima_write != 0 && vima_buffer[index]->vima_write_vec == NULL) {
                    if (store_hash[(vima_buffer[index]->vima_write >> index_bits_shift) % 1024] == 0) {
                        if (vima_buffer[index]->vima_write != vima_buffer[index]->vima_read1 && vima_buffer[index]->vima_write != vima_buffer[index]->vima_read2){
                            cache_status_t result = MISS;
                            vima_buffer[index]->vima_write_vec = search_cache (vima_buffer[index]->vima_write, &result);
                            if (vima_buffer[index]->vima_write_vec == NULL) return;
                            vima_buffer[index]->vima_write_vec->taken = true;
                            if (result == MISS) this->add_cache_misses();
                            else this->add_cache_hits();
                        }

                        this->add_cache_accesses();
                        this->add_cache_writes();
                        store_hash[(vima_buffer[index]->vima_write >> index_bits_shift) % 1024] = 1;
                    }
                }
                
                vima_buffer[index]->updatePackageTransmit(this->current_cache_access_latency);
                break;
            default:
                ///ORCS_PRINTF ("%lu %s -> \n", vima_buffer[index]->uop_number, get_enum_package_state_char (vima_buffer[index]->status))
                //vima_buffer.erase (std::remove (vima_buffer.begin(), vima_buffer.end(), vima_buffer[index]), vima_buffer.end());
                //vima_buffer.shrink_to_fit();
                break;
        }
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

    // Communication
    set_BURST_WIDTH(cfg_memory_ctrl["BURST_WIDTH"]);
    set_LINE_SIZE(cfg_memory_ctrl["LINE_SIZE"]);
    set_latency_burst(ceil ((LINE_SIZE/BURST_WIDTH) * CORE_TO_BUS_CLOCK_RATIO));
    this->CPU_transactions_status.allocate(1000); // We would need an extremelly large ROB to overflow this


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
}

bool vima_controller_t::addRequest (memory_package_t* request){
    if (vima_buffer_count < this->VIMA_BUFFER) {
        request->sent_to_ram = true;
        request->flushed = false;
        request->status = PACKAGE_STATE_VIMA;
        request->vima_cycle = orcs_engine.get_global_cycle();

        this->request_count++;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Controller addRequest(): NEW VIMA request from processor %u\n", orcs_engine.get_global_cycle(), request->processor_id)
        #endif

        vima_buffer[vima_buffer_end] = request;
        vima_buffer_end = (vima_buffer_end + 1) % VIMA_BUFFER;
        vima_buffer_count++;
        return true;
    } else {
        request->sent_to_ram = false;
        request->flushed = false;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("VIMA Controller addRequest(): VIMA buffer is full!\n")
        #endif
    }
    return false;
}

