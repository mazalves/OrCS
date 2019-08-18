#include "./../simulator.hpp"

// Constructor
cache_manager_t::cache_manager_t() {
    
}

// Desctructor
cache_manager_t::~cache_manager_t(){
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) delete[] instruction_cache[i];
    for (uint32_t i = 0; i < DATA_LEVELS; i++) delete[] data_cache[i];
    delete[] data_cache;
    delete[] instruction_cache;
}

void cache_manager_t::check_cache(uint32_t cache_size, uint32_t cache_level) {
    //printf("Checking cache size of level %u\n", cache_level);
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(cache_size) == OK, "Error - Cache Size Array must be power of 2 value\n")
    if (cache_level == 0) {
        ERROR_ASSERT_PRINTF(NUMBER_OF_PROCESSORS == cache_size, "Error - # First level instruction Caches must be equal # PROCESSORS\n")
    } else {
        ERROR_ASSERT_PRINTF(cache_size <= NUMBER_OF_PROCESSORS,"Error - # Other level instruction Caches must be less or equal # PROCESSORS \n")
    }
}

// TODO exclude enum definition about cache types, we will not use it anymore
// TODO 8 is SandyBridge associativity, Skylake has different associativities for each cache
void cache_manager_t::allocate() {
    L1_DATA_LATENCY = orcs_engine.configuration->getSetting ("L1_DATA_LATENCY");
    L2_LATENCY = orcs_engine.configuration->getSetting ("L2_LATENCY");
    LLC_LATENCY = orcs_engine.configuration->getSetting ("LLC_LATENCY");
    LINE_SIZE = orcs_engine.configuration->getSetting ("LINE_SIZE");

    PREFETCHER_ACTIVE = orcs_engine.configuration->getSetting ("PREFETCHER_ACTIVE");

    INSTRUCTION_LEVELS = orcs_engine.configuration->getSetting ("INSTRUCTION_LEVELS");
	DATA_LEVELS = orcs_engine.configuration->getSetting ("DATA_LEVELS");
    CACHE_LEVELS = orcs_engine.configuration->getSetting ("CACHE_LEVELS");
    CACHE_MANAGER_DEBUG = orcs_engine.configuration->getSetting ("CACHE_MANAGER_DEBUG");
    WAIT_CYCLE = orcs_engine.configuration->getSetting ("WAIT_CYCLE");
    POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);

    SIZE_OF_L1_CACHES_ARRAY = orcs_engine.configuration->getSetting ("SIZE_OF_L1_CACHES_ARRAY");     // Numero de caches L1
    SIZE_OF_L2_CACHES_ARRAY = orcs_engine.configuration->getSetting ("SIZE_OF_L2_CACHES_ARRAY");     // Numero de caches L2
    SIZE_OF_LLC_CACHES_ARRAY = orcs_engine.configuration->getSetting ("SIZE_OF_LLC_CACHES_ARRAY");

    data_cache = new cache_t*[DATA_LEVELS];
    instruction_cache = new cache_t*[INSTRUCTION_LEVELS];

    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        this->data_cache[i] = NULL;
    }
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        this->instruction_cache[i] = NULL;
    }

    //printf("%s\n", "Allocating caches in cacheManager");
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        this->instruction_cache[i] = new cache_t[ICACHE_AMOUNT[i]];
        check_cache(ICACHE_AMOUNT[i], i);
        for (uint32_t j = 0; j < ICACHE_AMOUNT[i]; j++) {
            this->instruction_cache[i][j].allocate(INSTRUCTION, i, ICACHE_SIZE[i], ICACHE_ASSOCIATIVITY[i], ICACHE_LATENCY[i]);
        }
    }
    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        this->data_cache[i] = new cache_t[DCACHE_AMOUNT[i]];
        check_cache(DCACHE_AMOUNT[i], i);
        for (uint32_t j = 0; j < DCACHE_AMOUNT[i]; j++) {
            this->data_cache[i][j].allocate(DATA, i, DCACHE_SIZE[i], DCACHE_ASSOCIATIVITY[i], DCACHE_LATENCY[i]);
        }
    }

    //Read/Write counters
    this->set_read_hit(0);
    this->set_read_miss(0);
    this->set_write_hit(0);
    this->set_write_miss(0);
    this->set_offset(utils_t::get_power_of_two(LINE_SIZE));

    //Allocate Prefetcher
    if (PREFETCHER_ACTIVE){
        this->prefetcher = new prefetcher_t;
        this->prefetcher->allocate();
    }
}

