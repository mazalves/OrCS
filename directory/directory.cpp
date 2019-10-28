#include "../simulator.hpp"

directory_t::directory_t() {
    this->n_sets = 0;
    this->sets = NULL;
}

directory_t::~directory_t() {
    for (uint32_t i = 0; i < this->n_sets; i++) {
        for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
            delete[] this->sets[i].lines[j];
        }
    }
    delete[] sets;
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
