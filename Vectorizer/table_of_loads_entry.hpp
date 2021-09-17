class table_of_loads_entry_t {
    private:
        // Dados da linha
        uint64_t pc;
        uint8_t uop_id; // Se é o primeiro ou segundo uop load

    public:
        uint64_t last_address;
        int32_t stride;
        uint8_t confidence_counter;
        uint8_t ld_confidence;

        // Ponteiros entre estruturas
        bool is_mov;
        uint32_t to_ts_entry;
        bool linked_to_ts;

        // Se está vetorizado
        table_of_vectorizations_entry_t *tv_entry;

        // Alocação
        uint64_t lru;
        bool free;

        // Indica que é vetorizável (confidence no máximo e stride com boa distância)
        // [dados contíguos e com tamanho 32 ou 64 bits]
        bool vectorizable;


        ////////////////////////
        // Methods and functions
        ////////////////////////
        table_of_loads_entry_t() {
            
            this->pc = 0;
            this->uop_id = 0;
            this->last_address = 0;
            this->stride = 0;
            this->confidence_counter = 0;
            this->ld_confidence = 0;
            this->is_mov = false;
            this->to_ts_entry = 0;
            this->linked_to_ts = false;
            this->tv_entry = NULL;
            this->lru = 0;
            this->free = true;
            this->vectorizable = false;
        }

        inline void clean() {
            
            this->pc = 0;
            this->uop_id = 0;
            this->last_address = 0;
            this->stride = 0;
            this->confidence_counter = 0;

            this->is_mov = false;
            this->to_ts_entry = 0;
            this->linked_to_ts = false;

            this->tv_entry = NULL;

            this->lru = 0;
            this->free = true;
            
            this->vectorizable = false;
        }


        void allocate(libconfig::Setting &vectorizer_configs) {
            assert((int32_t)vectorizer_configs["LD_CONFIDENCE"] < 256); // uint8_t
            this->ld_confidence = (int32_t)vectorizer_configs["LD_CONFIDENCE"];
        }

        inline void inc_confidence_counter(uint64_t lru) {
            if (this->confidence_counter < this->ld_confidence) {
                this->confidence_counter++;
            }
            this->lru = lru;
        }

        inline void reset_confidence_counter(uint64_t lru) {
            this->confidence_counter = 0;
            this->lru = lru;

        }


        inline void check_vectorizable(uop_package_t *uop) {
            //printf(" LOAD Vectorizable => Confidence %d/%d; mem_size: %u/4 || 8; 0 < stride <= mem_size: 0 < %d <= %u\n",
            //            this->confidence_counter, this->ld_confidence,
            //            uop->memory_size[0], this->stride, (int32_t)uop->memory_size[0]);
            if ((this->confidence_counter == this->ld_confidence) &&
                (uop->memory_size[0] == 4 || uop->memory_size[0] == 8) && //32 ou 64 bits
                (this->stride > 0) &&
                (this->stride <= (int32_t)uop->memory_size[0]) // Contíguos
                ) {
                    this->vectorizable = true;
                } else {
                    this->vectorizable = false;
                }
        }

        inline void fill_entry( uint64_t pc, uint8_t uop_id,
                                uint64_t last_address,
                                int32_t stride,
                                uint8_t confidence_counter,
                                bool is_mov,
                                int64_t to_ts_entry,
                                bool linked_to_ts,
                                table_of_vectorizations_entry_t *tv_entry,
                                uint64_t lru,
                                bool free,
                                bool vectorizable) {
            this->pc = pc;
            this->uop_id = uop_id;
            this->last_address = last_address;
            this->stride = stride;
            this->confidence_counter = confidence_counter;
            this->is_mov = is_mov;
            this->to_ts_entry = to_ts_entry;
            this->linked_to_ts = linked_to_ts;
            this->tv_entry = tv_entry;
            this->lru = lru;
            this->free = free;
            this->vectorizable = vectorizable;

        }

        inline bool is_entry(uop_package_t *uop) {
            if ((this->pc == uop->opcode_address) && 
                (this->uop_id == uop->uop_id) &&
                (this->free == false)) 
                {
                    return true;
                }
            return false;
        }

        inline uint64_t get_pc() {
            return this->pc;
        }

        inline uint64_t get_uop_id() {
            return this->uop_id;
        }

        void print () {
            printf("  %d %lu (%lu) Stride: %d Confidence: %d/%d is_mov: %d to_ts_entry: %d (V: %d) tv_entry: %p lru: %lu F: %s Vec: %s\n",
            this->uop_id,
            this->pc,
            this->last_address,
            this->stride,
            this->confidence_counter,
            this->ld_confidence,
            this->is_mov,
            this->to_ts_entry,
            this->linked_to_ts,
            (void *)this->tv_entry,
            this->lru,
            this->free ? "true" : "false",
            this->vectorizable ? "true" : "false");
        }
};