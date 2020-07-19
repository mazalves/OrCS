#include "../simulator.hpp"

prefetcher_t::prefetcher_t(){
    this->prefetcher = NULL;
    //ctor
}

prefetcher_t::~prefetcher_t()
{
    if(this->prefetcher!=NULL) delete &this->prefetcher;
    //dtor
}
void prefetcher_t::allocate(uint32_t NUMBER_OF_PROCESSORS){
    // libconfig::Setting* cfg_root = orcs_engine.configuration->getConfig();
    // set_PARALLEL_PREFETCH (cfg_root[0]["NUMBER_OF_PROCESSORS"]);

    this->set_latePrefetches(0);
    this->set_usefulPrefetches(0);
    this->set_latePrefetches(0);
    this->set_totalCycleLate(0);
    //#if STRIDE
        this->prefetcher = new stride_prefetcher_t();
        this->prefetcher->allocate(NUMBER_OF_PROCESSORS);
    //#endif  
    // List of cycle completation prefetchs. Allows control issue prefetchers
    this->prefetch_waiting_complete.reserve(NUMBER_OF_PROCESSORS);
}
// ================================================================
// @mobLine - references to index the prefetch
// @*cache - cache to be instaled line prefetched
// ================================================================
void prefetcher_t::prefecht(memory_order_buffer_line_t *mob_line, cache_t *cache){
    std::ignore = mob_line;
    std::ignore = cache;
    /*uint32_t idx_padding, line_padding;
    uint64_t cycle = orcs_engine.get_global_cycle();
    if((this->prefetch_waiting_complete.front() <= cycle) &&
        (this->prefetch_waiting_complete.size()!=0)){
        this->prefetch_waiting_complete.erase(this->prefetch_waiting_complete.begin());
    }
    int64_t newAddress = this->prefetcher->verify(mob_line->opcode_address,mob_line->memory_address);
    // sacrifice = 0 is a request to the function read in cache
    uint32_t sacrifice = 0;
    if(this->prefetch_waiting_complete.size()>= PARALLEL_PREFETCH){
        return;
    }
    //UPDATE IF GOING TO USE
    if(newAddress != POSITION_FAIL) {
        uint32_t status = cache->read(newAddress, sacrifice);
        if(status == MISS){
            this->add_totalPrefetched();
            uint64_t latency_prefetch = orcs_engine.memory_controller->requestDRAM(NULL, newAddress);
            orcs_engine.memory_controller->add_requests_prefetcher();
            line_t *linha = cache->installLine(newAddress, latency_prefetch, NULL, idx_padding, line_padding);
            linha->prefetched=1;
            this->prefetch_waiting_complete.push_back(cycle+latency_prefetch);
        }
    }*/
}
void prefetcher_t::statistics(){
    bool close = false;
    FILE *output = stdout;
	if(orcs_engine.output_file_name != NULL){
		output = fopen(orcs_engine.output_file_name,"a+");
        close=true;
    }
	if (output != NULL){
            utils_t::largeSeparator(output);
            fprintf(output,"##############  PREFETCHER ##################\n");
            fprintf(output,"Total Prefetches: %u\n", this->get_totalPrefetched());
            fprintf(output,"Useful Prefetches: %u\n", this->get_usefulPrefetches());
            fprintf(output,"Late Prefetches: %u\n",this->get_latePrefetches());
            fprintf(output,"MediaAtraso: %.4f\n",(float)this->get_totalCycleLate()/(float)this->get_latePrefetches());
            utils_t::largeSeparator(output);
        }
	if(close) fclose(output);
}
