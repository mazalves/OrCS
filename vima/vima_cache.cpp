#include "./../simulator.hpp"
#include <string>

vima_cache_t::vima_cache_t(){

}

vima_cache_t::~vima_cache_t(){
    ORCS_PRINTF ("VIMA Cache ACCESSES %u\n", this->get_cache_accesses())
    ORCS_PRINTF ("VIMA Cache HITS %u\n", this->get_cache_hits())
    ORCS_PRINTF ("VIMA Cache MISSES %u\n", this->get_cache_misses())
}

void vima_cache_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_VIMA_CACHE_SIZE (cfg_processor["VIMA_CACHE_SIZE"]);
    set_VIMA_CACHE_LINE_SIZE (cfg_processor["VIMA_CACHE_LINE_SIZE"]);
    set_VIMA_CACHE_ASSOCIATIVITY (cfg_processor["VIMA_CACHE_ASSOCIATIVITY"]);
    set_VIMA_CACHE_LATENCY (cfg_processor["VIMA_CACHE_LATENCY"]);

    uint64_t lines = this->get_VIMA_CACHE_SIZE()/this->get_VIMA_CACHE_LINE_SIZE();
    uint64_t sets = lines/this->get_VIMA_CACHE_ASSOCIATIVITY();

    this->cache = (vima_cache_line_t**) malloc (sizeof (vima_cache_line_t*)*sets);
    std::memset (this->cache, 0, sizeof (vima_cache_line_t*)*sets);
    for (size_t i = 0; i < sets; i++){
        this->cache[i] = (vima_cache_line_t*) malloc (this->get_VIMA_CACHE_ASSOCIATIVITY()*sizeof (vima_cache_line_t));
        std::memset (this->cache[i], 0, this->get_VIMA_CACHE_ASSOCIATIVITY()*sizeof(vima_cache_line_t));
    }

    this->offset = utils_t::get_power_of_two (this->get_VIMA_CACHE_LINE_SIZE());
    this->index_bits_shift = utils_t::get_power_of_two(this->get_VIMA_CACHE_LINE_SIZE());
    this->tag_bits_shift = index_bits_shift + utils_t::get_power_of_two(sets);

    uint64_t i;
    /// INDEX MASK
    for (i = 0; i < utils_t::get_power_of_two(sets); i++) {
        this->index_bits_mask |= 1 << (i + index_bits_shift);
    }

    /// TAG MASK
    for (i = tag_bits_shift; i < utils_t::get_power_of_two((uint64_t)INT64_MAX+1); i++) {
        this->tag_bits_mask |= 1 << i;
    }
}

bool vima_cache_t::searchAddress (uint64_t address){
    ORCS_PRINTF ("%lu VIMA Cache: addr %lu, index %lu, tag %lu. ", orcs_engine.get_global_cycle(), address, this->get_index (address), this->get_tag (address))
    this->add_cache_accesses();
    vima_cache_line_t* set = cache[get_index(address)];
    for (size_t i = 0; i < this->get_VIMA_CACHE_ASSOCIATIVITY(); i++){
        if (set[i].tag == this->get_tag (address)) {
            set[i].cycle_used = orcs_engine.get_global_cycle();
            this->add_cache_hits();
            ORCS_PRINTF ("HIT!\n")
            return true;
        }
    }
    this->add_cache_misses();
    ORCS_PRINTF ("MISS!\n")
    return false;
}

void vima_cache_t::installLine (uint64_t address){
    vima_cache_line_t* set = cache[get_index(address)];
    for (size_t i = 0; i < this->get_VIMA_CACHE_ASSOCIATIVITY(); i++){
        if (set[i].cycle_used == 0) {
            set[i].cycle_used = orcs_engine.get_global_cycle();
            set[i].tag = this->get_tag (address);
            return;
        }
    }

    uint64_t lru = UINT64_MAX;
    uint32_t way = 0;
    for (size_t i = 0; i < this->get_VIMA_CACHE_ASSOCIATIVITY(); i++){
        if (set[i].cycle_used < lru) {
            lru = set[i].cycle_used;
            way = i;
        }
    }
    ORCS_PRINTF ("%lu VIMA Cache: install addr %lu, index %lu, tag %lu.\n", orcs_engine.get_global_cycle(), address, this->get_index (address), this->get_tag (address))
    set[way].cycle_used = orcs_engine.get_global_cycle();
    set[way].tag = this->get_tag (address);
    return;
}