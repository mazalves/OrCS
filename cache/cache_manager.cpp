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
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    set_L1_DATA_LATENCY (cfg_root[0]["L1_DATA_LATENCY"]);
    set_L2_LATENCY (cfg_root[0]["L2_LATENCY"]);
    set_LLC_LATENCY (cfg_root[0]["LLC_LATENCY"]);
    set_LINE_SIZE (cfg_root[0]["LINE_SIZE"]);

    set_PREFETCHER_ACTIVE (cfg_root[0]["PREFETCHER_ACTIVE"]);

    set_INSTRUCTION_LEVELS (cfg_root[0]["INSTRUCTION_LEVELS"]);
	set_DATA_LEVELS (cfg_root[0]["DATA_LEVELS"]);
    set_CACHE_MANAGER_DEBUG (cfg_root[0]["CACHE_MANAGER_DEBUG"]);
    set_WAIT_CYCLE (cfg_root[0]["WAIT_CYCLE"]);
    POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);

    set_SIZE_OF_L1_CACHES_ARRAY (cfg_root[0]["SIZE_OF_L1_CACHES_ARRAY"]);     // Numero de caches L1
    set_SIZE_OF_L2_CACHES_ARRAY (cfg_root[0]["SIZE_OF_L2_CACHES_ARRAY"]);     // Numero de caches L2
    set_SIZE_OF_LLC_CACHES_ARRAY (cfg_root[0]["SIZE_OF_LLC_CACHES_ARRAY"]);
    
    set_L1_INST_ASSOCIATIVITY (cfg_root[0]["L1_INST_ASSOCIATIVITY"]);
	set_L1_DATA_ASSOCIATIVITY (cfg_root[0]["L1_DATA_ASSOCIATIVITY"]);
    set_L2_ASSOCIATIVITY (cfg_root[0]["L2_ASSOCIATIVITY"]);
    set_LLC_ASSOCIATIVITY (cfg_root[0]["LLC_ASSOCIATIVITY"]);

    set_PARALLEL_LIM_ACTIVE (cfg_root[0]["PARALLEL_LIM_ACTIVE"]);
    set_MAX_PARALLEL_REQUESTS_CORE (cfg_root[0]["MAX_PARALLEL_REQUESTS_CORE"]);

    uint32_t *CACHE_LEVELS = new uint32_t[POINTER_LEVELS];
    CACHE_LEVELS[0] = SIZE_OF_L1_CACHES_ARRAY;
    CACHE_LEVELS[1] = SIZE_OF_L2_CACHES_ARRAY;
    CACHE_LEVELS[2] = SIZE_OF_LLC_CACHES_ARRAY;    
    
    ICACHE_AMOUNT = new uint32_t[INSTRUCTION_LEVELS];
    DCACHE_AMOUNT = new uint32_t[DATA_LEVELS];

    ICACHE_LATENCY = new uint32_t[INSTRUCTION_LEVELS];
    ICACHE_LATENCY[0] = L1_DATA_LATENCY;
    DCACHE_LATENCY = new uint32_t[DATA_LEVELS];
    DCACHE_LATENCY[0] = L1_DATA_LATENCY;
    DCACHE_LATENCY[1] = L2_LATENCY;
    DCACHE_LATENCY[2] = LLC_LATENCY;

    ICACHE_ASSOCIATIVITY = new uint32_t[INSTRUCTION_LEVELS];
    ICACHE_ASSOCIATIVITY[0] = L1_INST_ASSOCIATIVITY;
    DCACHE_ASSOCIATIVITY = new uint32_t[DATA_LEVELS];
    DCACHE_ASSOCIATIVITY[0] = L1_DATA_ASSOCIATIVITY;
    DCACHE_ASSOCIATIVITY[1] = L2_ASSOCIATIVITY;
    DCACHE_ASSOCIATIVITY[2] = LLC_ASSOCIATIVITY;

