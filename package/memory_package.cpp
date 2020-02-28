#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {
    this->latency = 0;
    this->row_buffer = false;
    this->op_count = (uint64_t*) malloc (MEMORY_OPERATION_LAST*sizeof(uint64_t));
    for (uint64_t i = 0; i < MEMORY_OPERATION_LAST; i++) this->op_count[i] = 0;

    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
	libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_MSHR_DEBUG (cfg_processor["MSHR_DEBUG"]);
}

memory_package_t::~memory_package_t(){
    vector<memory_request_client_t*>().swap(this->clients);
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::updatePackageReady(){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_READY;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency)
}

void memory_package_t::updatePackageWait (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_WAIT;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::updatePackageTransmit (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_TRANSMIT;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::updatePackageFree (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_FREE;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}
void memory_package_t::updatePackageHive (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_HIVE;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::printPackage(){
    ORCS_PRINTF ("Memory Package\nAddress: %lu | Operation: %s | Status: %s | Uop: %lu | ReadyAt: %lu\n", memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status), uop_number, readyAt)
}

void memory_package_t::updateClients(){
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (0);
    }
}