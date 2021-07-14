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
    
    this->sent_ram = 0;
    this->sent_ram_cycles = 0;
    this->sent_hive = 0;
    this->sent_hive_cycles = 0;
    this->sent_vima = 0;
    this->sent_vima_cycles = 0;

    this->max_vima = 0;

    this->LINE_SIZE = 0;
    this->PREFETCHER_ACTIVE = 0;
    this->DATA_LEVELS = 0;
    this->INSTRUCTION_LEVELS = 0;
    this->POINTER_LEVELS = 0;
    this->LLC_CACHES = 0;
    this->CACHE_MANAGER_DEBUG = 0;
    this->WAIT_CYCLE = 0;
    
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

    delete[] this->total_operations;
    delete[] this->total_latency;
    delete[] this->min_wait_operations;
    delete[] this->max_wait_operations;

    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) delete[] instruction_cache[i];
    for (uint32_t i = 0; i < DATA_LEVELS; i++) delete[] data_cache[i];

    delete[] ICACHE_AMOUNT;
    delete[] DCACHE_AMOUNT;

    delete[] data_cache;
    delete[] instruction_cache;

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
    clevels.shrink_to_fit();

//TODO CHECK DCACHE_AMOUNT masks.
    if (cache_type == 0) {
        set_INSTRUCTION_LEVELS(clevels.size());
        ICACHE_AMOUNT = new uint32_t[INSTRUCTION_LEVELS]();
        for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
            ICACHE_AMOUNT[i] = std::count(camount.begin(), camount.end(), i);
        }
    } else {
        set_DATA_LEVELS(clevels.size());
        DCACHE_AMOUNT = new uint32_t[DATA_LEVELS]();
        for (uint32_t i = 0; i < DATA_LEVELS; i++) {
            DCACHE_AMOUNT[i] = std::count(camount.begin(), camount.end(), i);
        }
        //POINTER_LEVELS = 3;
        POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
    }

	//valgrind accused memory leak even when using delete, swapping actually solved the issue to erase these vectors.
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
                    cache[j].mshr_size = cfg_cache["MSHR_SIZE"];
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

    uint32_t CACHE_LEVELS, *CACHE_AMOUNT = NULL;
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
            cache[i][j].is_inst_cache = (cache_type == INSTRUCTION);
        }
    }
    delete[] CACHE_AMOUNT;
    return cache;
}

void cache_manager_t::allocate(uint32_t NUMBER_OF_PROCESSORS) {
    // Access configure file
    set_NUMBER_OF_PROCESSORS(NUMBER_OF_PROCESSORS);
    
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
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
    //set_POINTER_LEVELS(3);

    //Read/Write counters
    this->set_reads(0);
    this->set_read_hit(0);
    this->set_read_miss(0);
    this->set_writes(0);
    this->set_write_hit(0);
    this->set_write_miss(0);
    this->set_offset(utils_t::get_power_of_two(LINE_SIZE));
    this->set_min_sent_ram(UINT32_MAX);
    this->set_max_sent_ram(0);
    this->set_sent_ram(0);
    this->set_sent_ram_cycles(0);
    this->set_sent_hive(0);
    this->set_sent_hive_cycles(0);
    this->set_sent_vima(0);
    this->set_sent_vima_cycles(0);

    this->total_latency = new uint64_t [MEMORY_OPERATION_LAST]();
    this->total_operations = new uint64_t [MEMORY_OPERATION_LAST]();
    this->min_wait_operations = new uint64_t [MEMORY_OPERATION_LAST]();
    for (i = 0; i < MEMORY_OPERATION_LAST; i++) this->min_wait_operations[i] = UINT64_MAX;
    this->max_wait_operations = new uint64_t [MEMORY_OPERATION_LAST]();

    //Allocate Prefetcher
    if (PREFETCHER_ACTIVE) {
        this->prefetcher = new prefetcher_t();
        this->prefetcher->allocate(NUMBER_OF_PROCESSORS);
    }
}

