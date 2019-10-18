directory_line_t::directory_line_t() {
    this->cache_lines = NULL;
    this->shared = 0;
    this->status = INVALID;
    this->level = 0;
    this->id = 0;
}

directory_line_t::~directory_line_t() {}