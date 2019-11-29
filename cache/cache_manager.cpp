#include "./../simulator.hpp"

#ifdef CACHE_DEBUG
    #define CACHE_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define CACHE_DEBUG_PRINTF(...)
#endif

// Constructor
cache_manager_t::cache_manager_t() {}

// Desctructor
cache_manager_t::~cache_manager_t() {
    for (size_t i = 0; i < this->mshr_table.size(); i++){
        delete mshr_table[i];
    }

    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) delete[] instruction_cache[i];
    for (uint32_t i = 0; i < DATA_LEVELS; i++) delete[] data_cache[i];

    delete[] ICACHE_AMOUNT;
    delete[] DCACHE_AMOUNT;

    delete[] data_cache;
    delete[] instruction_cache;
    // delete[] directory;
    std::vector<memory_package_t *>().swap(mshr_table);
}

void cache_manager_t::check_cache(uint32_t cache_size, uint32_t cache_level) {
    ERROR_ASSERT_PRINTF(utils_t::check_if_power_of_two(cache_size) == OK, "Error - Cache Size Array must be power of 2 value\n")
    if (cache_level == 0) {
        ERROR_ASSERT_PRINTF(NUMBER_OF_PROCESSORS == cache_size, "Error - # First level instruction Caches must be equal # PROCESSORS\n")
    } else {
        ERROR_ASSERT_PRINTF(cache_size <= NUMBER_OF_PROCESSORS,"Error - # Other level instruction Caches must be less or equal # PROCESSORS \n")
    }
}

void cache_manager_t::get_cache_levels(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs) {
    const char *string_cache_type;
    if (cache_type == 0) {
        string_cache_type = "INSTRUCTION";
    } else {
        string_cache_type = "DATA";
    }

    vector<uint32_t> clevels, camount;
    libconfig::Setting &cfg_caches = cfg_cache_defs[string_cache_type];
    uint32_t N_CACHES = cfg_caches.getLength();
    for (uint32_t i = 0; i < N_CACHES; i++) {
        try {
            libconfig::Setting &cfg_cache = cfg_caches[i];
            clevels.push_back(cfg_cache["LEVEL"]);
        } catch (libconfig::SettingNotFoundException const(&nfex)) {
            ERROR_PRINTF("MISSING CACHE PARAMETERS");
        } catch (libconfig::SettingTypeException const(&tex)) {
            ERROR_PRINTF("WRONG TYPE CACHE PARAMETERS");
        }
    }
    camount = clevels;

    std::sort(clevels.begin(), clevels.end());
    clevels.erase(std::unique(clevels.begin(), clevels.end()), clevels.end());

    if (cache_type == 0) {
        set_INSTRUCTION_LEVELS(clevels.size());
        ICACHE_AMOUNT = new uint32_t[INSTRUCTION_LEVELS];
        for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
            ICACHE_AMOUNT[i] = std::count(camount.begin(), camount.end(), i);
        }
    } else {
        set_DATA_LEVELS(clevels.size());
        DCACHE_AMOUNT = new uint32_t[DATA_LEVELS];
        for (uint32_t i = 0; i < DATA_LEVELS; i++) {
            DCACHE_AMOUNT[i] = std::count(camount.begin(), camount.end(), i);
        }
        POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
    }
    std::vector<uint32_t>().swap(clevels);
    std::vector<uint32_t>().swap(camount);
}

void cache_manager_t::get_cache_info(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs, cache_t *cache, uint32_t cache_level, uint32_t CACHE_AMOUNT) {
    const char *string_cache_type;
    if (cache_type == 0) {
        string_cache_type = "INSTRUCTION";
    } else {
        string_cache_type = "DATA";
    }

    libconfig::Setting &cfg_caches = cfg_cache_defs[string_cache_type];

    // Get information of each cache
    for (uint32_t j = 0; j < CACHE_AMOUNT; j ++) {
        for (int32_t i = 0; i < cfg_caches.getLength(); i++) {
            try {
                libconfig::Setting &cfg_cache = cfg_caches[i];
                if ((uint32_t)cfg_cache["LEVEL"] == cache_level) {
                    cache[j].id = cache_type;
                    cache[j].size = cfg_cache["SIZE"];
                    cache[j].level = cfg_cache["LEVEL"];
                    cache[j].offset = utils_t::get_power_of_two(LINE_SIZE);
                    cache[j].latency = cfg_cache["LATENCY"];
                    cache[j].associativity = cfg_cache["ASSOCIATIVITY"];
                    cache[j].n_sets = (cache[j].size / LINE_SIZE) / cache[j].associativity;
                }
                
            } catch (libconfig::SettingNotFoundException const(&nfex)) {
                ERROR_PRINTF("MISSING CACHE PARAMETERS");
            } catch (libconfig::SettingTypeException const(&tex)) {
                ERROR_PRINTF("WRONG TYPE CACHE PARAMETERS");
            }
        }
    }
}

