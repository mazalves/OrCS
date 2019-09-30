directory_line_t::directory_line_t() {}

directory_line_t::~directory_line_t() {}

// Allocates the number of caches in the system
void directory_line_t::allocate() { 
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &inst_cache_defs = cfg_root["CACHE_MEMORY"]["INSTRUCTION"];
    uint32_t n_icaches = inst_cache_defs.getLength();
    libconfig::Setting &data_cache_defs = cfg_root["CACHE_MEMORY"]["DATA"];
    uint32_t n_dcaches += data_cache_defs.getLength();
    set_POINTER_LEVELS(((n_icaches > n_dcaches) ? n_icaches : n_dcaches));

    cache_lines = new *cache_t[POINTER_LEVELS - 1];
}