#include "./../simulator.hpp"
#include <string>

memory_package_t::memory_package_t() {

}

memory_package_t::~memory_package_t(){
    
}

void memory_package_t::updatePackageUntreated (uint32_t stallTime){
    client->updatePackageUntreated (stallTime);
}
void memory_package_t::updatePackageReady (uint32_t stallTime){
    client->updatePackageReady (stallTime);
}
void memory_package_t::updatePackageWait (uint32_t stallTime){
    client->updatePackageWait (stallTime);
}
void memory_package_t::updatePackageFree (uint32_t stallTime){
    client->updatePackageFree (stallTime);
}