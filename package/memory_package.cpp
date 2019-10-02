#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {
    this->latency = 0;
}

memory_package_t::~memory_package_t(){
    
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    for (size_t i = 0; i < clients.size(); i++) clients[i]->updatePackageUntreated (stallTime);
}
void memory_package_t::updatePackageReady (uint32_t stallTime){
    this->status = PACKAGE_STATE_READY;
    for (size_t i = 0; i < clients.size(); i++) {
        clients[i]->updatePackageReady (stallTime);
    }
}
void memory_package_t::updatePackageWait (uint32_t stallTime){
    for (size_t i = 0; i < clients.size(); i++) clients[i]->updatePackageWait (stallTime);
}
void memory_package_t::updatePackageFree (uint32_t stallTime){
    for (size_t i = 0; i < clients.size(); i++) clients[i]->updatePackageFree (stallTime);
}