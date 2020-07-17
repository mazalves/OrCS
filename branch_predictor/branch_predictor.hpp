class branch_predictor_t{
    private:
        uint32_t processor_id;
        uint32_t BTB_ENTRIES;
        uint32_t BTB_WAYS;
        uint32_t BTB_MISS_PENALITY;
        uint32_t MISSPREDICTION_PENALITY;
        branch_prediction_method_t BRANCH_PREDICTION_METHOD;

    public:
        //===================================   
        //atributos BTB
        //===================================
        btb_t *btb;
        uint32_t btbHits;
	    uint32_t btbMiss;
        //===================================
        // para acesso direto btb
        //===================================
        uint32_t index;
        uint8_t way;
        //===================================
        //metodos para btb
        //===================================
        inline uint32_t searchLRU(btb_t *btb);
        uint32_t installLine(opcode_package_t instruction);
        uint32_t searchLine(uint64_t pc);
        //===================================
        //Atributos branch predictor
        //===================================
        uint32_t branchTaken;
        uint32_t branchNotTaken;
        uint32_t branches;
        uint32_t branchTakenMiss;
        uint32_t branchNotTakenMiss;
        predictor_t *branchPredictor;

        //===================================
        //metodos branch predictor
        //===================================
        branch_predictor_t();
        ~branch_predictor_t();
        void allocate (uint32_t processor_id);
        uint32_t solveBranch(opcode_package_t instruction, opcode_package_t nextOpcode);
        void statistics();
        void reset_statistics();

        INSTANTIATE_GET_SET_ADD (uint32_t, btbHits)
        INSTANTIATE_GET_SET_ADD (uint32_t, btbMiss)

        INSTANTIATE_GET_SET_ADD (uint32_t, branchTaken)
        INSTANTIATE_GET_SET_ADD (uint32_t, branchNotTaken)
        INSTANTIATE_GET_SET_ADD (uint32_t, branches)
        INSTANTIATE_GET_SET_ADD (uint32_t, branchTakenMiss)
        INSTANTIATE_GET_SET_ADD (uint32_t, branchNotTakenMiss)

        INSTANTIATE_GET_SET_ADD (uint32_t, BTB_ENTRIES)
        INSTANTIATE_GET_SET_ADD (uint32_t, BTB_WAYS)
        INSTANTIATE_GET_SET_ADD (uint32_t, BTB_MISS_PENALITY)
        INSTANTIATE_GET_SET_ADD (uint32_t, MISSPREDICTION_PENALITY)

};