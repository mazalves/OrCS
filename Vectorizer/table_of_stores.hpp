// +++++++++++++++++++++++++++++++++++++++++++++++++
// Armazena Stores que poderiam ser vetorizados, 
// correspondentes a operações ou loads que poderiam
// ser vetorizados
// +++++++++++++++++++++++++++++++++++++++++++++++++
class table_of_stores_t {
    private:
        table_of_stores_entry_t *entries;
        uint32_t max_entries;

        table_of_loads_t *tl;
        table_of_operations_t *to;
        table_of_vectorizations_t *tv;

       vectorizer_t                 *vectorizer;


    public:

        // Deve ser chamado antes de qualquer execução
        void allocate (libconfig::Setting &vectorizer_configs, table_of_loads_t *tl, table_of_operations_t *to, table_of_vectorizations_t *tv, vectorizer_t *vectorizer);

        // Recebe um store e gerencia sua alocação e verificações de stride
        void new_st (uop_package_t *uop);


        // Adiciona na tabela de loads
        table_of_stores_entry_t* add_ts (uop_package_t *uop);

        // Chama invalidação para todos os envolvidos
        void start_invalidation (table_of_stores_entry_t *entry);

        // Invalida vetorização ou links caso existam
        void invalidate(table_of_stores_entry_t *entry);

        // Busca na tabela de operações
        table_of_stores_entry_t* find (uop_package_t *uop);

        // Busca na tabela de operações e retorna o id, ou found == false.
        uint32_t find_id (uop_package_t *uop, bool *found);

        // Obtém entrada pelo id
        table_of_stores_entry_t *get_id (uint32_t id);

        // Verifica se a entrada pode ser vetorizada e a vetoriza
        void vectorize (table_of_stores_entry_t *entry);

        // Imprime tabela
        void print();

        table_of_stores_t() {
            this->entries = NULL;
            this->max_entries = 0;

            this->tl = NULL;
            this->to = NULL;
        }
};