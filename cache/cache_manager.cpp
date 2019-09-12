#include "./../simulator.hpp"
#include <string>
using namespace std;
// Constructor
cache_manager_t::cache_manager_t(){
}

// Desctructor
cache_manager_t::~cache_manager_t(){
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) delete[] instruction_cache[i];
    for (uint32_t i = 0; i < DATA_LEVELS; i++) delete[] data_cache[i];
    delete[] data_cache;
    delete[] instruction_cache;
}

void cache_manager_t::check_cache(uint32_t cache_size, uint32_t cache_level) {
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(cache_size) == OK, "Error - Cache Size Array must be power of 2 value\n")
    if (cache_level == 0) {
        ERROR_ASSERT_PRINTF(NUMBER_OF_PROCESSORS == cache_size, "Error - # First level instruction Caches must be equal # PROCESSORS\n")
    } else {
        ERROR_ASSERT_PRINTF(cache_size <= NUMBER_OF_PROCESSORS,"Error - # Other level instruction Caches must be less or equal # PROCESSORS \n")
    }
}

uint32_t *cache_manager_t::get_cache_levels(vector<uint32_t> &v_levels, cache_t *cache, uint32_t cache_amount) {
    // Creates a vector containing the cache levels
    for (uint32_t i = 0; i < cache_amount; i++) {
        if (find(v_levels.begin(), v_levels.end(), cache[i].level) == v_levels.end()) {
            v_levels.push_back(cache[i].level);
        }
    }

    // Creates a vector containing the number of caches in each level
    uint32_t *level_amount = new uint32_t[v_levels.size()];
    for (uint32_t i = 0; i < v_levels.size(); i++) {
        level_amount[i] = 0;
    }
    for (uint32_t i = 0; i < cache_amount; i++) {
        level_amount[cache[i].level]++;
    }
    return level_amount;
}

void cache_manager_t::copy_cache(cache_t **cache, cache_t *aux_cache, uint32_t n_levels, uint32_t *v_levels, uint32_t cache_amount) {
    for (uint32_t i = 0; i < n_levels; i++) {
        for (uint32_t j = 0, k = 0; k < cache_amount; k++) {
            if (aux_cache[k].level == i) {
                cache[i][j] = aux_cache[k];
                if (j < v_levels[i]) {
                    j++;
                }
            }
        }
    }
}