// Dependending on processor_id, returns its correspondent data caches
void cache_manager_t::generateIndexArray(uint32_t processor_id, int32_t *cache_indexes) {
    for (uint32_t i = 0; i < POINTER_LEVELS; i++) {
        cache_indexes[i] = (int32_t) processor_id & (DCACHE_AMOUNT[i] - 1);
    }
}

// Install an address in every cache using pointers
void cache_manager_t::installCacheLines(memory_package_t* request, int32_t *cache_indexes, uint32_t latency_request, cacheId_t cache_type) {
    //printf("installCacheLines %lu in processor %u\n", request->memory_address, request->processor_id);
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
            #if MEMORY_DEBUG
                ORCS_PRINTF ("will be forwarded by %lu %s.\n", requests[i]->memory_address, get_enum_memory_operation_char (requests[i]->memory_operation))
            #endif
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
        ORCS_PRINTF ("MSHR Table entry %lu: mop %s | Addr :%lu | status %s. (flag LRU)\n", i, get_enum_memory_operation_char (requests[i]->memory_operation), requests[i]->memory_address, get_enum_package_state_char (requests[i]->status))
    }
}

void cache_manager_t::finishRequest (memory_package_t* request, int32_t* cache_indexes){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[CACM] %lu {%lu} %lu %s finishes! Took %lu cycles.\n", orcs_engine.get_global_cycle(), request->opcode_number, request->memory_address, get_enum_memory_operation_char (request->memory_operation), orcs_engine.get_global_cycle()-request->born_cycle)
    #endif
    wait_time = (orcs_engine.get_global_cycle() - request->born_cycle);
    if (request->sent_to_ram && !request->is_hive && !request->is_vima){
        sent_ram++;
        sent_ram_cycles += wait_time;
        if (wait_time > max_sent_ram) max_sent_ram = wait_time;
        if (wait_time < min_sent_ram) min_sent_ram = wait_time;
    } else if (request->is_hive){
        sent_hive++;
        sent_hive_cycles += wait_time;
    } else if (request->is_vima){
        sent_vima++;
        sent_vima_cycles += wait_time;
    }

    this->total_operations[request->memory_operation]++;
    this->total_latency[request->memory_operation] += wait_time;
    if (wait_time < this->min_wait_operations[request->memory_operation]) this->min_wait_operations[request->memory_operation] = wait_time;
    if (wait_time > this->max_wait_operations[request->memory_operation]) this->max_wait_operations[request->memory_operation] = wait_time;

    #if PROCESSOR_DEBUG
        ORCS_PRINTF ("%lu request %s ", orcs_engine.get_global_cycle(), get_enum_memory_operation_char(request->memory_operation))
        if (request->is_vima){
            if (request->vima_read1 != 0) ORCS_PRINTF ("READ1: %lu | ", request->vima_read1)
            if (request->vima_read2 != 0) ORCS_PRINTF ("READ2: %lu | ", request->vima_read2)
            if (request->vima_write != 0) ORCS_PRINTF ("WRITE: %lu | ", request->vima_write)
        } else if (request->is_hive){
            if (request->hive_read1 != 0) ORCS_PRINTF ("READ1: %lu | ", request->hive_read1)
            if (request->hive_read2 != 0) ORCS_PRINTF ("READ2: %lu | ", request->hive_read2)
            if (request->hive_write != 0) ORCS_PRINTF ("WRITE: %lu | ", request->hive_write)
        } else ORCS_PRINTF ("address: %lu ", request->memory_address)
        ORCS_PRINTF ("%lu born at %lu, finished at %lu. Took %lu cycles, came from processor %u.\n", request->uop_number, request->born_cycle, orcs_engine.get_global_cycle(), orcs_engine.get_global_cycle()-request->born_cycle, request->processor_id)
    #endif

    int32_t wait_time_levels = 0;

    if (request->memory_operation == MEMORY_OPERATION_INST) {
        //printf("Finishing request\n");
        for (i = 0; i < INSTRUCTION_LEVELS; i++) {
            if (request->sent_to_cache_level[this->instruction_cache[i][cache_indexes[i]].level]) {
                #if PROCESSOR_DEBUG
                    ORCS_PRINTF ("%lu memory = %lu | level = %lu, type = %s, count = %u -> count = %u, %s FINISH, %lu\n", orcs_engine.get_global_cycle(), requests.size(), i, get_enum_cache_type_char ((cacheId_t) instruction_cache[i][cache_indexes[i]].id), instruction_cache[i][cache_indexes[i]].count, instruction_cache[i][cache_indexes[i]].count-1, get_enum_memory_operation_char (request->memory_operation), request->memory_address)
                #endif
                wait_time_levels = orcs_engine.get_global_cycle() - request->sent_to_cache_level_at[i];
                this->instruction_cache[i][cache_indexes[i]].count--;
                this->instruction_cache[i][cache_indexes[i]].total_per_type[request->memory_operation] += wait_time_levels;
                if (wait_time_levels > this->instruction_cache[i][cache_indexes[i]].max_per_type[request->memory_operation]) this->instruction_cache[i][cache_indexes[i]].max_per_type[request->memory_operation] = wait_time_levels;
                if (wait_time_levels < this->instruction_cache[i][cache_indexes[i]].min_per_type[request->memory_operation]) this->instruction_cache[i][cache_indexes[i]].min_per_type[request->memory_operation] = wait_time_levels;
                ERROR_ASSERT_PRINTF (this->instruction_cache[i][cache_indexes[i]].count > -1, "VALUE BECOMES NEGATIVE")
            }
        }

        for (i = INSTRUCTION_LEVELS; i < request->next_level; i++) {
            if (request->sent_to_cache_level[this->data_cache[i][cache_indexes[i]].level]) {
                #if PROCESSOR_DEBUG
                    ORCS_PRINTF ("%lu memory = %lu | level = %lu, type = %s, count = %u -> count = %u, %s FINISH, %lu\n", orcs_engine.get_global_cycle(), requests.size(), i, get_enum_cache_type_char ((cacheId_t) data_cache[i][cache_indexes[i]].id), data_cache[i][cache_indexes[i]].count, data_cache[i][cache_indexes[i]].count-1, get_enum_memory_operation_char (request->memory_operation), request->memory_address)
                #endif
                wait_time_levels = orcs_engine.get_global_cycle() - request->sent_to_cache_level_at[i];
                this->data_cache[i][cache_indexes[i]].count--;
                this->data_cache[i][cache_indexes[i]].total_per_type[request->memory_operation] += wait_time_levels;
                if (wait_time_levels > this->data_cache[i][cache_indexes[i]].max_per_type[request->memory_operation]) this->data_cache[i][cache_indexes[i]].max_per_type[request->memory_operation] = wait_time_levels;
                if (wait_time_levels < this->data_cache[i][cache_indexes[i]].min_per_type[request->memory_operation]) this->data_cache[i][cache_indexes[i]].min_per_type[request->memory_operation] = wait_time_levels;
                ERROR_ASSERT_PRINTF (this->data_cache[i][cache_indexes[i]].count > -1, "VALUE BECOMES NEGATIVE")
            }
        }
    } else {
        for (i = 0; i < request->next_level; i++) {
            if (request->sent_to_cache_level[this->data_cache[i][cache_indexes[i]].level]){
                #if PROCESSOR_DEBUG
                    ORCS_PRINTF ("%lu memory = %lu | level = %lu, type = %s, count = %u -> count = %u, %s FINISH, %lu\n", orcs_engine.get_global_cycle(), requests.size(), i, get_enum_cache_type_char ((cacheId_t) data_cache[i][cache_indexes[i]].id), data_cache[i][cache_indexes[i]].count, data_cache[i][cache_indexes[i]].count-1, get_enum_memory_operation_char (request->memory_operation), request->memory_address)
                #endif
                wait_time_levels = orcs_engine.get_global_cycle() - request->sent_to_cache_level_at[i];
                this->data_cache[i][cache_indexes[i]].count--;
                this->data_cache[i][cache_indexes[i]].total_per_type[request->memory_operation] += wait_time_levels;
                if (wait_time_levels > this->data_cache[i][cache_indexes[i]].max_per_type[request->memory_operation]) this->data_cache[i][cache_indexes[i]].max_per_type[request->memory_operation] = wait_time_levels;
                if (wait_time_levels < this->data_cache[i][cache_indexes[i]].min_per_type[request->memory_operation]) this->data_cache[i][cache_indexes[i]].min_per_type[request->memory_operation] = wait_time_levels;
                ERROR_ASSERT_PRINTF (this->data_cache[i][cache_indexes[i]].count > -1, "VALUE BECOMES NEGATIVE")
            }
        }
    }

    //if (request->is_vima) ORCS_PRINTF ("%lu Cache Manager finishRequest(): VIMA INSTRUCTION READY!\n", orcs_engine.get_global_cycle())

    request->updatePackageReady();
    request->updateClients();
    requests.erase (std::remove (requests.begin(), requests.end(), request), requests.end());
    requests.shrink_to_fit();
    //if (request->memory_operation != MEMORY_OPERATION_INST) 
    #if PROCESSOR_DEBUG
        ORCS_PRINTF ("%lu Cache Manager finishRequest(): finished memory request %lu from uop %lu, %s.\n", orcs_engine.get_global_cycle(), request->memory_address, request->uop_number, get_enum_memory_operation_char (request->memory_operation))
    #endif
    delete request;
}

