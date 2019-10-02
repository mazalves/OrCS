#include "../simulator.hpp"

directory_t::directory_t() {
    this->n_sets = 0;
    this->sets = NULL;
}

directory_t::~directory_t() {
    delete[] sets;
}

void directory_t::allocate(cache_t llc) {
    printf("%s\n", "directory_t allocate");
    uint32_t n_caches = 2;
    this->n_sets = llc.n_sets;
    this->sets = new directory_set_t[this->n_sets];
    for (uint32_t i = 0; i < this->n_sets; i++) {
        this->sets[i].lines = new directory_line_t*[llc.associativity];
		this->sets[i].n_lines = llc.associativity;
        for (uint32_t j = 0; j < this->sets[i].n_lines; j++) {
            this->sets[i].lines[j] = new directory_line_t[n_caches];
        }
    }
}
