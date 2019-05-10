#ifndef CACHE_H
#define CACHE_H


class cache_t
{

     private:
        //=============
        // Statistics related
        //=============
        uint64_t cacheHit;
        uint64_t cacheMiss;
        uint64_t cacheAccess;
        uint64_t cacheRead;
        uint64_t cacheWrite;
        uint64_t cacheWriteBack;
        uint64_t changeLine;
    public:
        cache_t();
        ~cache_t();
        //atributtes
        uint32_t id;
        cacheLevel_t level;
        uint32_t nSets;
        uint32_t nLines;
        cacheSet_t *sets;
        uint32_t shiftData;
        //====================
        // Debug functions - Utils
        //====================
        inline void printLine(linha_t *linha);
        inline void printCacheConfiguration();
        // ============================================================================
        // Functions with void return
        // ============================================================================
        void statistics();
        void allocate(cacheLevel_t level);//allocate data structure
        void writeBack(linha_t *line); //makes writeback of line
        void returnLine(uint64_t address,cache_t *cache);//return line from lower cache level
        // ============================================================================
        // Functions with uint return
        // ============================================================================
        uint32_t idxSetCalculation(uint64_t address);//calculate index of data
        uint64_t tagSetCalculation(uint64_t address);//makes tag from address
        uint32_t searchLru(cacheSet_t *set);//searh LRU to substitue
        linha_t* installLine(uint64_t address,uint64_t latency);//install line of cache |mem_controller -> caches|
        uint32_t read(uint64_t address,uint64_t &ttc);
        uint32_t write(uint64_t address);
        //getters setters
        INSTANTIATE_GET_SET_ADD(uint64_t,cacheHit)
        INSTANTIATE_GET_SET_ADD(uint64_t,cacheMiss)
        INSTANTIATE_GET_SET_ADD(uint64_t,cacheAccess)
        INSTANTIATE_GET_SET_ADD(uint64_t,cacheRead)
        INSTANTIATE_GET_SET_ADD(uint64_t,cacheWrite)
        INSTANTIATE_GET_SET_ADD(uint64_t,cacheWriteBack)
        INSTANTIATE_GET_SET_ADD(uint64_t,changeLine)
        // ============================================================================
        // Functions for ORACLE
        // ============================================================================
        uint32_t read_oracle(uint64_t address);
};

#endif // CACHE_H
