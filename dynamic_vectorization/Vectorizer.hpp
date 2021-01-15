#ifndef VECTORIZER_HPP
#define VECTORIZER_HPP
class Vectorizer_t {
    public:
        uint64_t GMRBB;
        VR_state_bits_t *vr_control_bits;
        std::vector<int32_t> vr_state;

        vector_map_table_t *VRMT;
        table_of_loads_t *TL;
        register_rename_table_t *register_rename_table;

        // Methods
        int32_t allocate_VR(int32_t logical_register);
        DV::DV_ERROR new_commit (uop_package_t *inst);
        DV::DV_ERROR new_inst (uop_package_t *inst);
        void set_U (int32_t vr_id, int32_t index, bool value);

        bool vectorial_operands (uop_package_t *inst);
        void enter_pipeline (uop_package_t *inst);
        void GMRBB_changed ();
        void free_VR (int32_t vr_id);
        void squash_pipeline();
        Vectorizer_t(circular_buffer_t <uop_package_t> *inst_list);
        ~Vectorizer_t();

};
#endif