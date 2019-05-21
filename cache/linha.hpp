#ifndef LINE_H
#define LINE_H

// number of cache levels
#define INSTRUCTION_LEVELS 1
#define DATA_LEVELS 3

#define POINTER_LEVELS ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS)

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

        // Constructor
        line_t() {
            this->clean_line();
        }

        // Desctructor
        ~line_t() {
            // deleting pointes
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
                for (uint32_t j = 0; j < POINTER_LEVELS; j++) {
                    if (this->line_ptr_caches[i][j] != NULL) {
                        delete &line_ptr_caches[i][j];
                    }
                }
            }
            // Nulling pointers
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
                for (uint32_t j = 0; j < POINTER_LEVELS; j++) {
                    this->line_ptr_caches[i][j] = NULL;
                }
            }
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
