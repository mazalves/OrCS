// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Operações vetorizadas para instruções VIMA são armazenadas aqui
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class table_of_vectorizations_t {
    private:
        table_of_vectorizations_entry_t *entries;
        uint32_t num_entries;
        uint32_t max_entries;
        uint32_t vectorization_size;

        // Demais tabelas
        table_of_loads_t *tl;
        table_of_operations_t *to;
        table_of_stores_t *ts;
        table_of_ignored_t *ti;
        table_of_pre_vectorization_t *tpv;
        


        // VIMA configurations
        uint32_t mem_operation_latency;
        uint32_t mem_operation_wait_next;
        functional_unit_t *mem_operation_fu;


public:
    vectorizer_t *vectorizer;

private:
        void invalidate(table_of_vectorizations_entry_t *entry);

    public:
        // Deve ser chamado antes de qualquer execução
        void allocate (libconfig::Setting &vectorizer_configs, 
                        table_of_loads_t *tl, table_of_operations_t *to, table_of_stores_t *ts,
                        table_of_ignored_t *ti, table_of_pre_vectorization_t *tpv, vectorizer_t *vectorizer, 
                        uint32_t latency_mem_operation, uint32_t latency_mem_wait, functional_unit_t *mem_op_fu);

        table_of_vectorizations_entry_t *allocate_entry();
        
        table_of_vectorizations_entry_t *get_id(uint32_t id);

        instruction_operation_t define_vima_operation(table_of_stores_entry_t *ts_entry);

        table_of_vectorizations_entry_t *new_vectorization (table_of_stores_entry_t *ts_entry);

        /* True -> Sucesso; False -> Não conseguiu alocar */
        bool new_pre_vectorization (table_of_stores_entry_t *ts_entry);

        void start_invalidation (table_of_vectorizations_entry_t *tv_entry);

        // Return id from entry
        uint32_t entry_to_id(table_of_vectorizations_entry_t *entry);

        // Registrador de vetorização sobrescrito
        void register_overwritten (table_of_vectorizations_entry_t *entry);

        // Cria uma instrução VIMA correspondente e a insere no pipeline
        void generate_VIMA_instruction(table_of_vectorizations_entry_t *entry);

        // Verifica o stride de uma instrução waiting após sua passagem pela AGU
        void verify_stride(uop_package_t *uop);

        // Desvincula entrada de todas as demais tabelas
        // Utilizado para quando só falta esperar as instruções em waiting
        // e não conseguiu fazer a proxima vetorização.
        // (A entrada na tv deve continuar intacta, apenas sem vínculo com a ts)
        // Também limpa as entradas nas outras tabelas, as invalidando
        void unbind (table_of_vectorizations_entry_t *entry);


        // Cada load executado pode invalidar uma vetorização, caso carregue dados do seu destino
        // Cada store executado pode invalidar uma vetorização, caso carregue dados do sua origem
        // Isso pode acontecer entre duas vetorizações (instrução em waiting) ou uma instrução comum e uma vetorização
        // Isso porque não sabemos se devemos fornecer os dados novos ou antigos da linha
        void new_AGU_calculation (uop_package_t *uop);


        void statistics(FILE *);
};

