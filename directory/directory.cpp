#include "../simulator.hpp"

#ifdef DIRECTORY_DEBUG
#define DIRECTORY_DEBUG_PRINTF(...) DEBUG_PRINTF(__VA_ARGS__);
#else
#define DIRECTORY_DEBUG_PRINTF(...)
#endif

directory_t::directory_t()
{
    this->n_sets = 0;
    this->sets = NULL;
}

directory_t::~directory_t()
{
    // for (uint32_t i = 0; i < this->n_sets; i++) {
    //     for (uint32_t j = 0; j < this->sets[i].n_ways; j++) {
    //         delete[] this->sets[i].ways[j];
    //     }
    // }
    // delete[] sets;
}

void directory_t::allocate(cache_t llc, uint32_t NUMBER_OF_PROCESSORS, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS) {
    set_INSTRUCTION_LEVELS(INSTRUCTION_LEVELS);
    set_DATA_LEVELS(DATA_LEVELS);
    set_POINTER_LEVELS((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
    this->n_sets = llc.n_sets;
    this->sets = new directory_set_t[this->n_sets];
    for (uint32_t i = 0; i < this->n_sets; i++) {
        this->sets[i].allocate(llc.associativity, NUMBER_OF_PROCESSORS, INSTRUCTION_LEVELS, DATA_LEVELS);
    }
    set_OFFSET(llc.offset);
}

// void directory_t::installCachePointers(line_t ***cache_ways, uint32_t n_proc, uint32_t data_levels, uint32_t inst_levels, memory_operation_t mem_op, uint32_t idx, int32_t way)
// {
//     uint32_t i, j;
//     for (i = 0; i < n_proc; i++)
//     {
//         if (mem_op == MEMORY_OPERATION_INST)
//         {
//             for (j = 0; j < inst_levels; j++)
//             {
//                 this->sets[idx].ways[way].inst_cache[i][j].cache_way = cache_ways[i][j];
//                 this->sets[idx].ways[way].inst_cache[i][j].shared = 1;
//                 this->sets[idx].ways[way].inst_cache[i][j].cache_status = CACHED;
//                 this->sets[idx].ways[way].tag = cache_ways[i][j]->tag;
//                 cache_ways[i][j]->directory_line = &this->sets[idx].ways[way];
//                 // printf("inst cache %lu (TAG %lu) in directory in level %u\n", this->sets[idx].ways[way].inst_cache[i][j].cache_way->address, cache_ways[i][j]->directory_line->tag, j);
//             }
//         }
//         else
//         {
//             j = 0;
//         }
//         for (; j < data_levels; j++)
//         {
//             this->sets[idx].ways[way].data_cache[i][j].cache_way = cache_ways[i][j];
//             this->sets[idx].ways[way].data_cache[i][j].shared = 1;
//             this->sets[idx].ways[way].data_cache[i][j].cache_status = CACHED;
//             this->sets[idx].ways[way].tag = cache_ways[i][j]->tag;
//             cache_ways[i][j]->directory_line = &this->sets[idx].ways[way];
//             // printf("data cache %lu (TAG %lu) in directory in level %u\n", this->sets[idx].ways[way].data_cache[i][j].cache_way->address, cache_ways[i][j]->directory_line->tag, j);
//         }
//     }
// }

// // Return address index in cache
void directory_t::tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag) {
    uint32_t get_bits = this->n_sets - 1;
    *tag = address >> OFFSET;
    *idx = *tag & get_bits;
}

int32_t directory_t::searchAddress(memory_package_t *mob_line) {
    uint64_t address;
    int32_t cache_level;
    if (mob_line->memory_operation == MEMORY_OPERATION_INST) {
        address = mob_line->opcode_address;
    } else {
        address = mob_line->memory_address;
    }
    cache_level = this->read(address, mob_line->memory_operation);
    return cache_level;
}

uint32_t directory_t::checkInclusionPolicy(directory_way_t *way, memory_operation_t mem_op, uint32_t level) {
    uint32_t i = level;
    if (mem_op == MEMORY_OPERATION_INST) {
        for (; i < INSTRUCTION_LEVELS; i++) {
            if (way->inst_cache[i]->cache_way == NULL) {
                return FAIL;
            }
        }
    } else {
        i = 0;
    }
    for (; i < DATA_LEVELS; i++) {
        if (way->data_cache[i]->cache_way == NULL) {
            return FAIL;
        }
    }
    return OK;
}

int32_t directory_t::read(uint64_t address, memory_operation_t mem_op) {
    uint64_t tag;
    uint32_t idx, i, j;
    this->tagIdxSetCalculation(address, &idx, &tag);
    for (i = 0; i < this->n_sets; i++) {
        if (this->sets[idx].ways[i].tag == tag) {
            if (mem_op == MEMORY_OPERATION_INST) {
                for (j = 0; j < INSTRUCTION_LEVELS; j++) {
                    if (this->sets[idx].ways[i].inst_cache[j]->cache_way != NULL) {
                        if (this->checkInclusionPolicy(&this->sets[idx].ways[i], mem_op, j + 1)) {
                            return (int32_t)j;
                        }
                    }
                }
            } else {
                j = 0;
            }
            for (; j < DATA_LEVELS; j++) {
                if (this->sets[idx].ways[i].data_cache[j]->cache_way != NULL) {
                    if (this->checkInclusionPolicy(&this->sets[idx].ways[i], mem_op, j + 1)) {
                        return (int32_t)j;
                    }
                }
            }
        }
    }
    return POSITION_FAIL;
}

// void directory_t::setCachePointers(line_t *cache_way, uint32_t cache_level, memory_operation_t mem_op) {
//     uint32_t idx;
//     uint64_t tag;

//     this->tagIdxSetCalculation(cache_way->address, &idx, &tag);
//     int32_t way = this->getDirectoryLine(idx, tag);

//     DIRECTORY_DEBUG_PRINTF("Installing address %lu (TAG %lu) in directory in %s\n", cache_way->address, cache_way->tag, get_cache_level_char(cache_level));
//     printf("index %u - way %d - tag %lu - cache %lu - cache_level %u diretório %lu\n", idx, way, tag, cache_way->tag, cache_level, this->sets[idx].ways[way].tag);
//     if (cache_way->tag == tag) {
//         printf("L2: %lu %lu\n", this->sets[idx].ways[way].cache_level[1].cache_way->address, this->sets[idx].ways[way].cache_level[1].cache_way->tag);
//         // printf("L3: %lu %lu\n", this->sets[idx].ways[way].cache_level[2].cache_way->address, this->sets[idx].ways[way].cache_level[2].cache_way->tag);
//         // this->sets[idx].ways[way].cache_level[cache_level].cache_way = cache_way;
//         // this->sets[idx].ways[way].cache_level[cache_level].shared = 1;
//         // this->sets[idx].ways[way].cache_level[cache_level].cache_status = CACHED;
//         // if (mem_op == MEMORY_OPERATION_INST) {
//         //     this->sets[idx].ways[way].id = INSTRUCTION;
//         // } else {
//         //     this->sets[idx].ways[way].id = DATA;
//         // }
//         // this->sets[idx].ways[way].tag = tag;
//         if (mem_op == MEMORY_OPERATION_FREE) printf("oi\n");
//         cache_way->directory_line = &this->sets[idx].ways[way];
//         cache_way->directory_line->cache_level[cache_level].cache_way = cache_way;
//     } else {
//         printf("deu ruim \n");
//     }
// }

// int32_t directory_t::getDirectoryLine(uint32_t idx, uint64_t tag) {
//     int32_t way = POSITION_FAIL;
//     for (size_t i = 0; i < this->sets[idx].n_ways; i++) {
//         if (this->sets[idx].ways[way].tag == tag) {
//             way = i;
//             break;
//         }
//     }
//     return way;
// }

// void directory_t::nullingCaches(uint64_t address, uint32_t cache_levels) {
//     DIRECTORY_DEBUG_PRINTF("Delete all the pointers between cache and directory\n");
//     uint32_t idx;
//     uint64_t tag;

//     this->tagIdxSetCalculation(address, &idx, &tag);
//     int32_t way = this->getDirectoryLine(idx, tag);    //TODO renomear para VIA do diretorio e não linha
//     DIRECTORY_DEBUG_PRINTF("Address %lu found in set %u and way %u in directory.\n", address, idx, way);

//     for (uint32_t i = 0; i < cache_levels; i++) {
//         if (this->sets[idx].ways[way].cache_level[i].cache_way != NULL) {
//             // this->sets[idx].ways[way][i].cache_way->directory_way = NULL;
//             // this->sets[idx].ways[way][i].cache_way = NULL;
//             this->sets[idx].ways[way].cache_level[i].cache_way->clean_line();
//             this->sets[idx].ways[way].cache_level[i].clean_way();
//         }
//     }
// }


// uint32_t directory_t::validCacheLine(uint64_t address, uint32_t cache_level) {
//     uint32_t idx;
//     uint64_t tag;
//     this->tagIdxSetCalculation(address, &idx, &tag);
//     int32_t way = this->getDirectoryLine(idx, tag);
//     if (this->sets[idx].ways[way].cache_level[cache_level].cache_way != NULL) {
//         return 1;
//     }
//     return 0;
// }

// uint32_t directory_t::dirtyCacheLine(uint64_t address, uint32_t cache_level) {
//     uint32_t idx;
//     uint64_t tag;
//     this->tagIdxSetCalculation(address, &idx, &tag);
//     int32_t way = this->getDirectoryLine(idx, tag);
//     if (this->sets[idx].ways[way].cache_level[cache_level].cache_way->dirty == 1) {
//         return 1;
//     }
//     return 0;
// }

// void directory_t::copyCacheInfo(uint64_t address, uint32_t to_cache_level, uint32_t from_cache_level) {
//     uint32_t idx;
//     uint64_t tag;
//     this->tagIdxSetCalculation(address, &idx, &tag);
//     int32_t way = this->getDirectoryLine(idx, tag);

//     DIRECTORY_DEBUG_PRINTF("%s receives %s\n", get_cache_level_char(to_cache_level), get_cache_level_char(from_cache_level));
//     if (this->sets[idx].ways[way].cache_level[to_cache_level].cache_way == NULL) {
//         DIRECTORY_DEBUG_PRINTF("%s is NULL\n", get_cache_level_char(to_cache_level));
//     }
//     if (this->sets[idx].ways[way].cache_level[from_cache_level].cache_way == NULL) {
//         DIRECTORY_DEBUG_PRINTF("%s is NULL\n", get_cache_level_char(from_cache_level));
//     }
//     this->sets[idx].ways[way].cache_level[to_cache_level].cache_way->dirty = this->sets[idx].ways[way].cache_level[from_cache_level].cache_way->dirty;
//     this->sets[idx].ways[way].cache_level[to_cache_level].cache_way->lru = orcs_engine.get_global_cycle();
//     this->sets[idx].ways[way].cache_level[to_cache_level].cache_way->ready_at = this->sets[idx].ways[way].cache_level[from_cache_level].cache_way->ready_at;
// }

// void directory_t::nullCachePointer(uint64_t address, uint32_t cache_level) {
//     uint32_t idx;
//     uint64_t tag;
//     this->tagIdxSetCalculation(address, &idx, &tag);
//     int32_t way = this->getDirectoryLine(idx, tag);

//     if (this->sets[idx].ways[way].cache_level[cache_level].cache_way != NULL) {
//         // this->sets[idx].ways[way][cache_level].cache_way->clean_way();
//         // this->sets[idx].ways[way][cache_level].clean_way();
//         this->sets[idx].ways[way].cache_level[cache_level].cache_way->directory_line = NULL;
//         this->sets[idx].ways[way].cache_level[cache_level].cache_way = NULL;
//     }
// }