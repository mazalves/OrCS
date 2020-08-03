#include "./../simulator.hpp"

// Constructor
cache_manager_t::cache_manager_t() {
    this->i = 0;

    this->reads = 0;
    this->read_miss = 0;
    this->read_hit = 0;
    this->writes = 0;
    this->write_miss = 0;
    this->write_hit = 0;
    this->offset = 0;
    this->mshr_index = 0;
    this->op_count = NULL;
    this->op_max = NULL;

    this->sent_ram = 0;
    this->sent_ram_cycles = 0;
    this->sent_hive = 0;
    this->sent_hive_cycles = 0;
    this->sent_vima = 0;
    this->sent_vima_cycles = 0;

    this->LINE_SIZE = 0;
    this->PREFETCHER_ACTIVE = 0;
    this->DATA_LEVELS = 0;
    this->INSTRUCTION_LEVELS = 0;
    this->POINTER_LEVELS = 0;
    this->LLC_CACHES = 0;
    this->CACHE_MANAGER_DEBUG = 0;
    this->WAIT_CYCLE = 0;
    this->DEBUG = 0;

    this->NUMBER_OF_PROCESSORS = 0;
    this->MAX_PARALLEL_REQUESTS_CORE = 0;

    data_cache = NULL;
    instruction_cache = NULL;
    ICACHE_AMOUNT = NULL;
    DCACHE_AMOUNT = NULL;
}

// Desctructor
cache_manager_t::~cache_manager_t() {
    for (i = 0; i < this->requests.size(); i++) delete requests[i];
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) delete[] this->op_count[i];
    delete[] op_count;

    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) delete[] instruction_cache[i];
    for (uint32_t i = 0; i < DATA_LEVELS; i++) delete[] data_cache[i];

    delete[] ICACHE_AMOUNT;
    delete[] DCACHE_AMOUNT;

    delete[] data_cache;
    delete[] instruction_cache;

    delete[] op_max;

    std::vector<memory_package_t *>().swap(requests);
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
        } catch (libconfig::SettingNotFoundException const &nfex) {
            ERROR_PRINTF("MISSING CACHE PARAMETERS");
        } catch (libconfig::SettingTypeException const &tex) {
            ERROR_PRINTF("WRONG TYPE CACHE PARAMETERS");
        }
    }
    camount = clevels;

    std::sort(clevels.begin(), clevels.end());
    clevels.erase(std::unique(clevels.begin(), clevels.end()), clevels.end());

    if (cache_type == 0) {
        set_INSTRUCTION_LEVELS(N_CACHES);
        ICACHE_AMOUNT = new uint32_t[INSTRUCTION_LEVELS]();
        for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
            ICACHE_AMOUNT[i] = std::count(camount.begin(), camount.end(), i);
        }
    } else {
        set_DATA_LEVELS(N_CACHES);
        DCACHE_AMOUNT = new uint32_t[DATA_LEVELS]();
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
                    cache[j].set_LINE_SIZE (LINE_SIZE);
                }
                
            } catch (libconfig::SettingNotFoundException const &nfex) {
                ERROR_PRINTF("MISSING CACHE PARAMETERS");
            } catch (libconfig::SettingTypeException const &tex) {
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
        CACHE_AMOUNT = new uint32_t[CACHE_LEVELS]();
        for (uint32_t i = 0; i < CACHE_LEVELS; i++) {
            CACHE_AMOUNT[i] = ICACHE_AMOUNT[i];
            //ORCS_PRINTF ("ICACHE_AMOUNT[%u] = %u\n", i, ICACHE_AMOUNT[i])
        }
    } else {
        set_DATA_LEVELS(DATA_LEVELS);
        CACHE_LEVELS = DATA_LEVELS;
        CACHE_AMOUNT = new uint32_t[CACHE_LEVELS]();
        for (uint32_t i = 0; i < CACHE_LEVELS; i++) {
            CACHE_AMOUNT[i] = DCACHE_AMOUNT[i];
            //ORCS_PRINTF ("DCACHE_AMOUNT[%u] = %u\n", i, ICACHE_AMOUNT[i])
        }
    }

    // ORCS_PRINTF ("CACHE_LEVELS = %u\n", CACHE_LEVELS)
    cache_t **cache = new cache_t *[CACHE_LEVELS]();
    for (uint32_t i = 0; i < CACHE_LEVELS; i++) {
        cache[i] = new cache_t[CACHE_AMOUNT[i]]();
        this->get_cache_info(cache_type, cfg_cache_defs, cache[i], i, CACHE_AMOUNT[i]);
        this->check_cache(CACHE_AMOUNT[i], i);
        for (uint32_t j = 0; j < CACHE_AMOUNT[i]; j++) {
            cache[i][j].allocate(NUMBER_OF_PROCESSORS, INSTRUCTION_LEVELS, DATA_LEVELS);
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

    //Read/Write counters
    this->set_reads(0);
    this->set_read_hit(0);
    this->set_read_miss(0);
    this->set_writes(0);
    this->set_write_hit(0);
    this->set_write_miss(0);
    this->set_offset(utils_t::get_power_of_two(LINE_SIZE));
    this->set_sent_ram(0);
    this->set_sent_ram_cycles(0);
    this->set_sent_hive(0);
    this->set_sent_hive_cycles(0);
    this->set_sent_vima(0);
    this->set_sent_vima_cycles(0);

    this->op_count = new uint64_t*[NUMBER_OF_PROCESSORS]();
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++){
        this->op_count[i] = new uint64_t[MEMORY_OPERATION_LAST]();
    }
    
    this->op_max = new uint64_t[MEMORY_OPERATION_LAST]();
    for (i = 0; i < MEMORY_OPERATION_LAST; i++) this->op_max[i] = UINT64_MAX;
    op_max[MEMORY_OPERATION_READ] = this->get_MAX_PARALLEL_REQUESTS_CORE();
    op_max[MEMORY_OPERATION_WRITE] = this->get_MAX_PARALLEL_REQUESTS_CORE();

    //Allocate Prefetcher
    if (PREFETCHER_ACTIVE) {
        this->prefetcher = new prefetcher_t();
        this->prefetcher->allocate(NUMBER_OF_PROCESSORS);
    }
}

