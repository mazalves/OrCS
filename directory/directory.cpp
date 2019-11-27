#include "../simulator.hpp"

#ifdef DIRECTORY_DEBUG
    #define DIRECTORY_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
    #define DIRECTORY_DEBUG_PRINTF(...)
#endif


directory_t::directory_t() {
    this->n_sets = 0;
    this->sets = NULL;
}

directory_t::~directory_t() {
    // for (uint32_t i = 0; i < this->n_sets; i++) {
    //     for (uint32_t j = 0; j < this->sets[i].n_ways; j++) {
    //         delete[] this->sets[i].ways[j];
    //     }
    // }
    // delete[] sets;
}

void directory_t::allocate(cache_t llc, uint32_t POINTER_LEVELS) {
    this->n_sets = llc.n_sets;
    this->sets = new directory_set_t[this->n_sets];
    for (uint32_t i = 0; i < this->n_sets; i++) {
        this->sets[i].ways = new directory_way_t*[llc.associativity];
		this->sets[i].n_ways = llc.associativity;
        for (uint32_t j = 0; j < this->sets[i].n_ways; j++) {
            this->sets[i].ways[j] = new directory_way_t[POINTER_LEVELS];
            for (uint32_t k = 0; k < POINTER_LEVELS; k++) {
                this->sets[i].ways[j][k].clean_way();
                this->sets[i].ways[j][k].level = k;
            }
        }
    }
    set_OFFSET(llc.offset);
}

void directory_t::setCachePointers(way_t *cache_way, uint32_t cache_level, memory_operation_t mem_op) {
    uint32_t idx;
    uint64_t tag;
    
    this->tagIdxSetCalculation(cache_way->address, &idx, &tag);
    int32_t way = this->getDirectoryLine(idx, tag);

    DIRECTORY_DEBUG_PRINTF("Installing address %lu (TAG %lu) in directory in %s %s\n", cache_way->address, cache_way->tag, get_enum_cache_id_char(this->sets[idx].ways[way][cache_level].id), get_cache_level_char(cache_level));

    this->sets[idx].ways[way][cache_level].cache_way = cache_way;
    this->sets[idx].ways[way][cache_level].shared = 1;
    this->sets[idx].ways[way][cache_level].cache_status = CACHED;
    if (mem_op == MEMORY_OPERATION_INST) {
        this->sets[idx].ways[way][cache_level].id = INSTRUCTION;
    } else {
        this->sets[idx].ways[way][cache_level].id = DATA;
    }
    this->sets[idx].ways[way][cache_level].tag = tag;
    cache_way->directory_way = &this->sets[idx].ways[way][cache_level];
}

void directory_t::installCachePointers(way_t ***cache_ways, uint32_t n_proc, uint32_t cache_levels, uint32_t idx, int32_t way, memory_operation_t mem_op) {
    for (uint32_t i = 0; i < n_proc; i++) {
        for (uint32_t j = 0; j < cache_levels; j++) {
            this->sets[idx].ways[way][j].cache_way = cache_ways[i][j];
            this->sets[idx].ways[way][j].shared = 1;
            this->sets[idx].ways[way][j].cache_status = CACHED;
            if (mem_op == MEMORY_OPERATION_INST) {
                this->sets[idx].ways[way][j].id = INSTRUCTION;
            } else {
                this->sets[idx].ways[way][j].id = DATA;
            }
            this->sets[idx].ways[way][j].tag = cache_ways[i][j]->tag;
            cache_ways[i][j]->directory_way = &this->sets[idx].ways[way][j];
            DIRECTORY_DEBUG_PRINTF("Installing address %lu (TAG %lu) in directory in %s %s\n", cache_ways[i][j]->address, cache_ways[i][j]->tag, get_enum_cache_id_char(this->sets[idx].ways[way][j].id), get_cache_level_char(j));
        }
    }
}

int32_t directory_t::getDirectoryLine(uint32_t idx, uint64_t tag) {
    int32_t way = POSITION_FAIL;
    for (size_t i = 0; i < this->sets[idx].n_ways; i++) {
        if (this->sets[idx].ways[i][2].tag == tag) {
            way = i;
            break;
        }
    }
    return way;
}