// Dependending on processor_id, returns its correspondent cache
void cache_manager_t::generateIndexArray(uint32_t processor_id, int32_t *cache_indexes) {
    //printf("%s\n", "-> generateIndexArray in cache_manager.cpp");
    for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
        cache_indexes[i] = processor_id & (DCACHE_AMOUNT[i] - 1);
    }
}

// Install an address in every cache using pointers
void cache_manager_t::installCacheLines(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type) {
    //printf("%s\n", "-> installCacheLines in cache_manager.cpp");
    uint32_t i, j;
    line_t ***line = new line_t**[NUMBER_OF_PROCESSORS];
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        line[i] = new line_t*[POINTER_LEVELS];
    }
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            line[i][j] = NULL;
        }
    }
    if (cache_type == INSTRUCTION) {
        //printf("%s\n", "    Instruction");
        for (i = 0; i < INSTRUCTION_LEVELS; i++) {
            line[0][i] = this->instruction_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request);
            // printf("    installed line[%d]: %p\n", i, line[0][i]);
        }
    } else {
        i = 0;
    }
    for (; i < POINTER_LEVELS; i++) {
        line[0][i] = this->data_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request);
        // printf("    installed line[%d]: %p\n", i, line[0][i]);
    }
    for (i = 0; i < POINTER_LEVELS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            if (i == j) {
                continue;
            }
            line[0][i]->line_ptr_caches[0][j] = line[0][j];
            // //printf("    line[%d]->cache %d: %p\n", i, j, line[0][j]);
        }
    }
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            line[i][j] = NULL;
        }
    }
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) delete[] line[i];
    delete[] line;
}

mshr_entry_t* cache_manager_t::add_mshr_entry (memory_order_buffer_line_t* mob_line, uint64_t latency_request, int32_t* cache_indexes, cacheId_t cache_type){
    uint64_t tag = (mob_line->memory_address >> this->offset);
    for (std::size_t i = 0; i < mshr_table.size(); i++){
        if (mshr_table[i]->tag == tag) {
            mshr_table[i]->requests.push_back (mob_line);
            return mshr_table[i];
        }
    }

    mshr_entry_t* new_entry = new mshr_entry_t;
    new_entry->tag = tag;
    new_entry->latency = latency_request;
    new_entry->valid = false;
    new_entry->issued = true;
    new_entry->cache_indexes = cache_indexes;
    new_entry->cache_type = cache_type;
    new_entry->requests.push_back (mob_line);
    mshr_table.push_back (new_entry);
    return new_entry;
}

void cache_manager_t::clock() {
    for (std::size_t i = 0; i < mshr_table.size(); i++){
        if (mshr_table[i]->valid) {
            //uint64_t oldest = mshr_table[i]->requests[0]->readyAt;
            //int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
            //this->generateIndexArray(mshr_table[i]->requests[0]->processor_id, cache_indexes);
            for (uint32_t j = 0; j < mshr_table[i]->requests.size(); j++){
                mshr_table[i]->requests[j]->updatePackageReady (mshr_table[i]->latency);
            }
            //this->installCacheLines(mshr_table[i]->requests[0]->memory_address, mshr_table[i]->cache_indexes, mshr_table[i]->latency, mshr_table[i]->cache_type);
            mshr_table.erase (std::remove (mshr_table.begin(), mshr_table.end(), mshr_table[i]), mshr_table.end());
        }
    }
}

uint32_t cache_manager_t::searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc) {
    //printf("%s\n", "-> searchAddress in cache_manager.cpp");
    uint32_t cache_status = cache->read(instructionAddress, *ttc);
    cache->add_cache_access();
    *latency_request += *ttc;
    return cache_status;
}

uint32_t cache_manager_t::llcMiss(memory_order_buffer_line_t* mob_line, uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, cacheId_t cache_type) {
    mshr_entry_t* add = NULL;
    std::ignore = add;
    //printf("%s\n", "-> llcMiss in cache_manager.cpp");
    ttc = orcs_engine.memory_controller->requestDRAM(instructionAddress);
    orcs_engine.memory_controller->add_requests_llc();
    latency_request += ttc;
    if (mob_line != NULL) {
        add = this->add_mshr_entry (mob_line, latency_request, cache_indexes, cache_type);
        add->valid = true;
        //ORCS_PRINTF ("CM: %u\n", latency_request);
        this->installCacheLines(mob_line->memory_address, cache_indexes, latency_request, cache_type);
    } else this->installCacheLines(instructionAddress, cache_indexes, latency_request, cache_type);
    return latency_request;
}