cache_t **cache_manager_t::instantiate_cache(cacheId_t cache_type, libconfig::Setting &cfg_cache_defs) {
    this->get_cache_levels(cache_type, cfg_cache_defs);

    uint32_t CACHE_LEVELS, *CACHE_AMOUNT;
    if (cache_type == INSTRUCTION) {
        set_INSTRUCTION_LEVELS(INSTRUCTION_LEVELS);
        CACHE_LEVELS = INSTRUCTION_LEVELS;
        CACHE_AMOUNT = new uint32_t[CACHE_LEVELS];
        for (uint32_t i = 0; i < CACHE_LEVELS; i++) {
            CACHE_AMOUNT[i] = ICACHE_AMOUNT[i];
        }
    } else {
        set_DATA_LEVELS(DATA_LEVELS);
        CACHE_LEVELS = DATA_LEVELS;
        CACHE_AMOUNT = new uint32_t[CACHE_LEVELS];
        for (uint32_t i = 0; i < CACHE_LEVELS; i++) {
            CACHE_AMOUNT[i] = DCACHE_AMOUNT[i];
        }
    }

    cache_t **cache = new cache_t *[CACHE_LEVELS];
    for (uint32_t i = 0; i < CACHE_LEVELS; i++) {
        cache[i] = new cache_t[CACHE_AMOUNT[i]];
        this->get_cache_info(cache_type, cfg_cache_defs, cache[i], i, CACHE_AMOUNT[i]);
        this->check_cache(CACHE_AMOUNT[i], i);
        for (uint32_t j = 0; j < CACHE_AMOUNT[i]; j++) {
            cache[i][j].allocate(INSTRUCTION_LEVELS, DATA_LEVELS);
        }
    }
    delete[] CACHE_AMOUNT;
    return cache;
}

void cache_manager_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
    // Access configure file
    set_NUMBER_OF_PROCESSORS(NUMBER_OF_PROCESSORS);

    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_DEBUG (cfg_processor["DEBUG"]);

    // Get prefetcher info
    libconfig::Setting &prefetcher_defs = cfg_root["PREFETCHER"];
    set_PREFETCHER_ACTIVE(prefetcher_defs["PREFETCHER_ACTIVE"]);

    // Get general cache info
    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    set_CACHE_MANAGER_DEBUG(cfg_cache_defs["CONFIG"]["CACHE_MANAGER_DEBUG"]);
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_WAIT_CYCLE(cfg_cache_defs["CONFIG"]["WAIT_CYCLE"]);

    set_MAX_PARALLEL_REQUESTS_CORE(cfg_root["MEMORY_CONTROLLER"]["MAX_PARALLEL_REQUESTS_CORE"]);

    this->instruction_cache = this->instantiate_cache(INSTRUCTION, cfg_cache_defs);
    this->data_cache = this->instantiate_cache(DATA, cfg_cache_defs);

    set_POINTER_LEVELS((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);

    this->directory = new directory_t[NUMBER_OF_PROCESSORS];
    for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        this->directory[i].allocate(this->data_cache[2][0], POINTER_LEVELS);
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
void cache_manager_t::installCacheLines(uint64_t instructionAddress, int32_t *cache_indexes, uint32_t latency_request, memory_operation_t mem_op) {
    uint32_t i, j, idx; 
    int32_t way;
    way_t ***cache_line = new way_t**[NUMBER_OF_PROCESSORS];
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        cache_line[i] = new way_t*[POINTER_LEVELS];
        for (j = 0; j < POINTER_LEVELS; j++) {
            cache_line[i][j] = NULL;
        }
    }

    if (mem_op == MEMORY_OPERATION_INST) {
        for (i = 0; i < INSTRUCTION_LEVELS; i++) {
            cache_line[0][i] = this->instruction_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request, *this->directory, idx, way, mem_op);
        }
    } else {
        i = 0;
    }
    for (; i < POINTER_LEVELS; i++) {
        cache_line[0][i] = this->data_cache[i][cache_indexes[i]].installLine(instructionAddress, latency_request, *this->directory, idx, way, mem_op);
    }

    directory->installCachePointers(cache_line, NUMBER_OF_PROCESSORS, POINTER_LEVELS, idx, way, mem_op);

    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            cache_line[i][j] = NULL;
        }
    }
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        delete[] cache_line[i];
    }
    delete[] cache_line;
}