void directory_t::nullingCaches(uint64_t address, uint32_t cache_levels) {
    DIRECTORY_DEBUG_PRINTF("Delete all the pointers between cache and directory\n");
    uint32_t idx;
    uint64_t tag;

    this->tagIdxSetCalculation(address, &idx, &tag);
    int32_t way = this->getDirectoryLine(idx, tag);    //TODO renomear para VIA do diretorio e não linha
    DIRECTORY_DEBUG_PRINTF("Address %lu found in set %u and way %u in directory. ", address, idx, way);

    for (uint32_t i = 0; i < cache_levels; i++) {
        if (this->sets[idx].ways[way][i].cache_way != NULL) {
            DIRECTORY_DEBUG_PRINTF("In %s cache\n", get_enum_cache_id_char(this->sets[idx].ways[way][i].id));
            // this->sets[idx].ways[way][i].cache_way->directory_way = NULL;
            // this->sets[idx].ways[way][i].cache_way = NULL;
            this->sets[idx].ways[way][i].cache_way->clean_way();
            this->sets[idx].ways[way][i].clean_way();
        }
    }
}

// Return address index in cache
void directory_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag) {
    uint32_t get_bits = this->n_sets - 1;
    *tag = address >> OFFSET;
    *idx = *tag & get_bits;
}

uint32_t directory_t::validCacheLine(uint64_t address, uint32_t cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
    int32_t way = this->getDirectoryLine(idx, tag);
    if (this->sets[idx].ways[way][cache_level].cache_way != NULL) {
        return 1;
    }
    return 0;
}

uint32_t directory_t::dirtyCacheLine(uint64_t address, uint32_t cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
    int32_t way = this->getDirectoryLine(idx, tag);
    if (this->sets[idx].ways[way][cache_level].cache_way->dirty == 1) {
        return 1;
    }
    return 0;
}

void directory_t::copyCacheInfo(uint64_t address, uint32_t to_cache_level, uint32_t from_cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
    int32_t way = this->getDirectoryLine(idx, tag);

    DIRECTORY_DEBUG_PRINTF("Address %lu in %s %s is dirty. Copying its info to %s %s\n", address, get_enum_cache_id_char(this->sets[idx].ways[way][from_cache_level].id), get_cache_level_char(from_cache_level), get_enum_cache_id_char(this->sets[idx].ways[way][to_cache_level].id), get_cache_level_char(to_cache_level));
    printf("%s receives %s\n", get_cache_level_char(to_cache_level), get_cache_level_char(from_cache_level));
    if (this->sets[idx].ways[way][to_cache_level].cache_way == NULL) {
        printf("%s is NULL\n", get_cache_level_char(to_cache_level));
    }
    if (this->sets[idx].ways[way][from_cache_level].cache_way == NULL) {
        printf("%s is NULL\n", get_cache_level_char(from_cache_level));
    }
    this->sets[idx].ways[way][to_cache_level].cache_way->dirty = this->sets[idx].ways[way][from_cache_level].cache_way->dirty;
    this->sets[idx].ways[way][to_cache_level].cache_way->lru = orcs_engine.get_global_cycle();
    this->sets[idx].ways[way][to_cache_level].cache_way->ready_at = this->sets[idx].ways[way][from_cache_level].cache_way->ready_at;
}

void directory_t::nullCachePointer(uint64_t address, uint32_t cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag);
    int32_t way = this->getDirectoryLine(idx, tag);

    DIRECTORY_DEBUG_PRINTF("Deleting pointers between %s %s and directory\n to address %lu", get_enum_cache_id_char(this->sets[idx].ways[way][cache_level].id), get_cache_level_char(cache_level), address);

    if (this->sets[idx].ways[way][cache_level].cache_way != NULL) {
        // this->sets[idx].ways[way][cache_level].cache_way->clean_way();
        // this->sets[idx].ways[way][cache_level].clean_way();
        this->sets[idx].ways[way][cache_level].cache_way->directory_way = NULL;
        this->sets[idx].ways[way][cache_level].cache_way = NULL;
    }
}