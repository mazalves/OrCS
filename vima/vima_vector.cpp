#include "./../simulator.hpp"
#include <string>

vima_vector_t::vima_vector_t(){
    this->no_sub_requests = 0;
    this->sub_ready = 0;
    this->address = 0;
    this->next_address = 0;
    this->sub_req_offset = 0;
    this->sub_requests = NULL;
    this->assoc = NULL;

    this->LINE_SIZE = 0;
    this->VIMA_VECTOR_SIZE = 0;
    
    status = PACKAGE_STATE_FREE;
    this->tag = 0;
    this->lru = 0;
    this->dirty = false;
    this->gather = false;
    this->scatter = false;
}
vima_vector_t::~vima_vector_t(){
    delete[] sub_requests;
}

void vima_vector_t::print_vector(){
    ORCS_PRINTF ("VIMA Vector: address %lu, tag %lu, lru %lu, %s\n", address, tag, lru, get_enum_package_state_char (status))
}

void vima_vector_t::fetch (bool random) {
    //fetch
    if (this->next_address != 0 && !issued){
        this->address = this->next_address;
        this->next_address = 0;
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Cache [%lu][%lu] FETCH of address %lu for instruction %lu STARTED!\n", orcs_engine.get_global_cycle(), this->set, this->column, this->address, this->assoc->uop_number)
        #endif
        this->fetch_start = orcs_engine.get_global_cycle();
        this->fetch_count++;
        sub_ready = 0;
        for (uint32_t i = 0; i < get_no_sub_requests(); i++){
            sub_requests[i].memory_operation = MEMORY_OPERATION_READ;
            sub_requests[i].status = PACKAGE_STATE_UNTREATED;
            sub_requests[i].sent_to_ram = false;
            sub_requests[i].row_buffer = false;
            if (perfect) sub_requests[i].memory_address = address + i*this->get_ROW_BUFFER_SIZE();
            else sub_requests[i].memory_address = address + i * this->get_LINE_SIZE();
            if (random) sub_requests[i].memory_address += (rand() % UINT32_MAX);
            sub_requests[i].born_cycle = orcs_engine.get_global_cycle();
            orcs_engine.memory_controller->requestDRAM (&sub_requests[i]);
        } 
        issued = true;
    }

    while (sub_ready < no_sub_requests && 
        sub_requests[sub_ready].status == PACKAGE_STATE_WAIT && 
        sub_requests[sub_ready].readyAt <= orcs_engine.get_global_cycle()) {
            //ORCS_PRINTF ("LOAD : %u/%u sub-requests are ready! addr = %lu\n", sub_ready, get_no_sub_requests(), sub_requests[sub_ready].memory_address);
            sub_ready++;   
    }
        
    if (sub_ready >= no_sub_requests) {
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Cache [%lu][%lu] FETCH of address %lu for instruction %lu FINISHED! Took %lu cycles.\n", orcs_engine.get_global_cycle(), this->set, this->column, this->address, this->assoc->uop_number, (orcs_engine.get_global_cycle() - this->fetch_start))
            ORCS_PRINTF ("%lu VIMA Cache [%lu][%lu] READY!\n", orcs_engine.get_global_cycle(), this->set, this->column)
        #endif
        this->fetch_latency_total += (orcs_engine.get_global_cycle() - this->fetch_start);
        dirty = false;
        lru = orcs_engine.get_global_cycle();
        status = PACKAGE_STATE_READY;
        next_address = 0;
        issued = false;
    }
    //setar os novos endereços das instruções
    //setar todas as instruções para LOAD
    //enviar todas as instruções para  DRAM
    //contar quantas estão prontas
    //if contagem == total, torna-se READY
}

