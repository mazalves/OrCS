// +++++++++++++++++++++++++++++++++
// Rastreia os loads para descobrir 
// quais poderiam ser vetorizados
// +++++++++++++++++++++++++++++++++
class table_of_loads_t {
    private:
        table_of_loads_entry_t *entries;
        uint32_t num_entries;
        uint32_t max_entries;

        table_of_operations_t *to;
        table_of_stores_t *ts;

       vectorizer_t                 *vectorizer;
    public:
        table_of_vectorizations_t *tv;
        
    public:
        // Deve ser chamado antes de qualquer execução
        void allocate (libconfig::Setting &vectorizer_configs, table_of_operations_t *to, table_of_stores_t *ts, table_of_vectorizations_t *tv, vectorizer_t *vectorizer);

        // Recebe um load e gerencia sua alocação e verificações de stride
        table_of_loads_entry_t* new_ld (uop_package_t *uop);

        // Adiciona na tabela de loads
        table_of_loads_entry_t* add_tl (uop_package_t *uop);

        // Busca na tabela de loads
        table_of_loads_entry_t* find (uop_package_t *uop);

        // Retorna entrada pelo id
        table_of_loads_entry_t *get_id(uint32_t id);

        // Chama invalidação para todos os envolvidos
        void start_invalidation (table_of_loads_entry_t *entry);

        // Invalida vetorização ou links caso existam
        void invalidate(table_of_loads_entry_t *entry);

        // Imprime tabela
        void print();

        // Return id from entry
        uint32_t entry_to_id(table_of_loads_entry_t *entry);

        table_of_loads_t() {
            this->entries = NULL;
            this->num_entries = 0;
            this->max_entries = 0;

            this->to = NULL;
            this->ts = NULL;
        }
};