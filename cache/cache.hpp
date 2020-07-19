#ifndef CACHE_H
#define CACHE_H
using namespace std;

class cache_t {

    private:
        uint64_t cache_hit;
        uint64_t cache_miss;
        uint64_t cache_access;
        uint64_t cache_eviction;
        uint64_t cache_read;
        uint64_t cache_write;
        uint64_t cache_writeback;
        uint64_t change_line;

        void copyLevels(line_t *line, uint32_t idxa, uint32_t idxb, uint32_t processor_id);
        void copyNextLevels(line_t *line, uint32_t idx, uint32_t processor_id);

        uint64_t offset_bits_shift;
        uint64_t index_bits_shift;
        uint64_t tag_bits_shift;

        uint64_t offset_bits_mask;
        uint64_t index_bits_mask;
        uint64_t tag_bits_mask;

        // Get channel to access DATA
        inline uint64_t get_index(uint64_t addr) {
            return (addr & this->index_bits_mask) >> this->index_bits_shift;
        }

        inline uint64_t get_tag(uint64_t addr) {
            return (addr & this->tag_bits_mask) >> this->tag_bits_shift;
        }
        
        uint32_t LINE_SIZE;
        uint32_t PREFETCHER_ACTIVE;
        uint32_t INSTRUCTION_LEVELS;
        uint32_t DATA_LEVELS;
        uint32_t POINTER_LEVELS;
        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;

    public:
        cache_t();
        ~cache_t();

        uint32_t id;
        uint32_t level;
        uint32_t size;
        uint32_t latency;
        uint32_t associativity;
        uint32_t n_sets;
        cacheSet_t *sets;
        uint32_t offset;

        void statistics();
        void allocate(uint32_t NUMBER_OF_PROCESSORS, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS);//allocate data structure
        void writeBack(line_t *line, uint32_t processor_id);       //makes writeback of line
        void returnLine(memory_package_t* request, cache_t *cache);//return line from lower cache level
        void tagIdxSetCalculation(uint64_t address, uint64_t *idx, uint64_t *tag); //calculate index of data, makes tag from address
        uint32_t searchLru(cacheSet_t *set);//searh LRU to substitue
        uint32_t read(uint64_t address, uint32_t &ttc);
        uint32_t write(memory_package_t* request);
        void printTagIdx(uint64_t address);
        line_t *installLine(memory_package_t* request, uint32_t latency, uint64_t &idx, uint64_t &line); //install line of cache |mem_controller -> caches|

        // Getters and setters
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_access)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_eviction)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_read)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_write)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_writeback)
        INSTANTIATE_GET_SET_ADD(uint64_t,change_line)
        
        INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)
        INSTANTIATE_GET_SET_ADD(uint32_t,PREFETCHER_ACTIVE)
        INSTANTIATE_GET_SET_ADD(uint32_t,INSTRUCTION_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,DATA_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,POINTER_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,CACHE_MANAGER_DEBUG)
        INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)
};

#endif // CACHE_H
