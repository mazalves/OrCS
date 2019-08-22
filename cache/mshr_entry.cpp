#include "./../simulator.hpp"

mshr_entry_t::mshr_entry_t() {

}

mshr_entry_t::~mshr_entry_t(){

}

bool mshr_entry_t::contains (memory_order_buffer_line_t* mob_line){
    if(std::find(requests.begin(), requests.end(), mob_line) != requests.end()) return true;
    return false;
}

void mshr_entry_t::remove (memory_order_buffer_line_t* mob_line){
    requests.erase(std::remove(requests.begin(), requests.end(), mob_line));
}