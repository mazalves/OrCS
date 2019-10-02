#include "./../simulator.hpp"
#include <string>

hive_controller_t::hive_controller_t(){

}

hive_controller_t::~hive_controller_t(){

}

void hive_controller_t::clock(){
    if (hive_instructions.size() == 0) return;

    memory_package_t* current_entry = NULL;
    if (hive_lock){
        for (size_t i = 0; i < hive_instructions.size(); i++){
            if (hive_instructions[i]->uop_number == last_instruction+1){
                current_entry = hive_instructions[i];
                break;
            }
        }
    } else {
        for (size_t i = 0; i < hive_instructions.size(); i++){
            if (hive_instructions[i]->memory_operation == MEMORY_OPERATION_HIVE_LOCK && hive_instructions[i]->status != PACKAGE_STATE_READY){
                this->hive_lock = true;
                current_entry = hive_instructions[i];
                break;
            }
        }
    }

    if (current_entry != NULL){
        if (current_entry->memory_operation == MEMORY_OPERATION_HIVE_UNLOCK) this->hive_lock = false;
        this->last_instruction = current_entry->uop_number;
        current_entry->status = PACKAGE_STATE_READY;
        hive_instructions.erase (std::remove (hive_instructions.begin(), hive_instructions.end(), current_entry), hive_instructions.end());
    }
}

void hive_controller_t::allocate(){
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root[0]["PROCESSOR"];
    libconfig::Setting &cfg_cache_defs = cfg_root[0]["CACHE_MEMORY"];
    set_LINE_SIZE (cfg_cache_defs["CONFIG"]["LINE_SIZE"]);
    set_HIVE_REGISTERS (cfg_processor["HIVE_REGISTERS"]);
    set_HIVE_REGISTER_SIZE (cfg_processor["HIVE_REGISTER_SIZE"]);

    this->hive_register_ready = utils_t::template_allocate_initialize_matrix<bool>(this->HIVE_REGISTERS, HIVE_REGISTER_SIZE/LINE_SIZE, 0);
}

void hive_controller_t::addRequest (memory_package_t* request){
    hive_instructions.push_back (request);
}