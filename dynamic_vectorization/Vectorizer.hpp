#ifndef VECTORIZER_HPP
#define VECTORIZER_HPP
class Vectorizer_t {
    public:
        uint64_t GMRBB;
        VR_state_bits_t *vr_control_bits;
        std::vector<int32_t> vr_state;
        circular_buffer_t<int32_t> free_VR_list; // Lista de registradores livres para alocação rápida
        std::list<int32_t> allocated_VR_list; // Lista de registradores alocados para verificação rápida do MRBB

        bool *pipeline_squashed;
        uint64_t *store_squashing;

        vector_map_table_t *VRMT;
        table_of_loads_t *TL;
        register_rename_table_t *register_rename_table;

        // Tempo de requisição para o vetorizador
        uint32_t vectorizer_request_latency;

        // Statistics
        uint64_t vectorized_loads;
        uint64_t vectorized_ops;
        uint64_t totalLoadInstructions;
        uint64_t validationsLoads; // All loads that became validations
        uint64_t totalOtherInstructions;
        uint64_t validationsOther;

        uint64_t validated;
        uint64_t registersChanged;
        uint64_t withVectorialOperands;
        uint64_t withoutVectorialOperands;
        uint64_t notLoads;
        uint64_t otherInsider;
        uint64_t loadsInsider;
        uint64_t otherInsiderWithReads;
        uint64_t notVectorizedOutsider;
        uint64_t vectorialPartsLoads;
        uint64_t vectorialPartsOthers;
        uint64_t commonInstructionsLoads;
        uint64_t commonInstructionsOthers;

        uint64_t twoReadsLoads;
        uint64_t twoReadsOthers;

        uint64_t stride_changed;
        uint64_t stride_confirmed;
        uint64_t stride_greater_than_max;

        uint64_t pre_vectorizations;

        // Vetorização incorreta
        uint64_t source_changes_before_end;
        uint64_t source_changes_after_end;
        uint64_t changing_vectorized_stride;





        // Methods
        int32_t allocate_VR(int32_t logical_register);
        DV::DV_ERROR new_commit (uop_package_t *inst);
        DV::DV_ERROR new_inst (opcode_package_t *inst);
 
        DV::DV_ERROR new_memory_operation (memory_order_buffer_line_t *mem_access);
        
        // Invalida dados de vetorização
        // Retorna última posicação validada da vetorização
        // TODO
        uint32_t request_invalidation_from_TM (transactional_operation_t *TM_line);
        
        bool vectorial_operands (opcode_package_t *inst);
        DV::DV_ERROR enter_pipeline (opcode_package_t *inst);
        void GMRBB_changed ();
        void free_VR (int32_t vr_id);
        void squash_pipeline();
        void resume_pipeline();
        Vectorizer_t(bool *pipeline_squashed, uint64_t *store_squashing);
        void allocate(circular_buffer_t <opcode_package_t> *inst_list);
        ~Vectorizer_t();
        void statistics();
        void debug();

        // Registers SETTERs and GETTERs
        inline void set_executed (int32_t vr_id, int32_t index, bool value);        
        inline void set_sent (int32_t vr_id, int32_t index, bool value);
        inline void set_free (int32_t vr_id, int32_t index, bool value);
        inline void set_V (int32_t vr_id, int32_t index, uint32_t value);
        inline void set_R (int32_t vr_id, int32_t index, uint32_t value);
        inline void set_U (int32_t vr_id, int32_t index, uint32_t value);
        inline void set_F (int32_t vr_id, int32_t index, uint32_t value);

        // Register operations
        inline void sub_V (int32_t vr_id, int32_t index, uint32_t value);
        inline void sub_R (int32_t vr_id, int32_t index, uint32_t value);
        inline void sub_U (int32_t vr_id, int32_t index, uint32_t value);
        inline void sub_F (int32_t vr_id, int32_t index, uint32_t value);

        inline void add_R (int32_t vr_id, int32_t index, uint32_t value);

};


inline void Vectorizer_t::set_V (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].V = value;
    }
}
inline void Vectorizer_t::set_R (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].R = value;
    }
}
inline void Vectorizer_t::set_U (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].U = value;
    }
}
inline void Vectorizer_t::set_F (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].F = value;
    }
}


inline void Vectorizer_t::set_executed (int32_t vr_id, int32_t index, bool value){
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].executed = value;
    }
}      
inline void Vectorizer_t::set_sent (int32_t vr_id, int32_t index, bool value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].sent = value;
    }
}

inline void Vectorizer_t::set_free (int32_t vr_id, int32_t index, bool value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].free = value;
    }
}

inline void Vectorizer_t::sub_V (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].V -= value;
    }
}
inline void Vectorizer_t::sub_R (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].R -= value;
    }
}
inline void Vectorizer_t::sub_U (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].U -= value;
    }
}
inline void Vectorizer_t::sub_F (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].F -= value;
    }
}
inline void Vectorizer_t::add_R (int32_t vr_id, int32_t index, uint32_t value) {
    if (index < VECTORIZATION_SIZE) {
        this->vr_control_bits[vr_id].positions[index].R += value;
    }
}
#endif
