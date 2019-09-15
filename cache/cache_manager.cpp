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
                printf("v_levels[%d] = %u\n", i, v_levels[i]);
                if (j < v_levels[i]) {
                    j++;
                }
            }
        }
    }
}

cache_t *cache_manager_t::get_cache_info(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs, uint32_t *N_CACHES) {
    const char *string_cache_type;
    if (cache_type == 0) {
        string_cache_type = "INSTRUCTION";
        // strcpy(string_cache_type, "INSTRUCTION");
    } else {
        string_cache_type = "DATA";
        // strcpy(string_cache_type, "DATA");
    }

    // Get the list of caches
    libconfig::Setting &cfg_caches = cfg_cache_defs[string_cache_type];
    *N_CACHES = cfg_caches.getLength();
    cache_t *caches = new cache_t[*N_CACHES];

    // Get information of each cache
    for (uint32_t i = 0; i < *N_CACHES; i++) {
        libconfig::Setting &cfg_cache = cfg_caches[i];
        try {
            caches[i].id = cache_type;
            caches[i].size = cfg_cache["SIZE"];
            caches[i].level = cfg_cache["LEVEL"];
            caches[i].offset = utils_t::get_power_of_two(LINE_SIZE);
            caches[i].latency = cfg_cache["LATENCY"];
            caches[i].associativity = cfg_cache["ASSOCIATIVITY"];
            caches[i].n_sets = (caches[i].size / LINE_SIZE) / caches[i].associativity;
        } catch (libconfig::SettingNotFoundException const(&nfex)) {
            ERROR_PRINTF("MISSING CACHE PARAMETERS");
        } catch (libconfig::SettingTypeException const(&tex)) {
            ERROR_PRINTF("WRONG TYPE CACHE PARAMETERS");
        }
    }

    // Get the amount of caches in each level
    vector<uint32_t> cache_levels;
    
    if (cache_type == 0) {
        ICACHE_AMOUNT = this->get_cache_levels(cache_levels, caches, *N_CACHES);
        set_INSTRUCTION_LEVELS(cache_levels.size());
    } else {
        DCACHE_AMOUNT = this->get_cache_levels(cache_levels, caches, *N_CACHES);
        set_DATA_LEVELS(cache_levels.size());
        POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
    }
    return caches;
}

