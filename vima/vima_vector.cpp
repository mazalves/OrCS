#include "./../simulator.hpp"
#include <string>

vima_vector_t::vima_vector_t(){

}
        
vima_vector_t::~vima_vector_t(){

}

void vima_vector_t::clock(){

}

void vima_vector_t::allocate(){
    set_cycle_ready (UINT64_MAX);
    set_cycle_used (UINT64_MAX);
    set_tag (0);
    dirty = false;
    ready = false;

    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_VIMA_VECTOR_SIZE (cfg_processor["VIMA_VECTOR_SIZE"]);

    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);

    set_no_sub_requests (this->get_VIMA_VECTOR_SIZE()/this->get_LINE_SIZE());

    this->sub_requests = (memory_package_t*) malloc (this->get_no_sub_requests()*sizeof (memory_package_t));
    std::memset (this->sub_requests, 0, this->get_no_sub_requests()*sizeof (memory_package_t));
}