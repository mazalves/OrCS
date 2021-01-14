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
    uint64_t memory_address;
    uint32_t memory_size;

    uint64_t read_address;
    uint64_t read2_address;
    uint64_t write_address;
    
    bool is_hive;
    int32_t hive_read1;
    int32_t hive_read2;
    int32_t hive_write;

    bool is_vima;

    int is_vectorial_part;          // Identifica se é parte de uma instrução vetorial maior
    int32_t VR_id;                  // Contém o VR de uma instrução vetorial
                                    // (se for uma instrução vetorial ou sua validação)
    bool is_validation;             // Indica se é apenas uma validação
    int32_t will_free;              // Registrador vetorial que será liberado quando ela comitar (F).
    int32_t will_free_offset;       // Indica o offset que será liberado
    bool is_BB;                     // Indica se é um backward branch

    void opcode_to_uop(uint64_t uop_number, instruction_operation_t uop_operation, uint64_t memory_address, uint32_t memory_size, opcode_package_t opcode);
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
};