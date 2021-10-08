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
    this->ram_cycle = 0;
    this->vima_cycle = 0;
    this->hive_cycle = 0;
       
    sent_to_ram = false;
    next_level = L1;
    cache_latency = 0;
    is_hive = false;
    hive_read1 = 0;
    hive_read2 = 0;
    hive_write = 0;

    this->is_vima = false;
    this->vima_read1 = 0;
    this->vima_read1_vec = NULL;
    this->vima_read2 = 0;
    this->vima_read2_vec = NULL;
    this->vima_write = 0;
    this->vima_write_vec = NULL;

    this->is_vectorial_part = -1;

    row_buffer = false;
    type = DATA;
    op_count = new uint64_t[MEMORY_OPERATION_LAST]();
    sent_to_cache_level = new uint32_t[END]();
    sent_to_cache_level_at = new uint32_t[END]();
    this->latency = 0;

    memory_operation = MEMORY_OPERATION_LAST;    /// memory operation
}

memory_package_t::~memory_package_t(){
    vector<memory_request_client_t*>().swap(this->clients);
    delete[] op_count;
    delete[] sent_to_cache_level;
    delete[] sent_to_cache_level_at;
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    #if MEMORY_DEBUG 
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageReady(){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))        
    #endif
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle();
    #if MEMORY_DEBUG 
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency)
    #endif
}

void memory_package_t::updatePackageWait (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_WAIT;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageTransmit (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_TRANSMIT;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageFree (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_FREE;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageHive (uint32_t stallTime){
    #if MEMORY_DEBUG || HIVE_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_HIVE;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG || HIVE_DEBUG 
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageVima (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_VIMA;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageDRAMFetch (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_DRAM_FETCH;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::updatePackageDRAMReady (uint32_t stallTime){
    #if MEMORY_DEBUG
        ORCS_PRINTF ("[MEMP] %lu {%lu} %lu %s %s -> ", orcs_engine.get_global_cycle(), opcode_number, memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status))
    #endif
    this->status = PACKAGE_STATE_DRAM_READY;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency += stallTime;
    #if MEMORY_DEBUG
        ORCS_PRINTF ("%s, born: %lu, readyAt: %lu, latency: %u, stallTime: %u\n", get_enum_package_state_char (status), born_cycle, readyAt, latency, stallTime)
    #endif
}

void memory_package_t::printPackage(){
    ORCS_PRINTF ("%lu Address: %lu | Operation: %s | Status: %s | Uop: %lu | ReadyAt: %lu\n", orcs_engine.get_global_cycle(), memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status), uop_number, readyAt)
}

void memory_package_t::updateClients(){
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (0);
    }
}
