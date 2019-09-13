#ifndef LINE_H
#define LINE_H

// cache sizes for each level
// const uint32_t ICACHE_SIZE[1] = {32768};
// const uint32_t DCACHE_SIZE[3] = {32768, 262144, 4194304};

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

        uint32_t NUMBER_OF_PROCESSORS;
        // uint32_t INSTRUCTION_LEVELS;
        // uint32_t DATA_LEVELS;
        // uint32_t POINTER_LEVELS;

        // INSTANTIATE_GET_SET_ADD (uint32_t, INSTRUCTION_LEVELS)
        // INSTANTIATE_GET_SET_ADD(uint32_t, DATA_LEVELS)
        INSTANTIATE_GET_SET_ADD(uint32_t, NUMBER_OF_PROCESSORS)

        // Constructor
        line_t() {
            this->clean_line();
            // libconfig::Config cfg;
            // cfg.readFile(orcs_engine.config_file);

            // libconfig::Setting &cfg_root = cfg.getRoot();
            libconfig::Setting *cfg_root = orcs_engine.configuration->getConfig();
            // libconfig::Setting *cfg_root = orcs_engine.configuration->getConfig();

            set_NUMBER_OF_PROCESSORS(cfg_root[0]["PROCESSOR"]["NUMBER_OF_PROCESSORS"]);
            // printf("linha.hpp - NUMBER_OF_PROCESSORS: %u\n", NUMBER_OF_PROCESSORS);

            // libconfig::Setting &cfg_cache = cfg_root[0]["CACHE_MEMORY"]["CONFIG"];
            // set_INSTRUCTION_LEVELS(1);
            // set_DATA_LEVELS(3);
            // POINTER_LEVELS = ((INSTRUCTION_LEVELS > DATA_LEVELS) ? INSTRUCTION_LEVELS : DATA_LEVELS);
        }

        // Desctructor
        ~line_t() {
            libconfig::Setting *cfg_root = orcs_engine.configuration->getConfig();
            set_NUMBER_OF_PROCESSORS(cfg_root[0]["PROCESSOR"]["NUMBER_OF_PROCESSORS"]);
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) delete[] line_ptr_caches[i];
            delete[] line_ptr_caches;
        }

        void allocate(uint32_t POINTER_LEVELS) {
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
