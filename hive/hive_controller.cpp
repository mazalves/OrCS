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
    for (size_t i = 0; i < this->HIVE_REGISTERS; i++) this->hive_registers[i].clock();

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
        switch (current_entry->memory_operation){
            case MEMORY_OPERATION_HIVE_LOAD:
                if (!hive_registers[current_entry->hive_write].installRequest (current_entry)){
                    return;
                }
                break;
            case MEMORY_OPERATION_HIVE_STORE:
                if (!hive_registers[current_entry->hive_read1].installRequest (current_entry)) {
                    return;
                }
                break;
            case MEMORY_OPERATION_HIVE_INT_ALU:
            case MEMORY_OPERATION_HIVE_INT_DIV:
            case MEMORY_OPERATION_HIVE_INT_MUL:
            case MEMORY_OPERATION_HIVE_FP_ALU:
            case MEMORY_OPERATION_HIVE_FP_DIV:
            case MEMORY_OPERATION_HIVE_FP_MUL:
                if ((current_entry->hive_read2 != -1 && hive_registers[current_entry->hive_read2].status == PACKAGE_STATE_READY) || current_entry->hive_write != -1){
                    current_entry->latency += this->hive_op_latencies[current_entry->memory_operation];
                    current_entry->status = PACKAGE_STATE_READY;
                    hive_registers[current_entry->hive_read2].status = PACKAGE_STATE_FREE;
                }
                break;
            case MEMORY_OPERATION_HIVE_UNLOCK:
                this->hive_lock = false;
                current_entry->status = PACKAGE_STATE_READY;
                for (uint32_t i = 0; i < this->HIVE_REGISTERS; i++) hive_registers[i].status = PACKAGE_STATE_FREE;
                break;
            case MEMORY_OPERATION_HIVE_LOCK:
                current_entry->status = PACKAGE_STATE_READY;
                break;
            default:
                break;
        }
    } else return;

    if (current_entry->status == PACKAGE_STATE_READY){
        hive_instructions.erase (std::remove (hive_instructions.begin(), hive_instructions.end(), current_entry), hive_instructions.end());
    }
}

void hive_controller_t::allocate(){
    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_HIVE_REGISTERS (cfg_processor["HIVE_REGISTERS"]);
    
    this->hive_lock = false;
    this->hive_op_latencies = utils_t::template_allocate_initialize_array<uint32_t>(MEMORY_OPERATION_HIVE_FP_MUL, 0);
    this->hive_register_free = utils_t::template_allocate_initialize_array<bool>(this->HIVE_REGISTERS, 0);
    this->hive_registers = (hive_register_t*) malloc (this->HIVE_REGISTERS*sizeof (hive_register_t));
    std::memset(this->hive_registers,0,(this->HIVE_REGISTERS*sizeof(hive_register_t*)));
    for (uint32_t i = 0; i < this->HIVE_REGISTERS; i++) {
        this->hive_registers[i].allocate();
    }

    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_ALU] = cfg_processor["HIVE_LATENCY_INT_ALU"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_DIV] = cfg_processor["HIVE_LATENCY_INT_DIV"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_INT_MUL] = cfg_processor["HIVE_LATENCY_INT_MUL"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_ALU] = cfg_processor["HIVE_LATENCY_FP_ALU"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_DIV] = cfg_processor["HIVE_LATENCY_FP_DIV"];
    hive_op_latencies[MEMORY_OPERATION_HIVE_FP_MUL] = cfg_processor["HIVE_LATENCY_FP_MUL"];

}

void hive_controller_t::addRequest (memory_package_t* request){
    hive_instructions.push_back (request);
}