class transactional_operation_t {
    public:
        // Para receber informações e invalidar em trocas de contexto
        bool valid;
        uint64_t uop_number;
        uint64_t processor_id;
        uint64_t thread_id;

        // Para invalidar por acessos externos
        uint64_t source_addr[2];
        uint64_t destiny_addr;

        // Para registrar commits e invalidações
        vima_vector_t *xValid;
        // vima_vector *xTemp; -> Apenas fingimos

        // Para verificações e validações/invalidações
        uint32_t operation_size;

        // Validações
        uint32_t num_validated;

        // Contadores
        uint32_t total_validated;
        uint32_t total_discarded;



        transactional_operation_t() {
            valid          = false;
            uop_number     = 0;
            processor_id   = 0;
            thread_id      = 0;

            source_addr[0] = 0;
            source_addr[1] = 0;
            destiny_addr   = 0;

            xValid         = NULL;
            operation_size = 0;
            num_validated  = 0;

            total_validated = 0;
            total_discarded = 0;

        }



        inline bool request_owner(memory_package_t* request) {
            if (request->processor_id == this->processor_id &&
                // request->thread_id == this->thread_id &&
                request->uop_number == this->uop_number) {
                    return true;
                }
            return false;
        }

        inline bool intersect(uint64_t addr_a, uint32_t size_a, uint64_t addr_b, uint32_t size_b) {
            uint64_t menor = (addr_a > addr_b) ? addr_b : addr_a;
            uint64_t maior = (addr_a > addr_b) ? addr_a : addr_b;
            uint32_t tam_menor = (addr_a > addr_b) ? size_b : size_a;

            // Maior precisa começar antes do menor terminar
            if (maior < menor + tam_menor) {
                return true;
            }
            return false;
        }

        // Retorna true se invalidou
        // Retorna false se não invalidou
        bool possible_invalidation(memory_package_t* request);
};