// Dependending on processor_id, returns its correspondent cache
void cache_manager_t::generateIndexArray(uint32_t processor_id, int32_t *cache_indexes) {
    for (i = 0; i < POINTER_LEVELS; i++) {
        cache_indexes[i] = (int32_t) processor_id & (DCACHE_AMOUNT[i] - 1);
    }
}

// Install an address in every cache using pointers
void cache_manager_t::installCacheLines(memory_package_t* request, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type) {
    // printf("installCacheLines %lu in processor %u\n", request->memory_address, request->processor_id);
    uint32_t i, j;
    uint64_t llc_idx, llc_line;
    uint64_t *CACHE_TAGS = new uint64_t[POINTER_LEVELS]();
    line_t ***line = new line_t**[NUMBER_OF_PROCESSORS]();
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        line[i] = new line_t*[POINTER_LEVELS]();
        for (j = 0; j < POINTER_LEVELS; j++) {
            line[i][j] = NULL;
        }
    }
    if (cache_type == INSTRUCTION) {
        for (i = 0; i < INSTRUCTION_LEVELS; i++) {
            // printf("cache[%d][%d]\n", i, cache_indexes[i]);
            line[request->processor_id][i] = this->instruction_cache[i][cache_indexes[i]].installLine(request, latency_request, llc_idx, llc_line);
            //ORCS_PRINTF ("INST Installed addr: %lu, level: %u ", instructionAddress, i)
            //this->instruction_cache[i][cache_indexes[i]].printTagIdx (instructionAddress);
        }
    } else {
        i = 0;
    }
    for (; i < POINTER_LEVELS; i++) {
        // printf("cache[%d][%d]\n", i, cache_indexes[i]);
        line[request->processor_id][i] = this->data_cache[i][cache_indexes[i]].installLine(request, latency_request, llc_idx, llc_line);
        //ORCS_PRINTF ("DATA Installed addr: %lu, level: %u ", instructionAddress, i)
        //this->data_cache[i][cache_indexes[i]].printTagIdx (instructionAddress);
    }

    for (i = 0; i < POINTER_LEVELS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            if (i == j) {
                continue;
            }
            line[request->processor_id][i]->line_ptr_caches[request->processor_id][j] = line[request->processor_id][j];
        }
    }
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) {
        for (j = 0; j < POINTER_LEVELS; j++) {
            line[i][j] = NULL;
        }
    }
    for (i = 0; i < NUMBER_OF_PROCESSORS; i++) delete[] line[i];
    delete[] line;
    delete[] CACHE_TAGS;
}

