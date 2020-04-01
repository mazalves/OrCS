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
            if (dirty) {
                if (sub_ready >= no_sub_requests){
                    sub_ready = 0;
                    for (uint32_t i = 0; i < get_no_sub_requests(); i++){
                        sub_requests[i].memory_operation = MEMORY_OPERATION_WRITE;
                        sub_requests[i].status = PACKAGE_STATE_UNTREATED;
                        sub_requests[i].sent_to_ram = false;
                        sub_requests[i].row_buffer = false;
                        sub_requests[i].born_cycle = orcs_engine.get_global_cycle();
                        orcs_engine.memory_controller->requestDRAM (&sub_requests[i]);
                    }
                } else {
                    //print_vector();
                    while (sub_requests[sub_ready].status == PACKAGE_STATE_WAIT) {
                        //ORCS_PRINTF ("%lu sub-request %u born at %lu, finished at %lu. Took %lu cycles.\n", orcs_engine.get_global_cycle(), sub_ready, sub_requests[sub_ready].born_cycle, orcs_engine.get_global_cycle(), orcs_engine.get_global_cycle()-sub_requests[sub_ready].born_cycle)
                        sub_ready++;
                    }
                }
                if (sub_ready >= no_sub_requests) {
                    if (VIMA_DEBUG) ORCS_PRINTF ("%lu %s WRITEBACK FINISHED!\n", orcs_engine.get_global_cycle(), label)
                    status = PACKAGE_STATE_WAIT;
                }
            } else {
                //ORCS_PRINTF ("%lu %s %lu WRITEBACK NOT NEEDED, NOT DIRTY!\n", orcs_engine.get_global_cycle(), label, address)
                status = PACKAGE_STATE_WAIT;
            }
            //setar todas as instruções para STORE
            //enviar todas as instruções para a DRAM
            //contar quantas estão prontas
            //if contagem == total, torna-se WAIT
            break;
        case PACKAGE_STATE_WAIT:
            //fetch
            if (sub_ready >= no_sub_requests){
                sub_ready = 0;
                uint64_t memory_address = address >> sub_req_offset;
                for (uint32_t i = 0; i < get_no_sub_requests(); i++){
                    sub_requests[i].memory_operation = MEMORY_OPERATION_READ;
                    sub_requests[i].status = PACKAGE_STATE_UNTREATED;
                    sub_requests[i].sent_to_ram = false;
                    sub_requests[i].row_buffer = false;
                    sub_requests[i].memory_address = memory_address << sub_req_offset;
                    sub_requests[i].born_cycle = orcs_engine.get_global_cycle();
                    memory_address++;

                    orcs_engine.memory_controller->requestDRAM (&sub_requests[i]);
                } 
            } else {
                while (sub_requests[sub_ready].status == PACKAGE_STATE_WAIT) {
                    //ORCS_PRINTF ("%lu sub-request %u born at %lu, finished at %lu. Took %lu cycles.\n", orcs_engine.get_global_cycle(), sub_ready, sub_requests[sub_ready].born_cycle, orcs_engine.get_global_cycle(), orcs_engine.get_global_cycle()-sub_requests[sub_ready].born_cycle)
                    sub_ready++;
                }
            }
            if (sub_ready >= no_sub_requests) {
                if (VIMA_DEBUG) ORCS_PRINTF ("%lu %s %lu FETCH FINISHED!\n", orcs_engine.get_global_cycle(), label, address)
                dirty = false;
                lru = orcs_engine.get_global_cycle();
                status = PACKAGE_STATE_READY;
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

bool vima_vector_t::ready(){
    if (sub_ready >= no_sub_requests && status == PACKAGE_STATE_READY) return true;
    else return false;
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

    this->label = (const char*) malloc (127*sizeof(const char));
    //std::memset (this->label, 0, 127*sizeof(const char));

    for (size_t i = 0; i < get_no_sub_requests(); i++) {
        sub_requests[i].is_vima = true;
        sub_requests[i].status = PACKAGE_STATE_FREE;
    }

    status = PACKAGE_STATE_FREE;
    sub_ready = 128;
    last_ready = 0;
    sub_req_offset = utils_t::get_power_of_two(LINE_SIZE);
    set = false;
}