class vectorizer_t {
    private:
        table_of_loads_t          *tl;
        table_of_operations_t     *to;
        table_of_stores_t         *ts;
        table_of_vectorizations_t *tv;

        table_of_pre_vectorization_t *tpv;
        bool enabled;
        
    public:
            registers_tracker_t       *rt;
        // Vetorizações esperando o Rename
        circular_buffer_t<uop_package_t> vectorizations_to_execute;
        table_of_ignored_t *ti;  // Instruções que não devem ser executadas
                                 // apenas armazenadas no ROB até 
                                 // confirmação ou reexecução.
        circular_buffer_t<uop_package_t> instructions_to_reexecute; // Quando uma vetorização é invalidada
                                                                    // suas instruções ignoradas do ROB devem ser reexecutadas,
                                                                    // Então as presentes no ROB são ignoradas e novas correspondentes
                                                                    // são adicionadas a esse buffer para entrar no pipeline pelo rename.
    public:

        void allocate(functional_unit_t *mem_op_fu, libconfig::Setting &cfg_root);

        void new_committed_uop (uop_package_t *uop);

        void new_renamed_uop (uop_package_t *uop);

        uint32_t request_invalidation_from_TM(transactional_operation_t *operation);

        // Reexecuta instruções da vetorização de maneira escalar
        // Coloca instruções correspondentes às ignoradas no 'instructions_to_reexecute'
        // Marca a entrada na tv para descartar as instruções ignoradas durante o commit.
        void flush_vectorization(table_of_vectorizations_entry_t *tv_entry);

        // Toda uop de load/store que é calculada no estágio de execute chama essa
        // função para invalidar conflitos com vetorizações
        void new_AGU_calculation(uop_package_t *uop);
        

        void statistics(FILE *output);
};