void cache_manager_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
    // Access configure file
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    // libconfig::Config cfg;
    // cfg.readFile(orcs_engine.config_file);
    // libconfig::Setting &cfg_root = cfg.getRoot();

    set_NUMBER_OF_PROCESSORS(NUMBER_OF_PROCESSORS);

    // Get prefetcher info
    libconfig::Setting &prefetcher_defs = cfg_root[0]["PREFETCHER"];
    set_PREFETCHER_ACTIVE(prefetcher_defs["PREFETCHER_ACTIVE"]);
    printf("cache_manager.cpp - PREFETCHER_ACTIVE: %u\n", PREFETCHER_ACTIVE);
    // Get general cache info
    libconfig::Setting &cfg_cache_defs = cfg_root[0]["CACHE_MEMORY"];
    // printf("CACHE_MEMORY SIZE: %d   CONFIG SIZE: %d\n", cfg_cache_defs.getLength(), cfg_cache_defs["CONFIG"]["LINE_SIZE"].getLength());
    set_CACHE_MANAGER_DEBUG(cfg_cache_defs["CONFIG"]["CACHE_MANAGER_DEBUG"]);
    printf("cache_manager.cpp - CACHE_MANAGER_DEBUG: %u\n", CACHE_MANAGER_DEBUG);
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    printf("cache_manager.cpp - LINE_SIZE: %u\n", LINE_SIZE);
    set_WAIT_CYCLE(cfg_cache_defs["CONFIG"]["WAIT_CYCLE"]);
    printf("cache_manager.cpp - WAIT_CYCLE: %u\n", WAIT_CYCLE);

    // Get the list of intruction caches
    libconfig::Setting &cfg_inst_caches = cfg_cache_defs["INSTRUCTION"];
    uint32_t INSTRUCTION_CACHES = cfg_inst_caches.getLength();
    printf("cache_manager.cpp - INSTRUCTION_CACHES: %u\n", INSTRUCTION_CACHES);
    cache_t *instruction_caches = new cache_t[INSTRUCTION_CACHES];

    // Get information of each instruction cache
    for (uint32_t i = 0; i < INSTRUCTION_CACHES; i++) {
        libconfig::Setting &cfg_inst_cache = cfg_inst_caches[i];

        // try {
            instruction_caches[i].id = INSTRUCTION;
            instruction_caches[i].size = cfg_inst_cache["SIZE"];
            instruction_caches[i].level = cfg_inst_cache["LEVEL"];
            instruction_caches[i].offset = utils_t::get_power_of_two(LINE_SIZE);
            instruction_caches[i].latency = cfg_inst_cache["LATENCY"];
            instruction_caches[i].associativity = cfg_inst_cache["ASSOCIATIVITY"];
            instruction_caches[i].n_sets = (instruction_caches[i].size / LINE_SIZE) / instruction_caches[i].associativity;
        // } catch (int e) {
        //     cout << "An exception occurred. Exception Nr. " << e << '\n';
        // }
    }

    // Get the number of cache levels
    vector<uint32_t> cache_levels;
    ICACHE_AMOUNT = this->get_cache_levels(cache_levels, instruction_caches, INSTRUCTION_CACHES);
    set_INSTRUCTION_LEVELS(cache_levels.size());
    instruction_cache = new cache_t *[INSTRUCTION_LEVELS];

    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        this->instruction_cache[i] = new cache_t[ICACHE_AMOUNT[i]];
        check_cache(ICACHE_AMOUNT[i], i);
        copy_cache(this->instruction_cache, instruction_caches, INSTRUCTION_LEVELS, ICACHE_AMOUNT, INSTRUCTION_CACHES);
        for (uint32_t j = 0; j < ICACHE_AMOUNT[i]; j++) {
            // printf("id: %u | size: %u | level: %u | offset: %u | latency: %u | associ: %u | n_sets: %u\n", this->instruction_cache[i][j].id, this->instruction_cache[i][j].size, this->instruction_cache[i][j].level, this->instruction_cache[i][j].offset, this->instruction_cache[i][j].latency, this->instruction_cache[i][j].associativity, this->instruction_cache[i][j].n_sets);
            this->instruction_cache[i][j].allocate(NUMBER_OF_PROCESSORS);
        }
    }

    // Get the list of data caches
    libconfig::Setting &cfg_data_caches = cfg_cache_defs["DATA"];
    uint32_t DATA_CACHES = cfg_data_caches.getLength();
    printf("cache_manager.cpp - DATA_CACHES: %u\n", DATA_CACHES);
    cache_t *data_caches = new cache_t[DATA_CACHES];

    // Get information of each data cache
    for (uint32_t i = 0; i < DATA_CACHES; i++) {
        libconfig::Setting &cfg_data_cache = cfg_data_caches[i];
        // try
        // {
            data_caches[i].id = DATA;
            data_caches[i].size = cfg_data_cache["SIZE"];
            data_caches[i].level = cfg_data_cache["LEVEL"];
            data_caches[i].offset = utils_t::get_power_of_two(LINE_SIZE);
            data_caches[i].latency = cfg_data_cache["LATENCY"];
            data_caches[i].associativity = cfg_data_cache["ASSOCIATIVITY"];
            data_caches[i].n_sets = (data_caches[i].size / LINE_SIZE) / data_caches[i].associativity;
        // } catch (int e) {
        //     cout << "An exception occurred. Exception Nr. " << e << '\n';
        // }
    }

    vector<uint32_t> dcache_levels;
    DCACHE_AMOUNT = this->get_cache_levels(dcache_levels, data_caches, DATA_CACHES);
    for (uint32_t i = 0; i < dcache_levels.size(); i++) {
        printf("DCACHE_AMOUNT[%d] = %u\n", i, DCACHE_AMOUNT[i]);
    }
    set_DATA_LEVELS(dcache_levels.size());
    data_cache = new cache_t *[DATA_LEVELS];
    POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        this->data_cache[i] = new cache_t[DCACHE_AMOUNT[i]];
        check_cache(DCACHE_AMOUNT[i], i);
        copy_cache(this->data_cache, data_caches, DATA_LEVELS, DCACHE_AMOUNT, DATA_CACHES);
    printf("%s\n", "deu ruim aqui");
        for (uint32_t j = 0; j < DCACHE_AMOUNT[i]; j++) {
            printf("id: %u | size: %u | level: %u | offset: %u | latency: %u | associ: %u | n_sets: %u\n", this->data_cache[i][j].id, this->data_cache[i][j].size, this->data_cache[i][j].level, this->data_cache[i][j].offset, this->data_cache[i][j].latency, this->data_cache[i][j].associativity, this->data_cache[i][j].n_sets);
            this->data_cache[i][j].allocate(NUMBER_OF_PROCESSORS);
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
        this->prefetcher->allocate(NUMBER_OF_PROCESSORS);
    }
}

