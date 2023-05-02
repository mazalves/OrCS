#include "./../simulator.hpp"

void vima_prefetcher_t::make_prefetch(conversion_status_t *prev_conversion)
{
    conversion_status_t base = (this->prefetches.size > 0) ?
                                *this->prefetches.back()
                                : *prev_conversion;

    while(prefetches.size < PREFETCH_SIZE) {
        
        // Create sequential conversion data
        this->shift_sequential_conversion(&base);
#if VIMA_CONVERSION_DEBUG == 1
        printf("//****************************\n");
        printf("Prefetching VIMA instruction... [Conversion ID: %lu]\n", base.unique_conversion_id);
        printf("//****************************\n");
        printf("Base address[0]: %lu\n",  base.base_mem_addr[0]);
        printf("Base address[1]: %lu\n",  base.base_mem_addr[1]);
        printf("Base address[3]: %lu\n",  base.base_mem_addr[3]);
#endif
        // Create a new prefetch
        prefetches.push_back(base);

        // Create correspondent VIMA
        orcs_engine.processor->vima_converter.generate_VIMA_instruction(&base);
    }
}

conversion_status_t* vima_prefetcher_t::get_prefetch() {
    if (this->prefetches.size == 0) return NULL;
    return this->prefetches.front();
}

void vima_prefetcher_t::pop_prefetch() {
    assert (this->prefetches.size != 0);
    this->prefetches.pop_front();
}

void vima_prefetcher_t::vima_execution_completed(memory_package_t *vima_package, uint64_t readyAt) {
    // ***************
    // Find conversion
    // ***************
    int32_t conversion_index = -1;
    for (uint32_t i=0; i < this->prefetches.get_size(); ++i) {
        if (this->prefetches[i].unique_conversion_id == vima_package->unique_conversion_id) {
            conversion_index = i;
            break;
        }
    }

    if (conversion_index >= 0) {
    #if VIMA_CONVERSION_DEBUG == 1
        printf("******************************************************\n");
        printf("VIMA requirements achieved! (Prefetched) [Conversion ID %lu] in %lu\n", vima_package->unique_conversion_id, readyAt);
        printf("******************************************************\n");
    #endif
        this->prefetches[conversion_index].VIMA_requirements_meet = true;
        this->prefetches[conversion_index].VIMA_requirements_meet_readyAt = readyAt;
    }
}

void vima_prefetcher_t::shift_sequential_conversion(conversion_status_t *status) {
    status->unique_conversion_id++;
    status->VIMA_requirements_meet = false;
    status->VIMA_requirements_meet_readyAt = 0;


    // (VIMA_SIZE/status->mem_size)
    status->base_mem_addr[0] = status->base_mem_addr[0] + (status->mem_size * (orcs_engine.processor->vima_converter.VIMA_SIZE/status->mem_size));
    status->base_mem_addr[1] = (status->is_mov) ? 0x0 : status->base_mem_addr[1] + (status->mem_size * (orcs_engine.processor->vima_converter.VIMA_SIZE/status->mem_size));
    status->base_mem_addr[3] = status->base_mem_addr[3] + (status->mem_size * (orcs_engine.processor->vima_converter.VIMA_SIZE/status->mem_size));

}