// Searches an instruction in cache levels
uint32_t cache_manager_t::recursiveInstructionSearch(uint64_t instructionAddress, int32_t *cache_indexes,
                                                     uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    //printf("-> recursiveInstructionSearch in cache_manager.cpp - cache level %u\n", cache_level);
    // The first search
    uint32_t cache_status = this->searchAddress(instructionAddress, &this->instruction_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        //printf("    Cache %u level %u hit!!!\n", INSTRUCTION, cache_level);
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level != 0) {
            for (int32_t i = INSTRUCTION_LEVELS - 1; i >= 0; i--) {
                this->instruction_cache[cache_level][cache_indexes[cache_level]].returnLine(instructionAddress, &this->instruction_cache[i][cache_indexes[i]]);
            }
        }
        return latency_request;
    }
    // When cache miss, searches in lower levels
    //printf("    Cache %u level %u miss!!!\n", INSTRUCTION, cache_level);
    this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == INSTRUCTION_LEVELS - 1) {
        return recursiveDataSearch(NULL, instructionAddress, cache_indexes, latency_request, ttc, cache_level + 1, INSTRUCTION);
    }
    return recursiveInstructionSearch(instructionAddress, cache_indexes, latency_request, ttc, cache_level + 1);
}

// Searches an address in instruction cache, and lower data caches
uint32_t cache_manager_t::searchInstruction(uint32_t processor_id, uint64_t instructionAddress) {
    //printf("%s\n", "-> searchInstruction in cache_manager.cpp");
    uint32_t ttc = 0, latency_request = 0, result = 0;
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
    this->generateIndexArray(processor_id, cache_indexes);
    result = recursiveInstructionSearch(instructionAddress, cache_indexes, latency_request, ttc, 0);
    //delete[] cache_indexes;
    return result;
}

uint32_t cache_manager_t::recursiveDataSearch(memory_order_buffer_line_t *mob_line, uint64_t instructionAddress,
                                              int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc,
                                              uint32_t cache_level, cacheId_t cache_type) {
    //printf("-> recursiveDataSearch in cache_manager.cpp - cache level %u\n", cache_level);
    // The first search
    uint32_t cache_status = this->searchAddress(instructionAddress, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    if (cache_status == HIT) {
        //printf("    Cache %u level %u hit!!!\n", DATA, cache_level);
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level != 0) {
            for (int32_t i = cache_level - 1; i >= 0; i--) {
                this->data_cache[cache_level][cache_indexes[cache_level]].returnLine(instructionAddress, &this->data_cache[i][cache_indexes[i]]);
            }
        }
        // if (cache_level == DATA_LEVELS - 1 && cache_type == DATA) {
        //     #if PREFETCHER_ACTIVE
        //         this->prefetcher->prefecht(mob_line, &this->data_cache[cache_level][cache_indexes[cache_level]]);
        //     #endif
        // }
        if (mob_line != NULL) {
            mob_line->updatePackageReady(latency_request);
            //ORCS_PRINTF ("CACHE LEVEL: %u, LATENCY: %u\n", cache_level, latency_request)
        }
        return latency_request;
    }
    //printf("    Cache %u level %u miss!!!\n", DATA, cache_level);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        //printf("%s\n", "        LLC reached!");
        if (cache_type == DATA) {
            // orcs_engine.processor[mob_line->processor_id].has_llc_miss = true; // setting llc miss
            // mob_line->is_llc_miss = true;
            mob_line->waiting_DRAM = true;
            orcs_engine.processor[mob_line->processor_id].request_DRAM++;
            // #if PREFETCHER_ACTIVE
            //     this->prefetcher->prefecht(mob_line, &this->data_cache[cache_level][cache_indexes[cache_level]]);
            // #endif
        }
        return llcMiss(mob_line, instructionAddress, cache_indexes, latency_request, ttc, cache_type);
    }
    return recursiveDataSearch(mob_line, instructionAddress, cache_indexes, latency_request, ttc, cache_level + 1, cache_type);
}

uint32_t cache_manager_t::searchData(memory_order_buffer_line_t *mob_line) {
    //printf("%s\n", "-> searchData in cache_manager.cpp");
    //ORCS_PRINTF ("ADDRESS: %lu, OPERATION: %d, ", mob_line->memory_address, mob_line->memory_operation);
    uint32_t ttc = 0, latency_request = 0, result = 0;
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
    this->generateIndexArray(mob_line->processor_id, cache_indexes);
    if (mob_line->memory_operation == MEMORY_OPERATION_READ) {
        result = recursiveDataSearch(mob_line, mob_line->memory_address, cache_indexes, latency_request, ttc, 0, DATA);
		//mob_line->updatePackageReady(result);
    }
    else if (mob_line->memory_operation == MEMORY_OPERATION_WRITE) {
        result = recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, 0, DATA);
        //mob_line->updatePackageReady(result);
    }
    //delete[] cache_indexes;
    return result;
}

