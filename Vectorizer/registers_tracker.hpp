class registers_tracker_t {
    private:
        table_of_loads_t            *tl;
        table_of_operations_t       *to;
        table_of_stores_t           *ts;
        table_of_vectorizations_t   *tv;

        // ###############################################################################################
        // Rename
        // ###############################################################################################
        table_of_ignored_t           *ti;
        table_of_pre_vectorization_t *tpv;
        vectorizer_t                 *vectorizer;

        /* Em uma invalidação de uma instrução vetorizada os ponteiros para trás dessa entrada na to ou ts
           são usados para idendificar as instruções anteriores que precisam ser reexecutadas */

    public:
        registers_tracker_entry_t *entries;
        uint32_t num_entries;

        registers_tracker_t () {
            this->entries = NULL;
            this->num_entries = 0;
        }

        void allocate (table_of_loads_t *tl, 
                       table_of_operations_t *to, 
                       table_of_stores_t *ts,
                       table_of_vectorizations_t *tv,
                       table_of_ignored_t *ti,
                       table_of_pre_vectorization_t *tpv,
                       vectorizer_t *vectorizer);

        // **************************
        // No commit para treinamento
        // **************************
        void committed_ld (uop_package_t *uop);
        void committed_op (uop_package_t *uop);
        void committed_st (uop_package_t *uop);
        void committed_other (uop_package_t *uop);

        // *****************************************
        // No rename para pré-vetorização e rastreio
        // *****************************************
        void register_overwritten(uop_package_t *uop, table_of_vectorizations_entry_t *tv_entry);
        void renamed_ld (uop_package_t *uop);
        void renamed_op (uop_package_t *uop);
        void renamed_st (uop_package_t *uop);
        void renamed_other (uop_package_t *uop);


};