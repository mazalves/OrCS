#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {
    this->processor_id = 0;                  /// if (read / write) PROCESSOR.ID   else if (write-back / prefetch) CACHE_MEMORY.ID
    this->opcode_number = 0;                 /// initial opcode number
    this->opcode_address = 0;                /// initial opcode address
    this->uop_number = 0;                    /// initial uop number (Instruction == 0)
    this->memory_address = 0;                /// memory address
    this->memory_size = 0;                   /// operation size after offset

    status = PACKAGE_STATE_FREE;                  /// package state
    this->readyAt = 0;                   /// package latency
    this->born_cycle = 0;                    /// package create time
       
    sent_to_cache = false;
    sent_to_ram = false;
    is_hive = false;
    hive_read1 = 0;
    hive_read2 = 0;
    hive_write = 0;

    this->is_vima = false;
    this->vima_read1 = 0;
    this->vima_read2 = 0;
    this->vima_write = 0;

    row_buffer = false;
    type = DATA;
    op_count = new uint64_t[MEMORY_OPERATION_LAST]();
    this->latency = 0;

    memory_operation = MEMORY_OPERATION_LAST;    /// memory operation

    this->DEBUG = 0;
    this->MSHR_DEBUG = 0;

    libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
	libconfig::Setting &cfg_processor = cfg_root["PROCESSOR"][0];
    set_MSHR_DEBUG (cfg_processor["MSHR_DEBUG"]);
}

memory_package_t::~memory_package_t(){
    vector<memory_request_client_t*>().swap(this->clients);
    delete[] op_count;
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
    this->readyAt = orcs_engine.get_global_cycle();
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

void memory_package_t::updatePackageVima (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_VIMA;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::updatePackageDRAMFetch (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_DRAM_FETCH;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::updatePackageDRAMReady (uint32_t stallTime){
    if (MSHR_DEBUG) ORCS_PRINTF ("%lu %lu %s %s -> ", orcs_engine.get_global_cycle(), uop_number, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    this->status = PACKAGE_STATE_DRAM_READY;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    if (MSHR_DEBUG) ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
}

void memory_package_t::printPackage(){
    ORCS_PRINTF ("%lu Address: %lu | Operation: %s | Status: %s | Uop: %lu | ReadyAt: %lu\n", orcs_engine.get_global_cycle(), memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status), uop_number, readyAt)
}

void memory_package_t::updateClients(){
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (0);
    }
}