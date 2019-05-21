#ifndef CACHE_H
#define CACHE_H

// number of cache levels
#define INSTRUCTION_LEVELS 1
#define DATA_LEVELS 3

class cache_t {
     private:
        // Statistics related
        uint64_t cache_hit;
        uint64_t cache_miss;
        uint64_t cache_access;
        uint64_t cache_read;
        uint64_t cache_write;
        uint64_t cache_writeback;
        uint64_t change_line;

        void copyLevels(line_t *line, uint32_t idxa, uint32_t idxb);
        void copyNextLevels(line_t *line, uint32_t idx);

    public:
        cache_t();
        ~cache_t();

        //atributtes
        uint32_t id;    // instruction or data cache
        uint32_t level;
        uint32_t size;
        uint32_t latency;
        uint32_t associativity;
        uint32_t n_sets;
        cacheSet_t *sets;
        uint32_t offset;

        void statistics();
        // void cacheStatsAllocation(uint32_t cache_level, uint32_t cache_sets, uint32_t cache_associativity);
        void allocate(cacheId_t cache_type, uint32_t cache_level, uint32_t cache_size, uint32_t cache_associativity, uint32_t cache_latency);//allocate data structure
        void writeBack(line_t *line); //makes writeback of line
        void returnLine(uint64_t address, cache_t *cache);//return line from lower cache level
        void tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint32_t *tag); //calculate index of data, makes tag from address
        uint32_t searchLru(cacheSet_t *set);//searh LRU to substitue
        uint32_t read(uint64_t address, uint32_t &ttc);
        uint32_t write(uint64_t address);
        line_t* installLine(uint64_t address, uint32_t latency);//install line of cache |mem_controller -> caches|

        // Getters and setters
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_hit)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_miss)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_access)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_read)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_write)
        INSTANTIATE_GET_SET_ADD(uint64_t,cache_writeback)
        INSTANTIATE_GET_SET_ADD(uint64_t,change_line)
};

#endif // CACHE_H
