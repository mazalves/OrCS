#include "./../simulator.hpp"
#include <string>

vima_vector_t::vima_vector_t(){

}

vima_vector_t::~vima_vector_t(){

}

void vima_vector_t::print_vector(){
    ORCS_PRINTF ("VIMA Vector: address %lu, tag %lu, lru %lu, %s\n", address, tag, lru, get_enum_package_state_char (status))
}

void vima_vector_t::clock() {
    switch (status){
        case PACKAGE_STATE_TRANSMIT:
            //writeback
            if (sub_requests[0].sent_to_ram){
                sub_ready = 0;
                for (uint32_t i = 0; i < get_no_sub_requests(); i++){
                    sub_requests[i].memory_operation = MEMORY_OPERATION_WRITE;
                    sub_requests[i].status = PACKAGE_STATE_UNTREATED;
                    sub_requests[i].sent_to_ram = false;
                }
            }
            status = PACKAGE_STATE_READY;
            //setar todas as instruções para STORE
            //enviar todas as instruções para a DRAM
            //contar quantas estão prontas
            //if contagem == total, torna-se FREE
            break;
        case PACKAGE_STATE_WAIT:
            //fetch
            if (sub_requests[0].sent_to_ram){
                sub_ready = 0;
                for (uint32_t i = 0; i < get_no_sub_requests(); i++){
                    sub_requests[i].memory_operation = MEMORY_OPERATION_READ;
                    sub_requests[i].status = PACKAGE_STATE_UNTREATED;
                    sub_requests[i].sent_to_ram = false;
                }
            }
            //setar os novos endereços das instruções
            //setar todas as instruções para LOAD
            //enviar todas as instruções para  DRAM
            //contar quantas estão prontas
            //if contagem == total, torna-se READY
            break;
        default:
            break;
    }
}

void vima_vector_t::allocate() {
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_processor["VIMA_VECTOR_SIZE"]);
    set_no_sub_requests (get_VIMA_VECTOR_SIZE()/get_LINE_SIZE());

    this->sub_requests = (memory_package_t*) malloc (get_no_sub_requests()*sizeof (memory_package_t));
    std::memset (this->sub_requests, 0, get_no_sub_requests()*sizeof(memory_package_t));

    ORCS_PRINTF ("Criando VIMA Vector!\n")
}