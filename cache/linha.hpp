#ifndef LINE_H
#define LINE_H

class line_t {

    public:
        uint64_t tag;
        uint32_t dirty;
        uint64_t lru;
        uint32_t prefetched;
        uint32_t valid;
        uint64_t ready_at;
        directory_line_t *directory_line;
        line_t ***line_ptr_caches;
        line_t ***line_ptr_emc;

        uint32_t NUMBER_OF_PROCESSORS;
        INSTANTIATE_GET_SET_ADD(uint32_t, NUMBER_OF_PROCESSORS)

        line_t() {
            this->clean_line();

            libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
            set_NUMBER_OF_PROCESSORS(cfg_root["PROCESSOR"].getLength());
        }

        ~line_t() {
            libconfig::Setting &cfg_root = orcs_engine.configuration->getConfig();
            set_NUMBER_OF_PROCESSORS(cfg_root["PROCESSOR"].getLength());
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
        }
};

#endif // LINE_H