void cache_manager_t::install (memory_package_t* request){
    #if PROCESSOR_DEBUG 
        printf("%lu Cache Manager installCache(): %lu in processor %u\n", orcs_engine.get_global_cycle(), request->memory_address, request->processor_id);
    #endif
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
    this->generateIndexArray(request->processor_id, cache_indexes);
    switch (request->memory_operation){
        case MEMORY_OPERATION_INST:{
            // printf("instruction");
            //printf("Installing from DRAM\n");
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


uint64_t already_searching_inst = 0;
uint64_t already_searching_read = 0;
uint64_t already_searching_write = 0;
bool cache_manager_t::searchData(memory_package_t *request) {
    memory_operation_t mem_t;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[CACM] %lu {%lu} %lu %s enters | ", orcs_engine.get_global_cycle(), request->opcode_number, request->memory_address, get_enum_memory_operation_char (request->memory_operation))
    #endif
    mem_t = request->memory_operation;
    if (mem_t == MEMORY_OPERATION_READ) this->add_reads();
    else if (mem_t == MEMORY_OPERATION_WRITE) this->add_writes();


    if (!isIn (request)) {
        requests.push_back (request);
        requests.shrink_to_fit();

        #if MEMORY_DEBUG
            ORCS_PRINTF ("is new from uop: %lu\n", request->uop_number);
        #endif
    } else {
        if (mem_t == MEMORY_OPERATION_INST)
        {
            //printf("Hit MSHR %lu (flag LRU)\n", request->memory_address);
            already_searching_inst++;
        } else if (mem_t == MEMORY_OPERATION_READ) {
            already_searching_read++;
        } else if (mem_t == MEMORY_OPERATION_WRITE) {
            already_searching_write++;
        }
    }
    return true;
}

void cache_manager_t::process (memory_package_t* request, int32_t* cache_indexes){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[CACM] %lu {%lu} %lu %s will now be processed |", orcs_engine.get_global_cycle(), request->opcode_number, request->memory_address, get_enum_memory_operation_char (request->memory_operation))
    #endif
    #if PROCESSOR_DEBUG
        ORCS_PRINTF ("%lu Cache Manager process(): request %lu, %s %s, readyAt %lu \n", orcs_engine.get_global_cycle(), request->memory_address, get_enum_memory_operation_char (request->memory_operation), get_enum_package_state_char (request->status), request->readyAt)
    #endif
    cache_t* cache;
    switch (request->memory_operation){
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_WRITE:
        case MEMORY_OPERATION_INST:
            //printf("%lu: Trying to process ins: %lu Ready at: %lu(flag LRU)\n", orcs_engine.get_global_cycle(), request->opcode_address, request->readyAt);
            if (request->next_level < DATA_LEVELS){
                if (request->status == PACKAGE_STATE_UNTREATED && request->readyAt <= orcs_engine.get_global_cycle()){
                    if (request->memory_operation == MEMORY_OPERATION_INST && request->next_level == L1) {
                        //printf("Processing %lu (flag LRU)\n", request->memory_address);
                        cache = &this->instruction_cache[request->next_level][cache_indexes[request->next_level]];
                    } else cache = &this->data_cache[request->next_level][cache_indexes[request->next_level]];
                    if (cache->count < cache->mshr_size) {
                        #if MEMORY_DEBUG
                            ORCS_PRINTF (" sent to %s |", get_enum_cache_level_char ((cacheLevel_t) request->next_level))
                        #endif
                        /*if (request->memory_operation == MEMORY_OPERATION_WRITE && request->memory_size == this->get_LINE_SIZE()){
                            request->updatePackageWait(this->data_cache[request->next_level][cache_indexes[request->processor_id]].latency);
                            request->sent_to_ram = true;
                        } 
                        else */
                        this->cache_search (request, cache, cache_indexes);
                    }/* else {
                        printf("Greater than mshr\n");
                    }*/
                }
            } else if (!request->sent_to_ram) {
                #if MEMORY_DEBUG 
                    ORCS_PRINTF (" sent to RAM.\n")
                #endif
                orcs_engine.memory_controller->add_requests_llc();
                orcs_engine.memory_controller->requestDRAM (request);
            }
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
            #if MEMORY_DEBUG 
                ORCS_PRINTF (" sent to HIVE Controller.\n")
            #endif
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
        case MEMORY_OPERATION_VIMA_GATHER:
        case MEMORY_OPERATION_VIMA_SCATTER:
            #if MEMORY_DEBUG 
                ORCS_PRINTF (" sent to VIMA Controller.\n")
            #endif
            orcs_engine.vima_controller->addRequest (request);
            break;
        default:
            ERROR_ASSERT_PRINTF (true, "WRONG MEMORY OPERATION TYPE")
            break;
    }
}

void cache_manager_t::clock() {
    // printf("clock executing\n");
    if (requests.size() > 0) {
        int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
        for (size_t i = 0; i < requests.size(); i++){
            /*if (requests[i]->memory_operation == MEMORY_OPERATION_INST)
            {
                printf("Avaliando request: %lu [Type: %s] (Ready at: %lu)(flag LRU)\n", requests[i]->memory_address, get_enum_memory_operation_char(requests[i]->memory_operation), requests[i]->readyAt);
            }*/
            if (requests[i]->readyAt <= orcs_engine.get_global_cycle()){
                generateIndexArray(requests[i]->processor_id, cache_indexes);
                if (requests[i]->status == PACKAGE_STATE_WAIT){
                    if (requests[i]->sent_to_ram) this->install (requests[i]);
                    //if (requests[i]->memory_operation == MEMORY_OPERATION_INST) printf("%lu: Finishing request %lu (flag LRU)\n", orcs_engine.get_global_cycle(), requests[i]->opcode_address);
                    this->finishRequest (requests[i], cache_indexes);
                    --i;
                }
                else if (!requests[i]->sent_to_ram) this->process (requests[i], cache_indexes);
            }

        }
        delete[] cache_indexes;
    }
    /*if(orcs_engine.get_global_cycle() % 1000000 == 0) {
        printf("+++++++ L1 instructions cache info, [%luM] +++++++\n", orcs_engine.get_global_cycle()/1000000);
        this->instruction_cache[0][0].print_cache();
    }*/
}

uint32_t cache_manager_t::searchAddress(uint64_t instructionAddress, cache_t *cache, uint32_t *latency_request, uint32_t *ttc) {
    uint32_t cache_status = cache->read(instructionAddress, *ttc);
    *latency_request += *ttc;
    return cache_status;
}

cache_status_t cache_manager_t::cache_search (memory_package_t* request, cache_t* cache, int32_t* cache_indexes){
    // Se essa condição for falsa a transferência de dados entre níveis dá errado.
    assert(INSTRUCTION_LEVELS < DATA_LEVELS);
    #if PROCESSOR_DEBUG
        ORCS_PRINTF ("%lu Cache Manager cache_search(): memory = %lu | level = %u, type = %s, count = %u -> count = %u, %s, %lu\n", orcs_engine.get_global_cycle(), requests.size(), request->next_level, get_enum_cache_type_char ((cacheId_t) cache->id), cache->get_count(), cache->get_count()+1, get_enum_memory_operation_char (request->memory_operation), request->memory_address)
    #endif
    uint32_t cache_status = 0, ttc = 0;
    cache_status = this->searchAddress(request->memory_address, cache, &request->cache_latency, &ttc);
    cache->cache_count_per_type[request->memory_operation]++;    
    request->sent_to_cache_level[request->next_level] = 1;
    request->sent_to_cache_level_at[request->next_level] = orcs_engine.get_global_cycle();
    
    if (cache_status == HIT) {
        #if MEMORY_DEBUG 
            ORCS_PRINTF (" HIT!\n");
        #endif
        switch (request->memory_operation){
            case MEMORY_OPERATION_READ:
            case MEMORY_OPERATION_WRITE:
                if (request->next_level != 0) {
                    for (int32_t i = request->next_level - 1; i >= 0; i--) {
                        this->data_cache[i+1][cache_indexes[i+1]].returnLine(request, &this->data_cache[i][cache_indexes[i]]);
                    }
                }
                if (request->memory_operation == MEMORY_OPERATION_WRITE) this->data_cache[0][cache_indexes[0]].write(request);
            break;
            case MEMORY_OPERATION_INST:
                //printf("Hit %lu on cache level %u\n", request->memory_address, cache->level);
    
                if (request->next_level != 0 && cache->level < INSTRUCTION_LEVELS/*cache->level == L1*/) {
                    for (int32_t i = INSTRUCTION_LEVELS - 2; i >= 0; i--) {
                        //printf("Installing on level %d\n", i);
                        this->instruction_cache[i+1][cache_indexes[i+1]].returnLine(request, &this->instruction_cache[i][cache_indexes[i]]);
                    }
                } else if (request->next_level != 0) {
                    //printf("Encontrou em uma cache de dados\n");
                    // Between data caches
                    for (uint32_t i = request->next_level - 1; i >= INSTRUCTION_LEVELS; i--) {
                        this->data_cache[i+1][cache_indexes[i+1]].returnLine(request, &this->data_cache[i][cache_indexes[i]]);
                    }
                    // From D to I caches
                    this->data_cache[INSTRUCTION_LEVELS][cache_indexes[INSTRUCTION_LEVELS]].returnLine(request, &this->instruction_cache[INSTRUCTION_LEVELS - 1][cache_indexes[INSTRUCTION_LEVELS - 1]]);

                    // Between I caches
                    for (int32_t i = INSTRUCTION_LEVELS - 2; i >= 0; i--) {
                        this->instruction_cache[i+1][cache_indexes[i+1]].returnLine(request, &this->instruction_cache[i][cache_indexes[i]]);
                    }
                }

            break;
            default:
                ERROR_ASSERT_PRINTF (true, "WRONG MEMORY OPERATION TYPE")
            break;
        }
        request->updatePackageWait(ttc);
        request->next_level++;
        cache->cache_hit_per_type[request->memory_operation]++;


        return HIT;
    }

    #if MEMORY_DEBUG
        ORCS_PRINTF (" MISS!\n");
    #endif
    request->updatePackageUntreated(ttc);
    request->next_level++;
    cache->cache_miss_per_type[request->memory_operation]++;
    return MISS;
}

bool cache_manager_t::available (uint32_t processor_id, memory_operation_t op){
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
    bool result = false;
    this->generateIndexArray(processor_id, cache_indexes);
    switch (op){
        case MEMORY_OPERATION_READ:
        case MEMORY_OPERATION_WRITE:
            result = this->data_cache[L1][cache_indexes[L1]].get_count() < this->data_cache[L1][cache_indexes[L1]].mshr_size;
        break;
        case MEMORY_OPERATION_INST:
            result = this->instruction_cache[L1][cache_indexes[L1]].get_count() < this->instruction_cache[L1][cache_indexes[L1]].mshr_size;
        break;
        default:
            ERROR_ASSERT_PRINTF (true, "WRONG MEMORY OPERATION TYPE")
        break;
    }
    delete[] cache_indexes;
    return result;
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
        fprintf(output,"#========================================================================#\n");
        fprintf(output,"#Cache Manager\n");
        fprintf(output,"#========================================================================#\n");
        if (this->get_sent_ram() > 0) {
            ORCS_PRINTF ("Total Reads:                       %lu\nTotal Writes:                      %lu\n", this->get_reads(), this->get_writes())
            ORCS_PRINTF ("Total RAM requests:                %lu\nTotal RAM request latency cycles:  %lu\n", this->get_sent_ram(), this->get_sent_ram_cycles())
            ORCS_PRINTF ("Avg. wait for RAM requests:        %lu\n", sent_ram_cycles/sent_ram)
            ORCS_PRINTF ("Min. wait for RAM requests:        %lu\n", min_sent_ram)
            ORCS_PRINTF ("Max. wait for RAM requests:        %lu\n", max_sent_ram)
        }
        if (this->get_sent_hive() > 0) {
            ORCS_PRINTF ("Total HIVE requests:               %lu\nTotal HIVE request latency cycles: %lu\n", this->get_sent_hive(), this->get_sent_hive_cycles())
            ORCS_PRINTF ("Average wait for HIVE requests:    %lu\n", sent_hive_cycles/sent_hive)
        }
        if (this->get_sent_vima() > 0){
            ORCS_PRINTF ("Total VIMA requests:               %lu\nTotal VIMA request latency cycles: %lu\n", this->get_sent_vima(), this->get_sent_vima_cycles())
            ORCS_PRINTF ("Average wait for VIMA requests:    %lu\n", sent_vima_cycles/sent_vima)
        }
    
        utils_t::largestSeparator(output);

        for (i = 0; i < MEMORY_OPERATION_LAST; i++){
            if (this->total_operations[i] > 0) {
                fprintf(output,"%s_Tot._Latency_Hierarchy:  %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->total_latency[i]);
                fprintf(output,"%s_Avg._Latency_Hierarchy:  %lu (%lu/%lu)\n", get_enum_memory_operation_char ((memory_operation_t) i), this->total_latency[i]/this->total_operations[i], this->total_latency[i], this->total_operations[i]);
            }
            if (this->min_wait_operations[i] < UINT64_MAX) fprintf(output,"%s_Min._Latency_Hierarchy:  %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->min_wait_operations[i]);
            if (this->max_wait_operations[i] > 0) fprintf(output,"%s_Max._Latency_Hierarchy:  %lu\n", get_enum_memory_operation_char ((memory_operation_t) i), this->max_wait_operations[i]);
        }
        fprintf(output, "Already there (INST): %lu\n", already_searching_inst);
        fprintf(output, "Already there (READ): %lu\n", already_searching_read);
        fprintf(output, "Already there (WRITE): %lu\n", already_searching_write);

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

void cache_manager_t::reset_statistics (uint32_t core_id) {
    int32_t *cache_indexes = new int32_t[POINTER_LEVELS]();
    this->set_reads(0);
    this->set_writes(0);
    this->set_sent_ram(0);
    this->set_sent_ram_cycles(0);
    this->set_sent_hive(0);
    this->set_sent_hive(0);
    this->set_sent_hive_cycles(0);
    this->set_sent_vima(0);
    this->set_sent_vima(0);
    this->set_sent_vima_cycles(0);
    for (i = 0; i < MEMORY_OPERATION_LAST; i++){
        this->total_operations[i] = 0;
        this->total_latency[i] = 0;
        this->min_wait_operations[i] = 0;
        this->max_wait_operations[i] = 0;
    }
	this->generateIndexArray(core_id, cache_indexes);
    for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) this->instruction_cache[i][cache_indexes[i]].reset_statistics();
    for (uint32_t i = 0; i < DATA_LEVELS; i++) this->data_cache[i][cache_indexes[i]].reset_statistics();
    if (PREFETCHER_ACTIVE) this->prefetcher->reset_statistics();

    delete[] cache_indexes;
}