void cache_manager_t::add_mshr_entry (memory_package_t* mob_line, uint64_t latency_request){
    mob_line->latency = latency_request;
    mshr_table.push_back (mob_line);
    if (!mob_line->is_hive) orcs_engine.memory_controller->add_requests_llc();
}

bool cache_manager_t::isInMSHR (memory_package_t* mob_line){
    if (mob_line->is_hive) return false;
    uint64_t tag = (mob_line->memory_address >> this->offset);
    for (std::size_t i = 0; i < mshr_table.size(); i++){
        if ((mshr_table[i]->memory_address >> this->offset) == tag) {
            CACHE_DEBUG_PRINTF("Is in MSHR!\n");
            for (size_t j = 0; j < mob_line->clients.size(); j++){
                mshr_table[i]->clients.push_back (mob_line->clients[j]);
            }
            delete mob_line;
            return true;
        }
    }
    return false;
}

void cache_manager_t::print_mshr_table(){
    for (size_t i = 0; i < mshr_table.size(); i++){
        ORCS_PRINTF ("MSHR Table entry %lu: %s %lu %s.\n", i, get_enum_memory_operation_char (mshr_table[i]->memory_operation), mshr_table[i]->uop_number, get_enum_package_state_char (mshr_table[i]->status))
    }
}