bool cache_manager_t::isIn (memory_package_t* request){
    if (request->is_hive || request->is_vima) return false;
    uint64_t tag = (request->memory_address >> this->offset);
    //ORCS_PRINTF ("%lu %s\n", tag, get_enum_memory_operation_char (request->memory_operation))
    for (i = 0; i < requests.size(); i++){
        if ((requests[i]->memory_address >> this->offset) == tag && requests[i]->type == request->type) {
            //ORCS_PRINTF ("%s %s\n", get_enum_memory_operation_char(request->memory_operation), get_enum_memory_operation_char (requests[i]->memory_operation))
            //ORCS_PRINTF ("HIT! %lu %lu %s\n", requests[i]->memory_address >> this->offset, tag, get_enum_memory_operation_char (request->memory_operation))
            for (uint64_t j = 0; j < MEMORY_OPERATION_LAST; j++) requests[i]->op_count[j] += request->op_count[j];
            for (size_t j = 0; j < request->clients.size(); j++) requests[i]->clients.push_back (request->clients[j]);
            delete request;
            return true;
        }
    }
    return false;
}

void cache_manager_t::print_requests(){
    ORCS_PRINTF ("-------------%lu------------\n", orcs_engine.get_global_cycle())
    for (size_t i = 0; i < requests.size(); i++){
        ORCS_PRINTF ("MSHR Table entry %lu: mop %s | uop %lu | status %s.\n", i, get_enum_memory_operation_char (requests[i]->memory_operation), requests[i]->uop_number, get_enum_package_state_char (requests[i]->status))
    }
}

void cache_manager_t::finishRequest (memory_package_t* request){
    for (uint64_t i = 0; i < MEMORY_OPERATION_LAST; i++){
        this->op_count[request->processor_id][i] = this->op_count[request->processor_id][i] - request->op_count[i];
    }

    if (request->sent_to_ram && !request->is_hive && !request->is_vima){
        sent_ram++;
        sent_ram_cycles += (orcs_engine.get_global_cycle() - request->born_cycle);
    } else if (request->is_hive){
        sent_hive++;
        sent_hive_cycles += (orcs_engine.get_global_cycle() - request->born_cycle);
    } else if (request->is_vima){
        sent_vima++;
        sent_vima_cycles += (orcs_engine.get_global_cycle() - request->born_cycle);
    }

    //if (request->memory_operation != MEMORY_OPERATION_INST) ORCS_PRINTF ("%lu request %s %lu born at %lu, finished at %lu. Took %lu cycles.\n", orcs_engine.get_global_cycle(), get_enum_memory_operation_char(request->memory_operation), request->uop_number, request->born_cycle, orcs_engine.get_global_cycle(), orcs_engine.get_global_cycle()-request->born_cycle)

    request->updatePackageReady();
    request->updateClients();
    requests.erase (std::remove (requests.begin(), requests.end(), request), requests.end());
    //if (request->memory_operation != MEMORY_OPERATION_INST) ORCS_PRINTF ("%lu Cache Manager: finished memory request %lu from uop %lu, %s.\n", orcs_engine.get_global_cycle(), request->memory_address, request->uop_number, get_enum_memory_operation_char (request->memory_operation))
    delete request;
}

void cache_manager_t::install (memory_package_t* request){
    // printf("installCache %lu in processor %u\n", request->memory_address, request->processor_id);
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
    this->generateIndexArray(request->processor_id, cache_indexes);
    switch (request->memory_operation){
        case MEMORY_OPERATION_INST:{
            // printf("instruction");
            this->installCacheLines(request, cache_indexes, 0, INSTRUCTION);
            break;
        }
        case MEMORY_OPERATION_READ:{
            // printf("read");
            this->installCacheLines(request, cache_indexes, 0, DATA);
            break;
        }
        case MEMORY_OPERATION_WRITE:{
            // printf("write");
            this->installCacheLines(request, cache_indexes, 0, DATA);
            int cache_level = DATA_LEVELS - 1;
            for (int32_t k = cache_level - 1; k >= 0; k--) {
                this->data_cache[k+1][cache_indexes[k+1]].add_cache_write();
            }
            this->data_cache[0][cache_indexes[0]].write(request);
            break;
        }
        default:
            break;
    }
    
    delete[] cache_indexes;
}

