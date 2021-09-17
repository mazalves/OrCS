// Tabela para monitorar instruções potêncialmente vetorizáveis no Rename, descobrindo seu próximo endereço acessado.

class table_of_pre_vectorization_t {
    private:
        table_of_pre_vectorization_entry_t *entries;
        uint32_t occupied_entries;
        uint32_t max_entries;


        // Referências a outras tabelas
        table_of_loads_t          *tl;
        table_of_operations_t     *to;
        table_of_stores_t         *ts;

    public:
        table_of_pre_vectorization_t() {
            this->entries = NULL;
            this->max_entries = 0;
            this->occupied_entries = 0;
        }

        void allocate(libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_operations_t *to, table_of_stores_t *ts);

        bool insert (uint64_t addr, uint8_t uop_id, table_of_vectorizations_entry_t *tv_entry);

        bool remove (uint64_t addr, uint8_t uop_id);

        inline bool has_n_vacancies(uint32_t n) {
            return ((this->max_entries - this->occupied_entries) >= n);
        }

        table_of_vectorizations_entry_t* get_tv_entry (uint64_t addr, uint8_t uop_id);

        inline bool is_empty () {
            return (this->occupied_entries == 0);
        }

        void remove_vectorization (table_of_vectorizations_entry_t *entry);



};