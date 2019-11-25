#include "../simulator.hpp"

directory_t::directory_t() {
    this->n_sets = 0;
    this->sets = NULL;
}

directory_t::~directory_t() {
    // for (uint32_t i = 0; i < this->n_sets; i++) {
    //     for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
    //         delete[] this->sets[i].lines[j];
    //     }
    // }
    // delete[] sets;
}

void directory_t::allocate(cache_t llc, uint32_t POINTER_LEVELS) {
    this->n_sets = llc.n_sets;
    this->sets = new directory_set_t[this->n_sets];
    for (uint32_t i = 0; i < this->n_sets; i++) {
        this->sets[i].lines = new directory_line_t*[llc.associativity];
		this->sets[i].n_lines = llc.associativity;
        for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
            this->sets[i].lines[j] = new directory_line_t[POINTER_LEVELS];
            for (uint32_t k = 0; k < POINTER_LEVELS; k++) {
                this->sets[i].lines[j][k].clean_line();
                this->sets[i].lines[j][k].level = k;
                this->sets[i].lines[j][k].offset = 6;
            }
        }
    }
}

void directory_t::setCachePointers(line_t *cache_line, uint32_t cache_level, memory_operation_t mem_op) {
    uint32_t idx;
    uint64_t tag;
    
    this->tagIdxSetCalculation(cache_line->address, &idx, &tag, 6);
    int32_t line = this->getDirectoryLine(idx, tag);

    this->sets[idx].lines[line][cache_level].cache_line = cache_line;
    this->sets[idx].lines[line][cache_level].shared = 1;
    this->sets[idx].lines[line][cache_level].cache_status = CACHED;
    if (mem_op == MEMORY_OPERATION_INST) {
        this->sets[idx].lines[line][cache_level].id = INSTRUCTION;
    } else {
        this->sets[idx].lines[line][cache_level].id = DATA;
    }
    this->sets[idx].lines[line][cache_level].tag = tag;
    cache_line->directory_line = &this->sets[idx].lines[line][cache_level];
}

void directory_t::installCachePointers(line_t ***cache_lines, uint32_t n_proc, uint32_t cache_levels, uint32_t idx, int32_t line, memory_operation_t mem_op, uint64_t *cache_tags) {
    for (uint32_t i = 0; i < n_proc; i++) {
        for (uint32_t j = 0; j < cache_levels; j++) {
            this->sets[idx].lines[line][j].cache_line = cache_lines[i][j];
            this->sets[idx].lines[line][j].shared = 1;
            this->sets[idx].lines[line][j].cache_status = CACHED;
            if (mem_op == MEMORY_OPERATION_INST) {
                this->sets[idx].lines[line][j].id = INSTRUCTION;
            } else {
                this->sets[idx].lines[line][j].id = DATA;
            }
            this->sets[idx].lines[line][j].tag = cache_tags[j];
            cache_lines[i][j]->directory_line = &this->sets[idx].lines[line][j];
        }
    }
}

int32_t directory_t::getDirectoryLine(uint32_t idx, uint64_t tag) {
    int32_t line = POSITION_FAIL;
    for (size_t i = 0; i < this->sets[idx].n_lines; i++) {
        if (this->sets[idx].lines[i][2].tag == tag) {
            line = i;
            break;
        }
    }
    return line;
}

void directory_t::removeCachePointers(uint64_t address, uint32_t cache_levels) {

    uint32_t idx;
    uint64_t tag;
    
    CACHE_DEBUG_PRINTF("Searching for address in directory... ");
    this->tagIdxSetCalculation(address, &idx, &tag, 6); //TODO remober o 4096 feio!!!
    int32_t line = directory.getDirectoryLine(idx, tag);                    //TODO renomear para VIA do diretorio e não linha
    CACHE_DEBUG_PRINTF("... Found in set %u and way %u.\n", idx, line);

    for (uint32_t i = 0; i < cache_levels; i++) {
        if (this->sets[idx].lines[line][i].cache_line != NULL)
        {
            // this->sets[idx].lines[line][i].cache_line->clean_line();
            this->sets[idx].lines[line][i].cache_line->directory_line = NULL;
            // this->sets[idx].lines[line][i].clean_line();
            this->sets[idx].lines[line][i].cache_line = NULL;
        }
    }
}

// Return address index in cache
void directory_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag, uint32_t offset) {
    uint32_t get_bits = this->n_sets - 1;
    *tag = address >> offset;
    *idx = *tag & get_bits;
}

uint32_t directory_t::validCacheLine(uint64_t address, uint32_t cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag, 6);
    int32_t line = directory.getDirectoryLine(idx, tag);
    if (this->sets[idx].lines[line][cache_level].cache_line != NULL) {
        return 1;
    }
    return 0;
}

uint32_t directory_t::dirtyCacheLine(uint64_t address, uint32_t cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag, 6);
    int32_t line = directory.getDirectoryLine(idx, tag);
    if (this->sets[idx].lines[line][cache_level].cache_line->dirty == 1) {
        return 1;
    }
    return 0;
}

uint32_t directory_t::copyCacheInfo(uint64_t address, uint32_t to_cache_level, uint32_t cache_level) {
    uint32_t idx;
    uint64_t tag;
    this->tagIdxSetCalculation(address, &idx, &tag, 6);
    int32_t line = directory.getDirectoryLine(idx, tag);
    this->sets[idx].lines[line][to_cache_level].cache_line->dirty = this->sets[idx].lines[line][cache_level].cache_line->dirty;
    this->sets[idx].lines[line][to_cache_level].cache_line->lru = orcs_engine.get_global_cycle();
    this->sets[idx].lines[line][to_cache_level].cache_line->ready_at = this->sets[idx].lines[line][cache_level].cache_line->ready_at;
}