bool cache_manager_t::searchData(memory_package_t *request) {
    if (op_count[request->processor_id][request->memory_operation] >= op_max[request->memory_operation]) return false;
    op_count[request->processor_id][request->memory_operation]++;
    
    if (request->memory_operation == MEMORY_OPERATION_READ) this->add_reads();
    else if (request->memory_operation == MEMORY_OPERATION_WRITE) this->add_writes();

    if (!isIn (request)) requests.push_back (request);
    return true;
}

void cache_manager_t::clock() {
    // printf("clock executing\n");
    if (requests.size() > 0) {
        //ORCS_PRINTF ("%lu ", requests.size())
        for (size_t i = 0; i < requests.size(); i++){
            if (requests[i]->readyAt <= orcs_engine.get_global_cycle()) {
                if (requests[i]->status == PACKAGE_STATE_WAIT && requests[i]->readyAt <= orcs_engine.get_global_cycle()) {
                    if (requests[i]->sent_to_ram) this->install (requests[i]);
                    this->finishRequest (requests[i]);
                }
                else if (!requests[i]->sent_to_cache && !requests[i]->sent_to_ram) this->requestCache (requests[i]);
                else if (requests[i]->sent_to_cache && !requests[i]->sent_to_ram) orcs_engine.memory_controller->requestDRAM (requests[i]);
            }
        }
    }
}

void cache_manager_t::requestCache (memory_package_t* request){
    request->sent_to_cache = true;
    
    uint32_t ttc = 0, latency_request = 0;
        int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
        this->generateIndexArray(request->processor_id, cache_indexes);
        
        switch (request->memory_operation){
            case MEMORY_OPERATION_INST:
                recursiveInstructionSearch (request, cache_indexes, latency_request, ttc, 0);
                break;
            case MEMORY_OPERATION_READ:
                recursiveDataSearch (request, cache_indexes, latency_request, ttc, 0, DATA);
                break;
            case MEMORY_OPERATION_WRITE:
                recursiveDataWrite (request, cache_indexes, latency_request, ttc, 0, DATA);
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
                orcs_engine.hive_controller->addRequest (request);
                break;
            case MEMORY_OPERATION_VIMA_FP_ALU:
            case MEMORY_OPERATION_VIMA_FP_MUL:
            case MEMORY_OPERATION_VIMA_FP_DIV:
            case MEMORY_OPERATION_VIMA_INT_ALU:
            case MEMORY_OPERATION_VIMA_INT_MUL:
            case MEMORY_OPERATION_VIMA_INT_DIV:
            case MEMORY_OPERATION_VIMA_INT_MLA:
            case MEMORY_OPERATION_VIMA_FP_MLA:
                orcs_engine.vima_controller->addRequest (request);
                break;
            default:
                break;
        }

        delete[] cache_indexes;
}

uint32_t cache_manager_t::searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc) {
    uint32_t cache_status = cache->read(instructionAddress, *ttc);
    cache->add_cache_access();
    *latency_request += *ttc;
    return cache_status;
}

// Searches an instruction in cache levels
cache_status_t cache_manager_t::recursiveInstructionSearch(memory_package_t* request, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level) {
    // printf("recursiveInstructionSearch %lu in processor %u\n", request->memory_address, request->processor_id);
    uint32_t cache_status = this->searchAddress(request->opcode_address, &this->instruction_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    if (cache_status == HIT) {
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
        if (cache_level != 0) {
            for (int32_t i = INSTRUCTION_LEVELS - 1; i >= 0; i--) {
                this->instruction_cache[cache_level][cache_indexes[cache_level]].returnLine(request, &this->instruction_cache[i][cache_indexes[i]]);
            }
        }
        request->updatePackageWait(latency_request);
        return HIT;
    }
    this->instruction_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == INSTRUCTION_LEVELS - 1) {
        return recursiveDataSearch(request, cache_indexes, latency_request, ttc, cache_level + 1, INSTRUCTION);
    }
    return recursiveInstructionSearch(request, cache_indexes, latency_request, ttc, cache_level + 1);
}

