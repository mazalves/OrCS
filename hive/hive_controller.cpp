#include "./../simulator.hpp"
#include <string>

hive_controller_t::hive_controller_t(){

}

hive_controller_t::~hive_controller_t(){
    utils_t::template_delete_array(this->hive_register_free);
    for (size_t i = 0; i< this->HIVE_REGISTERS; i++){
        this->hive_registers[i].del();
    }
    free (this->hive_registers);
}

void hive_controller_t::clock(){
    if (hive_instructions.size() == 0) return;

    for (size_t i = 0; i < this->HIVE_REGISTERS; i++){
        this->hive_registers[i].clock();
    }

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
        this->last_instruction = current_entry->uop_number;
        if (current_entry->memory_operation == MEMORY_OPERATION_HIVE_UNLOCK) {
            this->hive_lock = false;
            current_entry->status = PACKAGE_STATE_READY;
        }
        else if (current_entry->memory_operation == MEMORY_OPERATION_HIVE_LOAD || current_entry->memory_operation == MEMORY_OPERATION_HIVE_STORE){
            if (!this->installInstruction (current_entry)) {
                ORCS_PRINTF ("ALL HIVE REGISTERS BUSY\n")
            }
        } else {
            current_entry->status = PACKAGE_STATE_READY;
        }
    } else return;

    if (current_entry->status == PACKAGE_STATE_READY){
        hive_instructions.erase (std::remove (hive_instructions.begin(), hive_instructions.end(), current_entry), hive_instructions.end());
    }
}

bool hive_controller_t::installInstruction (memory_package_t* memory_instruction){
    for (size_t i = 0; i < this->HIVE_REGISTERS; i++){
        if (hive_registers[i].installRequest (memory_instruction)) {
            ORCS_PRINTF ("Instalado no registrador %lu!\n", i)
            return true;
        }
    }
    return false;
}

void hive_controller_t::allocate(){
    libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root[0]["PROCESSOR"];
    set_HIVE_REGISTERS (cfg_processor["HIVE_REGISTERS"]);

    this->hive_lock = false;
    
    this->hive_register_free = utils_t::template_allocate_initialize_array<bool>(this->HIVE_REGISTERS, 0);
    this->hive_registers = (hive_register_t*) malloc (this->HIVE_REGISTERS*sizeof (hive_register_t));
    std::memset(this->hive_registers,0,(this->HIVE_REGISTERS*sizeof(hive_register_t*)));
    for (uint32_t i = 0; i < this->HIVE_REGISTERS; i++) {
        this->hive_registers[i].allocate();
    }
}

void hive_controller_t::addRequest (memory_package_t* request){
    hive_instructions.push_back (request);
    ORCS_PRINTF ("%s %lu boop\n", get_enum_memory_operation_char (request->memory_operation), request->uop_number)
}