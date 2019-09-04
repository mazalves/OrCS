#ifndef CACHE_H
#define CACHE_H

// number of cache levels
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
        //=============
        uint64_t cacheHit;
        uint64_t cacheMiss;
        uint64_t cacheAccess;
        uint64_t cacheRead;
        uint64_t cacheWrite;
        uint64_t cacheWriteBack;
        uint64_t changeLine;

        uint32_t LINE_SIZE;

        uint32_t L1_DATA_SIZE = 32*KILO;
        uint32_t ASSOCIATIVITY_L1D;
        uint32_t LATENCY_L1D;
        uint32_t L1_DATA_SETS;

        uint32_t L1_INST_SIZE = 32*KILO;
        uint32_t ASSOCIATIVITY_L1I;
        uint32_t LATENCY_L1I;
        uint32_t L1_INST_SETS;

        uint32_t L2_SIZE = 256*KILO;
        uint32_t ASSOCIATIVITY_L2D;
        uint32_t LATENCY_L2D;
        uint32_t L2_SETS;
        // ==================== LEVEL 2 =====================
        // ==================== LLC     =====================
        uint32_t LLC_SIZE = 20*MEGA;
        uint32_t ASSOCIATIVITY_LLCD;
        uint32_t LATENCY_LLCD;
        uint32_t LLC_SETS;

        uint32_t PREFETCHER_ACTIVE;

        uint32_t INSTRUCTION_LEVELS;
        uint32_t DATA_LEVELS;
        uint32_t POINTER_LEVELS;
        uint32_t CACHE_MANAGER_DEBUG;
        uint32_t WAIT_CYCLE;

    public:
        cache_t();
        ~cache_t();

        //atributtes
        uint32_t id;    // instruction or data cache
        char16_t *level;
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
        void tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag); //calculate index of data, makes tag from address
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
        
        INSTANTIATE_GET_SET_ADD(uint32_t,LINE_SIZE)

        INSTANTIATE_GET_SET_ADD(uint32_t,PREFETCHER_ACTIVE)

        INSTANTIATE_GET_SET_ADD(uint32_t,INSTRUCTION_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,DATA_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,POINTER_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t,CACHE_MANAGER_DEBUG)
        INSTANTIATE_GET_SET_ADD(uint32_t,WAIT_CYCLE)
};

#endif // CACHE_H