cache_status_t cache_manager_t::recursiveDataSearch(memory_package_t *request, int32_t *cache_indexes, uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type) {
    // printf("recursiveDataSearch %lu in processor %u\n", request->memory_address, request->processor_id);
    uint32_t cache_status = this->searchAddress(request->memory_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    //ORCS_PRINTF ("%s address: %lu cache level: %u ", get_enum_memory_operation_char (request->memory_operation), request->memory_address, cache_level)
    if (cache_status == HIT) {
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        for (int32_t i = cache_level - 1; i >= 0; i--) {
            this->data_cache[i+1][cache_indexes[i+1]].returnLine(request, &this->data_cache[i][cache_indexes[i]]);
            this->data_cache[i+1][cache_indexes[i+1]].add_cache_write();
        }
        //ORCS_PRINTF ("Cache HIT! %s uop: %lu cache level: %u ", get_enum_memory_operation_char (request->memory_operation), request->uop_number, cache_level)
        request->updatePackageWait(latency_request);
        return HIT;
    }
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        if (cache_type == DATA) orcs_engine.processor[request->processor_id].request_DRAM++;
        request->updatePackageUntreated (latency_request);
        return MISS;
    }
    return recursiveDataSearch(request, cache_indexes, latency_request, ttc, cache_level + 1, cache_type);
}

cache_status_t cache_manager_t::recursiveDataWrite(memory_package_t *request, int32_t *cache_indexes,
                                             uint32_t latency_request, uint32_t ttc, uint32_t cache_level, cacheId_t cache_type) {
    // printf("recursiveDataWrite %lu in processor %u\n", request->memory_address, request->processor_id);
    uint32_t cache_status = this->searchAddress(request->memory_address, &this->data_cache[cache_level][cache_indexes[cache_level]], &latency_request, &ttc);
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_read();
    //ORCS_PRINTF ("%s address: %lu cache level: %u ", get_enum_memory_operation_char (request->memory_operation), request->memory_address, cache_level)
    if (cache_status == HIT) {
        this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_hit();
        for (int32_t i = cache_level - 1; i >= 0; i--) {
            this->data_cache[i+1][cache_indexes[i+1]].returnLine(request, &this->data_cache[i][cache_indexes[i]]);
        }
        this->data_cache[0][cache_indexes[0]].write(request);
        //ORCS_PRINTF ("Cache HIT! %s address: %lu cache level: %u ", get_enum_memory_operation_char (request->memory_operation), request->memory_address, cache_level)
        request->updatePackageWait(latency_request);
        return HIT;
    }
    this->data_cache[cache_level][cache_indexes[cache_level]].add_cache_miss();
    ttc = 0;
    if (cache_level == DATA_LEVELS - 1) {
        request->updatePackageUntreated (latency_request);
        return MISS;
    }
    return recursiveDataWrite(request, cache_indexes, latency_request, ttc, cache_level + 1, cache_type);
}

bool cache_manager_t::available (uint32_t processor_id, memory_operation_t op){
    return op_count[processor_id][op] < op_max[op];
}

void cache_manager_t::statistics(uint32_t core_id) {
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
    bool close = false;
    FILE *output = stdout;
	if (orcs_engine.output_file_name != NULL) {
		output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
	if (output != NULL) {
        utils_t::largestSeparator(output);
        fprintf(output,"##############  Cache Memories ##################\n");
        if (this->get_sent_ram() > 0) {
            ORCS_PRINTF ("Total Reads: %lu\nTotal Writes: %lu\n", this->get_reads(), this->get_writes())
            ORCS_PRINTF ("Total RAM requests: %u\nTotal RAM request latency cycles: %u\n", this->get_sent_ram(), this->get_sent_ram_cycles())
            ORCS_PRINTF ("Average wait for RAM requests: %u\n", sent_ram_cycles/sent_ram)
        }
        if (this->get_sent_hive() > 0) {
            ORCS_PRINTF ("Total HIVE requests: %u\nTotal HIVE request latency cycles: %u\n", this->get_sent_hive(), this->get_sent_hive_cycles())
            ORCS_PRINTF ("Average wait for HIVE requests: %u\n", sent_hive_cycles/sent_hive)
        }
        if (this->get_sent_vima() > 0){
            ORCS_PRINTF ("Total VIMA requests: %u\nTotal VIMA request latency cycles: %u\n", this->get_sent_vima(), this->get_sent_vima_cycles())
            ORCS_PRINTF ("Average wait for VIMA requests: %u\n", sent_vima_cycles/sent_vima)
        }
    
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
