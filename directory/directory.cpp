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
            }
        }
    }
}

void directory_t::setPointers(line_t ***cache_lines, uint32_t n_processors, uint32_t cache_levels, uint32_t idx, int32_t line, memory_operation_t mem_op, uint64_t *cache_tags) {
    for (uint32_t i = 0; i < n_processors; i++) {
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