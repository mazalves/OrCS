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

        void allocate(uint32_t NUMBER_OF_PROCESSORS, uint32_t INSTRUCTION_LEVELS, uint32_t DATA_LEVELS) {
            this->inst_cache = new level_way_t *[NUMBER_OF_PROCESSORS];
            this->data_cache = new level_way_t *[NUMBER_OF_PROCESSORS];
            for (uint32_t i = 0; i < NUMBER_OF_PROCESSORS; i++) {
                this->inst_cache[i] = new level_way_t[INSTRUCTION_LEVELS];
                this->data_cache[i] = new level_way_t[DATA_LEVELS];
            }
        }

};