void cache_manager_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
    // Access configure file
    set_NUMBER_OF_PROCESSORS(NUMBER_OF_PROCESSORS);

    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();

    // Get prefetcher info
    libconfig::Setting &prefetcher_defs = cfg_root[0]["PREFETCHER"];
    set_PREFETCHER_ACTIVE(prefetcher_defs["PREFETCHER_ACTIVE"]);

    // Get general cache info
    libconfig::Setting &cfg_cache_defs = cfg_root[0]["CACHE_MEMORY"];
    set_CACHE_MANAGER_DEBUG(cfg_cache_defs["CONFIG"]["CACHE_MANAGER_DEBUG"]);
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_WAIT_CYCLE(cfg_cache_defs["CONFIG"]["WAIT_CYCLE"]);

    set_MAX_PARALLEL_REQUESTS_CORE (cfg_root[0]["MEMORY_CONTROLLER"]["MAX_PARALLEL_REQUESTS_CORE"]);

    // Get the list of intruction caches
    uint32_t INSTRUCTION_CACHES;
    cache_t *instruction_caches = get_cache_info(INSTRUCTION, cfg_cache_defs, &INSTRUCTION_CACHES);
    instruction_cache = new cache_t *[INSTRUCTION_LEVELS];
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        this->instruction_cache[i] = new cache_t[ICACHE_AMOUNT[i]];
        check_cache(ICACHE_AMOUNT[i], i);
    }
    copy_cache(this->instruction_cache, instruction_caches, INSTRUCTION_LEVELS, ICACHE_AMOUNT, INSTRUCTION_CACHES);
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
        for (uint32_t j = 0; j < ICACHE_AMOUNT[i]; j++) {
            this->instruction_cache[i][j].allocate(NUMBER_OF_PROCESSORS);
        }
    }

    // Get the list of data caches
    uint32_t DATA_CACHES;
    cache_t *data_caches = get_cache_info(DATA, cfg_cache_defs, &DATA_CACHES);
    data_cache = new cache_t *[DATA_LEVELS];
    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        this->data_cache[i] = new cache_t[DCACHE_AMOUNT[i]];
        check_cache(DCACHE_AMOUNT[i], i);
    }
    copy_cache(this->data_cache, data_caches, DATA_LEVELS, DCACHE_AMOUNT, DATA_CACHES);
    for (uint32_t i = 0; i < DATA_LEVELS; i++) {
        for (uint32_t j = 0; j < DCACHE_AMOUNT[i]; j++) {
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
    if (PREFETCHER_ACTIVE) {
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
            if (mshr_table[mshr_index]->requests[0]->memory_operation == MEMORY_OPERATION_INST){
                this->installCacheLines(mshr_table[mshr_index]->requests[0]->memory_address, cache_indexes, mshr_table[mshr_index]->latency, INSTRUCTION);
            } else this->installCacheLines(mshr_table[mshr_index]->requests[0]->memory_address, cache_indexes, mshr_table[mshr_index]->latency, DATA);
            if (mshr_table[mshr_index]->requests[0]->memory_operation == MEMORY_OPERATION_WRITE){
                int cache_level = DATA_LEVELS - 1;
                for (int32_t k = cache_level - 1; k >= 0; k--) {
                    this->data_cache[k+1][cache_indexes[k+1]].returnLine(mshr_table[mshr_index]->requests[0]->memory_address, &this->data_cache[k][cache_indexes[k]]);
                    this->data_cache[k+1][cache_indexes[k+1]].add_cache_write();
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
}

uint32_t cache_manager_t::searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc) {
    uint32_t cache_status = cache->read(instructionAddress, *ttc);
    cache->add_cache_access();
    *latency_request += *ttc;
    return cache_status;
}

uint32_t cache_manager_t::llcMiss(memory_order_buffer_line_t* mob_line, uint32_t latency_request) {
    this->add_mshr_entry (mob_line, latency_request);
    return latency_request;
}

// Searches an instruction in cache levels
uint32_t cache_manager_t::recursiveInstructionSearch(memory_order_buffer_line_t* mob_line, int32_t *cache_indexes,
                                                     uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    uint32_t cache_status = this->searchAddress(mob_line->opcode_address, &this->instruction_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
        if (cache_level != 0) {
            for (int32_t i = INSTRUCTION_LEVELS - 1; i >= 0; i--) {
                this->instruction_cache[cache_level][cache_indexes[cache_level]].returnLine(mob_line->opcode_address, &this->instruction_cache[i][cache_indexes[i]]);
            }
        }
        mob_line->updatePackageReady (latency_request);
        return 0;
    }

    this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == INSTRUCTION_LEVELS - 1) {
        return recursiveDataSearch(mob_line, mob_line->opcode_address, cache_indexes, latency_request, ttc, cache_level + 1, INSTRUCTION);
    }
    return recursiveInstructionSearch(mob_line, cache_indexes, latency_request, ttc, cache_level + 1);
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
            this->data_cache[i+1][cache_indexes[i+1]].add_cache_write();
        }
        mob_line->updatePackageReady(latency_request);
        return latency_request;
    }
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        if (cache_type == DATA) {
            mob_line->waiting_DRAM = true;
            orcs_engine.processor[mob_line->processor_id].request_DRAM++;
        }
        return llcMiss(mob_line, latency_request);
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
        } else if (mob_line->memory_operation == MEMORY_OPERATION_WRITE) {
            recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, 0, DATA);
        } else if (mob_line->memory_operation == MEMORY_OPERATION_INST) {
            recursiveInstructionSearch (mob_line, cache_indexes, latency_request, ttc, 0);
        }
        delete[] cache_indexes;
    }
    return 0;
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
        latency_request = llcMiss(mob_line, latency_request);
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