void vima_vector_t::writeback (bool random) {
    if (dirty) {
        if (!issued){
            #if VIMA_DEBUG 
                ORCS_PRINTF ("%lu VIMA Cache [%lu][%lu] WRITEBACK of address %lu, lru = %lu STARTED!\n", orcs_engine.get_global_cycle(), this->set, this->column, this->address, this->lru)
            #endif
            this->writeback_start = orcs_engine.get_global_cycle();
            this->writeback_count++;
            sub_ready = 0;
            for (uint32_t i = 0; i < get_no_sub_requests(); i++){
                sub_requests[i].memory_operation = MEMORY_OPERATION_WRITE;
                sub_requests[i].status = PACKAGE_STATE_UNTREATED;
                sub_requests[i].sent_to_ram = false;
                sub_requests[i].row_buffer = false;
                if (perfect) sub_requests[i].memory_address = address + i * this->get_ROW_BUFFER_SIZE();
                else sub_requests[i].memory_address = address + i * this->get_LINE_SIZE();
                if (random) sub_requests[i].memory_address += (rand() % UINT32_MAX);
                sub_requests[i].born_cycle = orcs_engine.get_global_cycle();
                orcs_engine.memory_controller->requestDRAM (&sub_requests[i]);
            } 
            issued = true;
    }

    while (sub_ready < no_sub_requests && 
        sub_requests[sub_ready].status == PACKAGE_STATE_WAIT && 
        sub_requests[sub_ready].readyAt <= orcs_engine.get_global_cycle()) {
            //ORCS_PRINTF ("STORE: %u/%u sub-requests are ready! addr = %lu\n", sub_ready, get_no_sub_requests(), sub_requests[sub_ready].memory_address);
            sub_ready++;
        }
    }
    
    if (sub_ready >= no_sub_requests) {
        #if VIMA_DEBUG 
            ORCS_PRINTF ("%lu VIMA Cache [%lu][%lu] WRITEBACK of address %lu FINISHED! Took %lu cycles.\n", orcs_engine.get_global_cycle(), this->set, this->column, this->address, (orcs_engine.get_global_cycle() - this->writeback_start))
        #endif
        this->writeback_latency_total += (orcs_engine.get_global_cycle() - this->writeback_start);
        status = PACKAGE_STATE_WAIT;
        issued = false;
    }
    //setar todas as instruções para STORE
    //enviar todas as instruções para a DRAM
    //contar quantas estão prontas
    //if contagem == total, torna-se WAIT
}

void vima_vector_t::clock() {
    if (this->assoc != NULL){
        if (this->assoc->status == PACKAGE_STATE_READY){
            this->assoc = NULL;
            #if VIMA_DEBUG 
                ORCS_PRINTF ("%lu VIMA Cache [%lu][%lu] is now free.\n", orcs_engine.get_global_cycle(), this->set, this->column)
            #endif
        }
    }
    switch (status){
        case PACKAGE_STATE_TRANSMIT: 
            if (this->scatter) this->writeback(true);
            else this->writeback(false);
            break;
        case PACKAGE_STATE_WAIT: 
            if (this->gather) this->fetch (true);
            else this->fetch (false);
            break;
        default: break;
    }
}

bool vima_vector_t::ready(){
    if (sub_ready >= no_sub_requests && status == PACKAGE_STATE_READY) return true;
    else return false;
}

void vima_vector_t::allocate() {
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["VIMA_CONTROLLER"];
    libconfig::Setting &cfg_cache_defs = cfg_root["CACHE_MEMORY"];
    libconfig::Setting &cfg_memory_ctrl = cfg_root["MEMORY_CONTROLLER"];
    set_LINE_SIZE(cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_VIMA_VECTOR_SIZE (cfg_processor["VIMA_VECTOR_SIZE"]);
    set_ROW_BUFFER_SIZE (cfg_memory_ctrl["BANK_ROW_BUFFER_SIZE"]);
    if (perfect) set_no_sub_requests (get_VIMA_VECTOR_SIZE()/get_ROW_BUFFER_SIZE());
    else set_no_sub_requests (get_VIMA_VECTOR_SIZE()/get_LINE_SIZE());

    this->sub_requests = new memory_package_t[this->no_sub_requests]();

    for (size_t i = 0; i < get_no_sub_requests(); i++) {
        sub_requests[i].is_vima = true;
        sub_requests[i].status = PACKAGE_STATE_FREE;
    }

    status = PACKAGE_STATE_FREE;
    sub_ready = no_sub_requests;
    sub_req_offset = utils_t::get_power_of_two(LINE_SIZE);
}
