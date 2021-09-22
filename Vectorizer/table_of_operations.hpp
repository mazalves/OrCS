// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Armazena operações cujos operandos venham de loads 
// que poderiam ser vetorizados ou outras operações vetorizadas na TO.
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class table_of_operations_t {
    private:
        table_of_operations_entry_t *entries;
        uint32_t max_entries;


        table_of_loads_t *tl;
        table_of_stores_t *ts;
        table_of_vectorizations_t *tv;

       vectorizer_t                 *vectorizer;

    public:
        uint64_t allocated_entries;
        // Deve ser chamado antes de qualquer execução
        void allocate (libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_stores_t *ts, table_of_vectorizations_t *tv, vectorizer_t *vectorizer);

        // Ajusta o LRU da entrada correspondente à instrução em execução
        void new_op (uop_package_t *uop);

        // Chama invalidação para todos os envolvidos
        void start_invalidation (table_of_operations_entry_t *entry);

        // Invalida vetorização ou links caso existam
        void invalidate(table_of_operations_entry_t *entry);

        // Busca na tabela de operações
        table_of_operations_entry_t* find (uop_package_t *uop);

        // Busca na tabela de operações e retorna o id, ou found == false.
        uint32_t find_id (uop_package_t *uop, bool *found);

        // Obtém entrada pelo id
        table_of_operations_entry_t* get_id (uint32_t id);

        // Tenta criar uma nova entrada e retorna seu id, caso contrário informa que é inválida
        uint32_t new_entry_id (uop_package_t *uop, bool *valid);

        // Imprime tabela
        void print();

        // Return id from entry
        uint32_t entry_to_id(table_of_operations_entry_t *entry);

        table_of_operations_t() {
            this->entries = NULL;
            this->max_entries = 0;

            this->tl = NULL;
            this->ts = NULL;
        }

        void check_for_nulls();
};