//         // cache associativity for each level
// const uint32_t ICACHE_ASSOCIATIVITY[1] = {8};
// const uint32_t DCACHE_ASSOCIATIVITY[3] = {8, 8, 8};

    data_cache = new cache_t*[DATA_LEVELS];
    instruction_cache = new cache_t*[INSTRUCTION_LEVELS];

    mshr_index = 0;

    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        this->data_cache[i] = NULL;
        DCACHE_AMOUNT[i] = CACHE_LEVELS[i];
    }
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        this->instruction_cache[i] = NULL;
        ICACHE_AMOUNT[i] = CACHE_LEVELS[i];
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
    // printf("%s\n", "-> generateIndexArray in cache_manager.cpp");
    for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
        cache_indexes[i] = processor_id & (DCACHE_AMOUNT[i] - 1);
    }
}

// Install an address in every cache using pointers
void cache_manager_t::installCacheLines(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type) {
    // printf("%s\n", "-> installCacheLines in cache_manager.cpp");
    uint32_t i, j;
    line_t ***line = new line_t**[NUMBER_OF_PROCESSORS];
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        line[i] = new line_t*[POINTER_LEVELS];
        for (j = 0; j < POINTER_LEVELS; j++) {
            line[i][j] = NULL;
        }
    }

    if (cache_type == INSTRUCTION) {
        //printf("%s\n", "    Instruction");
        for (i = 0; i < INSTRUCTION_LEVELS; i++) {
            line[0][i] = this->instruction_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request);
            // printf("    installed line[%d]: %d\n", i, line[0][i]);
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

void cache_manager_t::add_mshr_entry (memory_order_buffer_line_t* mob_line, uint64_t latency_request){
    // printf("%s\n", "mshr entry");
    uint64_t tag = (mob_line->memory_address >> this->offset);
    mshr_entry_t* new_entry = new mshr_entry_t;
    new_entry->tag = tag;
    new_entry->latency = latency_request;
    new_entry->valid = false;
    new_entry->issued = false;
    new_entry->treated = false;
    new_entry->cycle_created = orcs_engine.get_global_cycle();
    new_entry->requests.push_back (mob_line);
    mshr_table.push_back (new_entry);
    orcs_engine.memory_controller->add_requests_llc();
}

bool cache_manager_t::isInMSHR (memory_order_buffer_line_t* mob_line){
    uint64_t tag = (mob_line->memory_address >> this->offset);
    for (std::size_t i = 0; i < mshr_table.size(); i++){
        if (mshr_table[i]->tag == tag) {
            mshr_table[i]->requests.push_back (mob_line);
            return true;
        }
    }
    return false;
}

void cache_manager_t::clock() {
    if (mshr_table.size() > 0){
        mshr_index += 1;
        if (mshr_index >= this->MAX_PARALLEL_REQUESTS_CORE || mshr_index >= mshr_table.size()) mshr_index = 0;
        if (mshr_table[mshr_index]->valid) {
            int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
            this->generateIndexArray(mshr_table[mshr_index]->requests[0]->processor_id, cache_indexes);
            this->installCacheLines(mshr_table[mshr_index]->requests[0]->memory_address, cache_indexes, mshr_table[mshr_index]->latency, DATA);
            if (mshr_table[mshr_index]->requests[0]->memory_operation == MEMORY_OPERATION_WRITE){
                int cache_level = DATA_LEVELS - 1;
                for (int32_t k = cache_level - 1; k >= 0; k--) {
                    this->data_cache[k+1][cache_indexes[k+1]].returnLine(mshr_table[mshr_index]->requests[0]->memory_address, &this->data_cache[k][cache_indexes[k]]);
                }
                this->data_cache[0][cache_indexes[0]].write(mshr_table[mshr_index]->requests[0]->memory_address);
            }
            
            for (uint32_t j = 0; j < mshr_table[mshr_index]->requests.size(); j++){
                mshr_table[mshr_index]->requests[j]->updatePackageReady (mshr_table[mshr_index]->latency);
            }
            mshr_table.erase (std::remove (mshr_table.begin(), mshr_table.end(), mshr_table[mshr_index]), mshr_table.end());
        }
        else {
            if (!mshr_table[mshr_index]->issued){
                orcs_engine.memory_controller->requestDRAM(mshr_table[mshr_index], mshr_table[mshr_index]->requests[0]->memory_address);
                mshr_table[mshr_index]->issued = true;
            }
        }
    }
    //for (std::size_t i = 0; i < mshr_table.size(); i++){
        
    //}
}

uint32_t cache_manager_t::searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc) {
    // printf("%s\n", "-> searchAddress in cache_manager.cpp");
    uint32_t cache_status = cache->read(instructionAddress, *ttc);
    cache->add_cache_access();
    *latency_request += *ttc;
    return cache_status;
}

