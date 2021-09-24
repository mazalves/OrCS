class uop_package_t{
    
    public:
    uop_package_t();
    ~uop_package_t();
    
    /// TRACE Variables
    char opcode_assembly[MAX_ASSEMBLY_SIZE];
    instruction_operation_t opcode_operation;
    uint64_t opcode_address;
    uint32_t opcode_size;

    int32_t read_regs[MAX_REGISTERS];
    int32_t write_regs[MAX_REGISTERS];


    instruction_operation_t uop_operation;
    uint64_t memory_address[MAX_MEM_OPERATIONS];
    uint32_t memory_size[MAX_MEM_OPERATIONS];
    uint32_t num_mem_operations;

    uint64_t read_address;
    uint64_t read2_address;
    uint64_t write_address;

    bool is_hive;
    int32_t hive_read1;
    int32_t hive_read2;
    int32_t hive_write;

    bool is_vima;

    uint32_t latency;
    uint32_t throughput;
    functional_unit_t *functional_unit;

    // # do uop na instrução
    uint8_t uop_id;
    bool waiting; // Esperando correspondente vetorial, não executa, só fica no ROB e calcula o endereço se for ld ou st
    bool reexecution; // Foi reexecutada escalarmente após uma tentativa de vetorização frustrada
    uint32_t validation_number; // Número da validação da qual a instrução faz parte
                                // Usado para a verificação do stride pela AGU
    uint8_t structural_id;

    bool sent_to_new_renamed_uop; // Controle para enviar apenas uma vez mesmo com buffers cheios

    void opcode_to_uop(uint64_t uop_number, 
            instruction_operation_t uop_operation, 
            uint32_t latency, uint32_t throughput, functional_unit_t *fu_id,
            opcode_package_t opcode, uint8_t uop_id);

    inline void add_memory_operation(uint64_t memory_address, uint32_t memory_size);

    inline void add_vectorization_reference(table_of_vectorizations_entry_t *tl_entry);


    bool operator==(const uop_package_t &package);
    void package_clean();
    void updatePackageUntrated(uint32_t stallTime);
    void updatePackageReady(uint32_t stallTime);
    void updatePackageWait(uint32_t stallTime);
    void updatePackageFree(uint32_t stallTime);
    std::string content_to_string();
    std::string content_to_string2();
    // Control variables
    uint64_t opcode_number;
    uint64_t uop_number;
    uint64_t born_cycle;
    uint64_t readyAt;
    package_state_t status;

    // Quando completar a execução, marca na tabela que ela está completa
    // Permitindo o commit das instruções ignoradas (após juntar todas e garantir que deu certo :p)
    table_of_vectorizations_entry_t *tv_pointer;
};

inline void uop_package_t::add_memory_operation(uint64_t memory_address, uint32_t memory_size) {
    assert(num_mem_operations < MAX_MEM_OPERATIONS);
    this->memory_address[num_mem_operations] = memory_address;
    this->memory_size[num_mem_operations] = memory_size;
    ++num_mem_operations;

    return;
}

inline void uop_package_t::add_vectorization_reference(table_of_vectorizations_entry_t *tv_entry) {
    this->tv_pointer = tv_entry;
}
