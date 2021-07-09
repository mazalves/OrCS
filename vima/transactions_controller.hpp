class transactions_controller_t {
    private:
        transactional_operation_t *entries;
        uint32_t num_entries;

        // Tempo de requisição para o vetorizador
        uint32_t vectorizer_request_latency;

    public:

        transactions_controller_t() {
            this->entries = NULL;
            num_entries = 0;
        }


        ~transactions_controller_t();


        void allocate();

        transactional_operation_t * get_entry();

        // Chamado pelo memory_controller a cada request recebido (inclusive vindo da VIMA)
        void new_memory_access(memory_package_t* request);

};