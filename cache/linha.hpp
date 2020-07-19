#ifndef LINE_H
#define LINE_H

class line_t {

    private:
        uint32_t POINTER_LEVELS;

    public:
        uint64_t tag;
        uint32_t dirty;
        uint64_t lru;
        uint32_t prefetched;
        uint32_t valid;
        uint64_t ready_at;
        line_t ***line_ptr_caches;

        uint32_t NUMBER_OF_PROCESSORS;
        INSTANTIATE_GET_SET_ADD(uint32_t, NUMBER_OF_PROCESSORS)

        line_t() {
            this->clean_line();

            libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
            set_NUMBER_OF_PROCESSORS(cfg_root["PROCESSOR"].getLength());
        }

        ~line_t() {
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
                for (uint32_t j = 0; j < POINTER_LEVELS; j++) {
                    delete line_ptr_caches[i][j];
                }
                delete[] line_ptr_caches[i];
            }
            delete[] line_ptr_caches;
        }

        void allocate(uint32_t POINTER_LEVELS) {
            set_POINTER_LEVELS(POINTER_LEVELS);
            this->line_ptr_caches = new line_t**[NUMBER_OF_PROCESSORS]();
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
                this->line_ptr_caches[i] = new line_t*[POINTER_LEVELS]();
            }
        }

        void clean_line() {
            this->tag = POSITION_FAIL;
            this->dirty = 0;
            this->lru = UINT64_MAX;
            this->prefetched = 0;
            this->valid = 0;
            this->ready_at = 0;

            this->POINTER_LEVELS = 0;
        }

        void print_line(){
            ORCS_PRINTF ("tag: %lu, dirty: %u, lru: %lu, prefetched: %u, valid: %u, ready_at: %lu\n", tag, dirty, lru, prefetched, valid, ready_at)
        }

        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};

#endif // LINE_H
