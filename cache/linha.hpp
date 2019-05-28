#ifndef LINE_H
#define LINE_H

// cache sizes for each level
const uint32_t ICACHE_SIZE[1] = {32768};
const uint32_t DCACHE_SIZE[3] = {32768, 262144, 4194304};

// cache associativity for each level
const uint32_t ICACHE_ASSOCIATIVITY[1] = {8};
const uint32_t DCACHE_ASSOCIATIVITY[3] = {8, 8, 8};

// cache latency for each level
const uint32_t ICACHE_LATENCY[1] = {3};
const uint32_t DCACHE_LATENCY[3] = {3, 6, 9};

// number of caches in each level
const uint32_t ICACHE_AMOUNT[1] = {1};
const uint32_t DCACHE_AMOUNT[3] = {1, 1, 1};

// EMC pointers removed!
class line_t {
    public:
        uint64_t tag;
        uint32_t dirty;
        uint64_t lru;
        uint32_t prefetched;
        uint32_t valid;
        uint64_t ready_at;
        line_t ***line_ptr_caches;
        line_t ***line_ptr_emc;

        uint32_t INSTRUCTION_LEVELS;
        uint32_t DATA_LEVELS;
        uint32_t POINTER_LEVELS;

        // Constructor
        line_t() {
            this->clean_line();
            INSTRUCTION_LEVELS = orcs_engine.configuration->getSetting ("INSTRUCTION_LEVELS");
	        DATA_LEVELS = orcs_engine.configuration->getSetting ("DATA_LEVELS");
            POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
        }

        // Desctructor
        ~line_t() {
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) delete[] line_ptr_caches[i];
            delete[] line_ptr_caches;
        }

        void allocate() {
            this->line_ptr_caches = new line_t**[NUMBER_OF_PROCESSORS];
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
                this->line_ptr_caches[i] = new line_t*[POINTER_LEVELS];
            }
        }

        void clean_line() {
            this->tag = 0;
            this->dirty = 0;
            this->lru = 0;
            this->prefetched = 0;
            this->valid = 0;
            this->ready_at = 0;
            // this->line_ptr_caches = NULL;
            // for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
            //     for (uint32_t j = 0; j < POINTER_LEVELS; j++) {
            //         this->line_ptr_caches[i][j] = NULL;
            //     }
            // }
        }
};

#endif // LINE_H
