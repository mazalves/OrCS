class register_rename_table_t {
    public:
        bool vectorial;                                 /// Its a scalar or vetorial register
        int32_t offset;                                 /// Current element
        int32_t correspondent_vectorial_reg;            /// Registrador vetorial correspondente
                                                        //// Utilizado para validações

    inline register_rename_table_t () {
        this->vectorial = false;
        this->offset = -1;
        this->correspondent_vectorial_reg = -1;
    }
};