#ifndef __PIN__
#include "../simulator.hpp"
#else
#include "memory_request_client.hpp"
#endif

memory_request_client_t::memory_request_client_t(){
    
}

memory_request_client_t::~memory_request_client_t(){
    
}

#ifndef __PIN__
void memory_request_client_t::updatePackageUntreated (uint32_t stallTime){
    this->status = PACKAGE_STATE_UNTREATED;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}

void memory_request_client_t::updatePackageReady (uint32_t stallTime){
    this->status = PACKAGE_STATE_READY;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}

void memory_request_client_t::updatePackageWait (uint32_t stallTime){
    this->status = PACKAGE_STATE_WAIT;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}

void memory_request_client_t::updatePackageFree (uint32_t stallTime){
    this->status = PACKAGE_STATE_FREE;
    this->readyAt = orcs_engine.get_global_cycle()+stallTime;
}
#endif
