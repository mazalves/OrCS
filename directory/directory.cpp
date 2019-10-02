directory_t::directory_t() {
    this->n_sets = 0;
    this->sets = NULL;
    this->associativity = 0;
    this->size = 0;
}

// TODO: remover todos os diretórios
directory_t::~directory_t() {
    delete[] sets;
}

void directory_t::allocate() {
     this->sets = new directory_set_t[this->n_sets];
     for (uint32_t i = 0; i < this->n_sets; i++) {
        this->sets[i].lines = new directory_line_t[this->associativity];
		this->sets[i].n_lines = this->associativity;
     }
}
