#include "./../simulator.hpp"

mshr_entry_t::mshr_entry_t() {

}

mshr_entry_t::~mshr_entry_t(){

}

void mshr_entry_t::updateRequests(){
    uint64_t oldest = requests[0]->readyToGo;
    for (std::size_t j = 0; j < requests.size(); j++){
        requests[j]->updatePackageReady (latency - (oldest - requests[j]->readyToGo));
    }
}