#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {
    this->latency = 0;
    this->memop_count = (uint64_t*) malloc (MEMORY_OPERATION_LAST*sizeof(uint64_t));
    for (uint64_t i = 0; i < MEMORY_OPERATION_LAST; i++) this->memop_count[i] = 0;
}

memory_package_t::~memory_package_t(){
    delete this->memop_count;
    vector<memory_request_client_t*>().swap(this->clients);
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    this->status = PACKAGE_STATE_UNTREATED;
    for (size_t i = 0; i < clients.size(); i++) clients[i]->updatePackageUntreated (stallTime);
}
void memory_package_t::updatePackageReady (uint32_t stallTime){
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle() + stallTime;
    this->latency = 0;
    //ORCS_PRINTF ("%lu memory_package_t::updatePackageReady(): uop %lu, %s, stallTime: %u, readyAt %lu.\n", orcs_engine.get_global_cycle(), this->uop_number, get_enum_memory_operation_char (this->memory_operation), stallTime, this->readyAt)
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (stallTime);
    }
}
void memory_package_t::updatePackageWait (uint32_t stallTime){
    this->status = PACKAGE_STATE_WAIT;
    for (size_t i = 0; i < clients.size(); i++) clients[i]->updatePackageWait (stallTime);
}
void memory_package_t::updatePackageFree (uint32_t stallTime){
    this->status = PACKAGE_STATE_FREE;
    for (size_t i = 0; i < clients.size(); i++) clients[i]->updatePackageFree (stallTime);
}

void memory_package_t::printPackage(){
    ORCS_PRINTF ("Memory Package\nAddress: %lu | Operation: %s | Status: %s | Uop: %lu | ReadyAt: %lu\n", memory_address, get_enum_memory_operation_char (memory_operation), get_enum_package_state_char (status), uop_number, readyAt)
}