// Dependending on processor_id, returns its correspondent cache
void cache_manager_t::generateIndexArray(uint32_t processor_id, int32_t *cache_indexes) {
    for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
        cache_indexes[i] = processor_id & (DCACHE_AMOUNT[i] - 1);
    }
}

// Install an address in every cache using pointers
void cache_manager_t::installCacheLines(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type) {
    uint32_t i, j;
    line_t ***line = new line_t**[NUMBER_OF_PROCESSORS];
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        line[i] = new line_t*[POINTER_LEVELS];
        for (j = 0; j < POINTER_LEVELS; j++) {
            line[i][j] = NULL;
        }
    }

    if (cache_type == INSTRUCTION) {
        for (i = 0; i < INSTRUCTION_LEVELS; i++) {
            line[0][i] = this->instruction_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request);
        }
    } else {
        i = 0;
    }
    for (; i < POINTER_LEVELS; i++) {
        line[0][i] = this->data_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request);
    }
    for (i = 0; i < POINTER_LEVELS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            if (i == j) {
                continue;
            }
            line[0][i]->line_ptr_caches[0][j] = line[0][j];
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

mshr_entry_t* cache_manager_t::add_mshr_entry (memory_order_buffer_line_t* mob_line, uint64_t latency_request){
    uint64_t tag = (mob_line->memory_address >> this->offset);
    for (std::size_t i = 0; i < mshr_table.size(); i++){
        if (mshr_table[i]->contains (mob_line)) return mshr_table[i];
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
    new_entry->cycle_created = orcs_engine.get_global_cycle();
    new_entry->requests.push_back (mob_line);
    mshr_table.push_back (new_entry);
    return new_entry;
}

void cache_manager_t::clock() {
    for (std::size_t i = 0; i < mshr_table.size(); i++){
        if (mshr_table[i]->valid) {
            int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
            this->generateIndexArray(mshr_table[i]->requests[0]->processor_id, cache_indexes);
            this->installCacheLines(mshr_table[i]->requests[0]->memory_address, cache_indexes, mshr_table[i]->latency, DATA);
            if (mshr_table[i]->requests[0]->memory_operation == MEMORY_OPERATION_WRITE){
                int cache_level = DATA_LEVELS - 1;
                for (int32_t k = cache_level - 1; k >= 0; k--) {
                    this->data_cache[k+1][cache_indexes[k+1]].returnLine(mshr_table[i]->requests[0]->memory_address, &this->data_cache[k][cache_indexes[k]]);
                }
                this->data_cache[0][cache_indexes[0]].write(mshr_table[i]->requests[0]->memory_address);
            }
            
            for (uint32_t j = 0; j < mshr_table[i]->requests.size(); j++){
                mshr_table[i]->requests[j]->updatePackageReady (mshr_table[i]->latency);
            }
            mshr_table.erase (std::remove (mshr_table.begin(), mshr_table.end(), mshr_table[i]), mshr_table.end());
        }
        else orcs_engine.memory_controller->requestDRAM(mshr_table[i], mshr_table[i]->requests[0]->memory_address);
    }
}

uint32_t cache_manager_t::searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc) {
    uint32_t cache_status = cache->read(instructionAddress, *ttc);
    cache->add_cache_access();
    *latency_request += *ttc;
    return cache_status;
}

uint32_t cache_manager_t::llcMiss(memory_order_buffer_line_t* mob_line, uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, cacheId_t cache_type) {
    std::ignore = ttc;
    orcs_engine.memory_controller->add_requests_llc();
    if (mob_line != NULL) {
        this->add_mshr_entry (mob_line, latency_request);
        return 0;
    }
    else {
        latency_request += orcs_engine.memory_controller->requestDRAM(NULL, instructionAddress);
        this->installCacheLines(instructionAddress, cache_indexes, latency_request, cache_type); //instruction, not data
        return latency_request;
    }
}

// Searches an instruction in cache levels
uint32_t cache_manager_t::recursiveInstructionSearch(uint64_t instructionAddress, int32_t *cache_indexes,
                                                     uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    uint32_t cache_status = this->searchAddress(instructionAddress, &this->instruction_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level != 0) {
            for (int32_t i = INSTRUCTION_LEVELS - 1; i >= 0; i--) {
                this->instruction_cache[cache_level][cache_indexes[cache_level]].returnLine(instructionAddress, &this->instruction_cache[i][cache_indexes[i]]);
            }
        }
        return latency_request;
    }

    this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == INSTRUCTION_LEVELS - 1) {
        return recursiveDataSearch(NULL, instructionAddress, cache_indexes, latency_request, ttc, cache_level + 1, INSTRUCTION);
    }
    return recursiveInstructionSearch(instructionAddress, cache_indexes, latency_request, ttc, cache_level + 1);
}

