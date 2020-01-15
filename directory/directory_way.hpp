class directory_way_t {

    public:
        level_way_t **inst_cache;
        level_way_t **data_cache;
        uint64_t tag;

        directory_way_t() {
            this->inst_cache = NULL;
            this->data_cache = NULL;
            this->tag = 0;
        }

        ~directory_way_t()=default;

        void allocate(uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS, uint32_t *ICACHE_AMOUNT, uint32_t *DCACHE_AMOUNT) {
            this->inst_cache = new level_way_t *[INSTRUCTION_LEVELS];
            this->data_cache = new level_way_t *[DATA_LEVELS];
            for (uint32_t i = 0; i < INSTRUCTION_LEVELS; i++) {
                this->inst_cache[i] = new level_way_t[ICACHE_AMOUNT[i]];
                // this->inst_cache[i]->allocate();
            }
            for (uint32_t i = 0; i < DATA_LEVELS; i++) {
                this->data_cache[i] = new level_way_t[DCACHE_AMOUNT[i]];
                // this->data_cache[i]->allocate();
            }
        }

};