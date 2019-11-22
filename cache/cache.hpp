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

        // void copyLevels(line_t *line, uint32_t idxa, uint32_t idxb);
        // void copyNextLevels(line_t *line, uint32_t idx);

        // uint64_t cacheHit;
        // uint64_t cacheMiss;
        // uint64_t cacheAccess;
        // uint64_t cacheRead;
        // uint64_t cacheWrite;
        // uint64_t cacheWriteBack;
        // uint64_t changeLine;

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
        void allocate(uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS); //allocate data structure
        inline void eviction(directory_t directory, uint32_t idx, int32_t line);       //makes writeback of line
        inline uint32_t is_LLC();
         void returnLine(uint64_t address, cache_t *cache, directory_t directory, cacheId_t cache_type); //return line from lower cache level
        inline void writeback(uint32_t c_idx, int32_t c_line, uint32_t d_idx, int32_t d_line, directory_t directory);
        void tagIdxSetCalculation(uint64_t address, uint32_t *idx, uint64_t *tag, uint32_t n_sets, uint32_t offset); //calculate index of data, makes tag from address
        int32_t searchLru(uint32_t idx);//searh LRU to substitue
        inline int32_t getCacheLine(uint32_t idx, uint64_t tag);
        inline int32_t getInvalidLine(uint32_t idx);
        inline int32_t getDirectoryLine(directory_t directory, uint32_t idx, uint64_t tag);
        uint32_t read(uint64_t address, uint32_t &ttc);
        uint32_t write(uint64_t address, directory_t directory);
        line_t *installLine(uint64_t address, uint32_t latency, directory_t directory, uint32_t &idx, int32_t &line, uint64_t &tag); //install line of cache |mem_controller -> caches|

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