// Searches an address in instruction cache, and lower data caches
uint32_t cache_manager_t::searchInstruction(uint32_t processor_id, uint64_t instructionAddress) {
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
    uint32_t cache_status = this->searchAddress(instructionAddress, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    if (cache_status == HIT) {
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        for (int32_t i = cache_level - 1; i >= 0; i--) {
            this->data_cache[i+1][cache_indexes[i+1]].returnLine(instructionAddress, &this->data_cache[i][cache_indexes[i]]);
        }
        // }
        // if (cache_level == DATA_LEVELS - 1 && cache_type == DATA) {
        //     #if PREFETCHER_ACTIVE
        //         this->prefetcher->prefecht(mob_line, &this->data_cache[cache_level][cache_indexes[cache_level]]);
        //     #endif
        // }
        if (mob_line != NULL) {
            mob_line->updatePackageReady(latency_request);
        }
        return latency_request;
    }
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
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
    delete[] cache_indexes;
    return result;
}

uint32_t cache_manager_t::recursiveDataWrite(memory_order_buffer_line_t *mob_line, int32_t *cache_indexes,
                                             uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type) {
    uint32_t cache_status = this->searchAddress(mob_line->memory_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        for (int32_t i = cache_level - 1; i >= 0; i--) {
            this->data_cache[i+1][cache_indexes[i+1]].returnLine(mob_line->memory_address, &this->data_cache[i][cache_indexes[i]]);
        }
        this->data_cache[0][cache_indexes[0]].write(mob_line->memory_address);
        mob_line->updatePackageReady(latency_request);
        return latency_request;
    }
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
