#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {
    this->row_buffer = false;
    this->op_count = (uint64_t*) malloc (MEMORY_OPERATION_LAST*sizeof(uint64_t));
    for (uint64_t i = 0; i < MEMORY_OPERATION_LAST; i++) this->op_count[i] = 0;

    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
    libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_DEBUG (cfg_processor["DEBUG"]);
}

memory_package_t::~memory_package_t(){
    vector<memory_request_client_t*>().swap(this->clients);
    free (op_count);
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_UNTREATED;
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%s\n", get_enum_package_state_char (status))
    
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
}
void memory_package_t::updatePackageReady(){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_READY;
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%s\n", get_enum_package_state_char (status))
}
void memory_package_t::updatePackageWait (uint32_t stallTime){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_WAIT;
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%s\n", get_enum_package_state_char (status))
    
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
}

void memory_package_t::updatePackageFree (uint32_t stallTime){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) {
        ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    }
    this->status = PACKAGE_STATE_FREE;
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) {
        ORCS_PRINTF ("%s\n", get_enum_package_state_char (status))
    }
    
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
}

void memory_package_t::updatePackageHive (uint32_t stallTime){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_HIVE;
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%s\n", get_enum_package_state_char (status))
    
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
}

void memory_package_t::updatePackageTransmit (uint32_t stallTime){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_TRANSMIT;
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("%s\n", get_enum_package_state_char (status))
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
}

void memory_package_t::printPackage(){
    if (memory_operation != MEMORY_OPERATION_INST && DEBUG) ORCS_PRINTF ("Memory Package\nAddress: %lu | Operation: %s | Status: %s | Uop: %lu | ReadyAt: %lu\n", memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status), uop_number, readyAt)
}

void memory_package_t::updateClients(){
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (0);
    }
}