uint32_t cache_manager_t::llcMiss(memory_order_buffer_line_t* mob_line, uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, cacheId_t cache_type) {
    // printf("%s\n", "-> llcMiss in cache_manager.cpp");
    std::ignore = ttc;
    if (mob_line != NULL) {
        this->add_mshr_entry (mob_line, latency_request);
    } else {
        latency_request += orcs_engine.memory_controller->requestDRAM(NULL, instructionAddress);
        this->installCacheLines(instructionAddress, cache_indexes, latency_request, cache_type); //instruction, not data
        orcs_engine.memory_controller->add_requests_llc();
    }
    return latency_request;
}

// Searches an instruction in cache levels
uint32_t cache_manager_t::recursiveInstructionSearch(uint64_t instructionAddress, int32_t *cache_indexes,
                                                     uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    // printf("-> recursiveInstructionSearch in cache_manager.cpp - cache level %u\n", cache_level);
    // The first search
    uint32_t cache_status = this->searchAddress(instructionAddress, &this->instruction_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        //printf("    Cache %u level %u hit!!!\n", INSTRUCTION, cache_level);
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
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
    // printf("%s\n", "-> searchInstruction in cache_manager.cpp");
    uint32_t ttc = 0, latency_request = 0, result = 0;
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
    this->generateIndexArray(processor_id, cache_indexes);
    result = recursiveInstructionSearch(instructionAddress, cache_indexes, latency_request, ttc, 0);
    delete[] cache_indexes;
    return result;
}

uint32_t cache_manager_t::recursiveDataSearch(memory_order_buffer_line_t *mob_line, uint64_t instructionAddress,
                                              int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc,
                                              uint32_t cache_level, cacheId_t cache_type) {
    // printf("-> recursiveDataSearch in cache_manager.cpp - cache level %u\n", cache_level);
    // The first search
    uint32_t cache_status = this->searchAddress(instructionAddress, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    if (cache_status == HIT) {
        //printf("    Cache %u level %u hit!!!\n", DATA, cache_level);
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level != 0) {
            for (int32_t i = cache_level - 1; i >= 0; i--) {
                this->data_cache[i+1][cache_indexes[i+1]].returnLine(instructionAddress, &this->data_cache[i][cache_indexes[i]]);
            }
        }
        if (mob_line != NULL) {
            mob_line->updatePackageReady(latency_request);
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
    uint32_t ttc = 0, latency_request = 0;
    if (!isInMSHR (mob_line)){
        int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
        this->generateIndexArray(mob_line->processor_id, cache_indexes);
        if (mob_line->memory_operation == MEMORY_OPERATION_READ) {
            recursiveDataSearch(mob_line, mob_line->memory_address, cache_indexes, latency_request, ttc, 0, DATA);
        }
        else if (mob_line->memory_operation == MEMORY_OPERATION_WRITE) {
            recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, 0, DATA);
        }
        delete[] cache_indexes;
    }
    return 0;
}

uint32_t cache_manager_t::recursiveDataWrite(memory_order_buffer_line_t *mob_line, int32_t *cache_indexes,
                                             uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type) {
    // printf("-> recursiveDataWrite in cache_manager.cpp - cache level %u\n", cache_level);
    // The first search
    uint32_t cache_status = this->searchAddress(mob_line->memory_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        //printf("    Cache %u level %u hit!!!\n", cache_type, cache_level);
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level > 0) {
            for (int32_t i = cache_level - 1; i >= 0; i--) {
                this->data_cache[i+1][cache_indexes[i+1]].returnLine(mob_line->memory_address, &this->data_cache[i][cache_indexes[i]]);
            }
        }
        this->data_cache[0][cache_indexes[0]].write(mob_line->memory_address);
        mob_line->updatePackageReady(latency_request);
        return latency_request;
    }
    //printf("    Cache %u level %u miss!!!\n", cache_type, cache_level);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        latency_request = llcMiss(mob_line, mob_line->memory_address, cache_indexes, latency_request, ttc, cache_type);
        return latency_request;
    }
    return recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, cache_level + 1, cache_type);
}

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
