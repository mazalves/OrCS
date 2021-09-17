class table_of_operations_entry_t {
    private:
        uint64_t pc;
        uint8_t uop_id;

    public:
        char operation[MAX_ASSEMBLY_SIZE];
        //int32_t source_operand_1;
        //int32_t source_operand_2;
        table_of_loads_entry_t *tl_entries[2];
        table_of_stores_entry_t *ts_entry;
        table_of_vectorizations_entry_t *tv_entry;

        uint64_t lru;
        bool free;

        table_of_operations_entry_t () {
            pc = 0;
            uop_id = 0;
            //source_operand_1 = -1;
            //source_operand_2 = -1;
            tl_entries[0] = NULL;
            tl_entries[1] = NULL;
            ts_entry = NULL;
            tv_entry = NULL;

            lru = 0;
            free = true;
        }

        void clean() {
            this->pc = 0;
            this->uop_id = 0;
            //source_operand_1 = -1;
            //source_operand_2 = -1;
            this->tl_entries[0] = NULL;
            this->tl_entries[1] = NULL;
            this->ts_entry = NULL;
            this->tv_entry = NULL;

            this->lru = 0;
            this->free = true;
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

        inline void fill_entry( uint64_t pc, uint8_t uop_id,
                                char operation[],
                                table_of_loads_entry_t *tl_entries_1,
                                table_of_loads_entry_t *tl_entries_2,
                                table_of_stores_entry_t *ts_entry,
                                table_of_vectorizations_entry_t *tv_entry,
                                uint64_t lru) {
            this->pc = pc;
            this->uop_id = uop_id;;
            strcpy(this->operation, operation);
            this->tl_entries[0] = tl_entries_1;
            this->tl_entries[1] = tl_entries_2;
            this->ts_entry = ts_entry;
            this->tv_entry = tv_entry;
            this->lru = lru;
            this->free = false;
        }

        inline uint64_t get_pc() {
            return this->pc;
        }

        inline uint64_t get_uop_id() {
            return this->uop_id;
        }



        void print () {
            printf("  %d %lu Operation %s tl[0]: %p tl[1]: %p ts: %p tv: %p lru: %lu\n",
            this->uop_id,
            this->pc,
            this->operation, (void *)this->tl_entries[0], (void *)this->tl_entries[1], (void *)this->ts_entry, (void *)this->tv_entry, this->lru);
        }
};