uint32_t cache_manager_t::recursiveDataWrite(memory_order_buffer_line_t *mob_line, int32_t *cache_indexes,
                                             uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type) {
    //printf("-> recursiveDataWrite in cache_manager.cpp - cache level %u\n", cache_level);
    // The first search
    uint32_t cache_status = this->searchAddress(mob_line->memory_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        //printf("    Cache %u level %u hit!!!\n", cache_type, cache_level);
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level > 0) {
            for (int32_t i = cache_level - 1; i >= 0; i--) {
                this->data_cache[cache_level][cache_indexes[cache_level]].returnLine(mob_line->memory_address, &this->data_cache[i][cache_indexes[i]]);
            }
        }
        this->data_cache[0][cache_indexes[0]].write(mob_line->memory_address);
        mob_line->updatePackageReady(latency_request);
        //ORCS_PRINTF ("CACHE LEVEL: %u, LATENCY: %u\n", cache_level, latency_request)
        return latency_request;
    }
    //printf("    Cache %u level %u miss!!!\n", cache_type, cache_level);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == CACHE_LEVELS - 1) {
        latency_request = llcMiss(mob_line, mob_line->memory_address, cache_indexes, latency_request, ttc, cache_type);
        this->data_cache[0][cache_indexes[0]].write(mob_line->memory_address);
        return latency_request;
    }
    return recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, cache_level + 1, cache_type);
}

// uint32_t cache_manager_t::search_EMC_Data(memory_order_buffer_line_t *mob_line){
//     uint32_t ttc = 0;
//     uint32_t latency_request = 0;
//
//     int32_t index_llc = this->generateIndexArray(mob_line->processor_id,LLC);
//     if(index_llc==POSITION_FAIL){
//         ERROR_PRINTF("Error on generate index to access array")
//     }
//     uint32_t cache_status = orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->read(mob_line->memory_address,ttc);
//     orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->add_cacheRead();
//     latency_request+=ttc;
//     //EMC data cache Hit
//     if(cache_status==HIT){
//         //=========================================
//         orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->add_cacheAccess();
//         orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->add_cacheHit();
//         //=========================================
//     }else{
//         // EMC CACHE MISS
//         //=========================================
//         orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->add_cacheAccess();
//         orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->add_cacheMiss();
//         //=========================================
//         cache_status = this->LLC_data_cache[index_llc].read(mob_line->memory_address,ttc);
//
//         if(cache_status == HIT){
//             latency_request+=ttc;
//             // marcando access llc emc
//             orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_LLC();
//             orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_LLC_Hit();
//             mob_line->is_llcMiss=false;
//         }else{
//             orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_LLC();
//             orcs_engine.memory_controller->emc[mob_line->processor_id].add_access_llcMiss();
//
//             latency_request += RAM_LATENCY;
//
//             line_t *linha_llc = this->LLC_data_cache[index_llc].installLine(mob_line->memory_address,latency_request);
//             line_t *linha_emc = orcs_engine.memory_controller->emc[mob_line->processor_id].data_cache->installLine(mob_line->memory_address,latency_request);
//             // linking emc and llc
//             linha_llc->line_ptr_emc = linha_emc;
//             linha_emc->line_ptr_llc = linha_llc;
//             orcs_engine.memory_controller->add_requests_emc();//number of requests made by emc
//             orcs_engine.memory_controller->add_requests_made();//add requests made by emc to total
//         }
//     }
//     return latency_request;
// };

void cache_manager_t::statistics(uint32_t core_id){
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
    bool close = false;
    FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
	if (output != NULL){
        utils_t::largestSeparator(output);
        fprintf(output,"##############  Cache Memories ##################\n");
        utils_t::largestSeparator(output);
    }
	if(close) fclose(output);
    this->generateIndexArray(core_id, cache_indexes);
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        this->instruction_cache[i][cache_indexes[i]].statistics();
    }
    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        this->data_cache[i][cache_indexes[i]].statistics();
    }
    // Prefetcher
    if (PREFETCHER_ACTIVE){
        this->prefetcher->statistics();
    }
    
    delete[] cache_indexes;
}