void cache_manager_t::clock() {
    if (mshr_table.size() > 0) {
        mshr_index += 1;
        if (mshr_index >= this->MAX_PARALLEL_REQUESTS_CORE || mshr_index >= mshr_table.size()) mshr_index = 0;
        if (mshr_table[mshr_index]->status == PACKAGE_STATE_READY) {
            int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
            this->generateIndexArray(mshr_table[mshr_index]->processor_id, cache_indexes);
            this->installCacheLines(mshr_table[mshr_index]->memory_address, cache_indexes, mshr_table[mshr_index]->latency, mshr_table[mshr_index]->memory_operation);
            mshr_table[mshr_index]->updatePackageReady (mshr_table[mshr_index]->latency);
            delete mshr_table[mshr_index];
            mshr_table.erase (std::remove (mshr_table.begin(), mshr_table.end(), mshr_table[mshr_index]), mshr_table.end());
            delete[] cache_indexes;
        }
        else if (mshr_table[mshr_index]->status == PACKAGE_STATE_UNTREATED){
            switch (mshr_table[mshr_index]->memory_operation){
                case MEMORY_OPERATION_FREE:
                    break;
                case MEMORY_OPERATION_READ:
                case MEMORY_OPERATION_WRITE:
                case MEMORY_OPERATION_INST:
                    orcs_engine.memory_controller->requestDRAM(mshr_table[mshr_index], mshr_table[mshr_index]->memory_address);
                    mshr_table[mshr_index]->status = PACKAGE_STATE_TRANSMIT;
                    break;
                case MEMORY_OPERATION_HIVE_LOCK:
                case MEMORY_OPERATION_HIVE_UNLOCK:
                case MEMORY_OPERATION_HIVE_INT_ALU:
                case MEMORY_OPERATION_HIVE_INT_DIV:
                case MEMORY_OPERATION_HIVE_INT_MUL:
                case MEMORY_OPERATION_HIVE_FP_ALU:
                case MEMORY_OPERATION_HIVE_FP_DIV:
                case MEMORY_OPERATION_HIVE_FP_MUL:
                case MEMORY_OPERATION_HIVE_LOAD:
                case MEMORY_OPERATION_HIVE_STORE:
                    if (orcs_engine.hive_controller->addRequest (mshr_table[mshr_index])){
                        if (DEBUG) ORCS_PRINTF ("Cache Manager clock(): sending HIVE instruction %lu, %s to memory.\n", mshr_table[mshr_index]->uop_number, get_enum_memory_operation_char (mshr_table[mshr_index]->memory_operation))
                    }                    
                    break;
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

uint32_t cache_manager_t::llcMiss(memory_package_t* mob_line, uint32_t latency_request) {
    this->add_mshr_entry (mob_line, latency_request);
    return latency_request;
}

// Searches an instruction in cache levels
uint32_t cache_manager_t::recursiveInstructionSearch(memory_package_t* mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    uint32_t cache_status = this->searchAddress(mob_line->opcode_address, &this->instruction_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    if (cache_status == HIT) {
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level > 0) {
            for (int32_t i = INSTRUCTION_LEVELS - 1; i >= 0; i--) {
                this->instruction_cache[cache_level][cache_indexes[cache_level]].returnLine(mob_line->opcode_address, &this->instruction_cache[i][cache_indexes[i]], *this->directory, mob_line->memory_operation);
            }
        }
        mob_line->updatePackageReady(latency_request);
        delete mob_line;
        return 0;
    }
    this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == INSTRUCTION_LEVELS - 1) {
        return recursiveDataSearch(mob_line, cache_indexes, latency_request, ttc, cache_level + 1);
    }
    return recursiveInstructionSearch(mob_line, cache_indexes, latency_request, ttc, cache_level + 1);
}

uint32_t cache_manager_t::recursiveDataSearch(memory_package_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    uint32_t cache_status = this->searchAddress(mob_line->opcode_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    if (cache_status == HIT) {
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level > 0) {
            for (int32_t i = cache_level - 1; i >= 0; i--) {
                this->data_cache[i + 1][cache_indexes[i + 1]].returnLine(mob_line->opcode_address, &this->data_cache[i][cache_indexes[i]], *this->directory, mob_line->memory_operation);
                // this->data_cache[i + 1][cache_indexes[i + 1]].add_cache_write();
            }
        }
        mob_line->updatePackageReady(latency_request);
        delete mob_line;
        return latency_request;
    }
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        if (mob_line->memory_operation == MEMORY_OPERATION_READ || mob_line->memory_operation == MEMORY_OPERATION_WRITE) {
            for (size_t i = 0; i < mob_line->clients.size(); i++) {
                mob_line->clients[i]->waiting_DRAM = true;
            }
            orcs_engine.processor[mob_line->processor_id].request_DRAM++;
        }
        return llcMiss(mob_line, latency_request);
    }
    return recursiveDataSearch(mob_line, cache_indexes, latency_request, ttc, cache_level + 1);
}

uint32_t cache_manager_t::searchData(memory_package_t *mob_line) {
    CACHE_DEBUG_PRINTF("\n\n\nREQUESTED DATA %lu\n", mob_line->memory_address);
    uint32_t ttc = 0, latency_request = 0;
    if (!isInMSHR(mob_line)){
        CACHE_DEBUG_PRINTF("Is NOT in MSHR! ");
        int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
        this->generateIndexArray(mob_line->processor_id, cache_indexes);
        switch (mob_line->memory_operation){
            case MEMORY_OPERATION_READ:
                CACHE_DEBUG_PRINTF("Is a READ\n");
                recursiveDataSearch(mob_line, cache_indexes, latency_request, ttc, 0);
                break;
            case MEMORY_OPERATION_HIVE_FP_ALU:
            case MEMORY_OPERATION_HIVE_FP_DIV:
            case MEMORY_OPERATION_HIVE_FP_MUL:
            case MEMORY_OPERATION_HIVE_INT_ALU:
            case MEMORY_OPERATION_HIVE_INT_DIV:
            case MEMORY_OPERATION_HIVE_INT_MUL:
            case MEMORY_OPERATION_HIVE_LOAD:
            case MEMORY_OPERATION_HIVE_STORE:
            case MEMORY_OPERATION_HIVE_LOCK:
            case MEMORY_OPERATION_HIVE_UNLOCK:
                llcMiss (mob_line, latency_request);
                break;
            case MEMORY_OPERATION_WRITE:
                CACHE_DEBUG_PRINTF("Is a WRITE\n");
                recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, 0);
                break;
            case MEMORY_OPERATION_INST:
                CACHE_DEBUG_PRINTF("Is an INSTRUCTION\n");
                recursiveInstructionSearch (mob_line, cache_indexes, latency_request, ttc, 0);
                break;
            default:
                break;
        }
        delete[] cache_indexes;
    }
    return 0;
}

uint32_t cache_manager_t::recursiveDataWrite(memory_package_t *mob_line, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    uint32_t cache_status = this->searchAddress(mob_line->memory_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        if (cache_level > 0) {
            for (int32_t i = cache_level - 1; i >= 0; i--) {
                this->data_cache[i + 1][cache_indexes[i + 1]].returnLine(mob_line->memory_address, &this->data_cache[i][cache_indexes[i]], *this->directory, mob_line->memory_operation);
            }
        }
        mob_line->updatePackageReady(latency_request);
        delete mob_line;
        return latency_request;
    }
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        latency_request = llcMiss(mob_line, latency_request);
        return latency_request;
    }
    return recursiveDataWrite(mob_line, cache_indexes, latency_request, ttc, cache_level + 1);
}

void cache_manager_t::statistics(uint32_t core_id) {
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS];
    bool close = false;
    FILE *output = stdout;
	if (orcs_engine.output_file_name != NULL) {
		output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
	if (output != NULL) {
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
