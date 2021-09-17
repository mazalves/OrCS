class registers_tracker_entry_t {
    public:
        // ######################################################################
        // Commit
        // ######################################################################
        //bool validation_restrict; // Indica que está no meio de uma validação (começou por um load, mas pode ser indicado pelos ponteiros)
        table_of_loads_entry_t *tl_pointer; // Aponta para a entrada na TL da instrução que o gerou
        table_of_operations_entry_t *to_pointer; // Aponta para a entrada na TO da instrução que o gerou
        table_of_stores_entry_t *ts_pointer; // Aponta para a entrada na TS da instrução que o gerou

        // ######################################################################
        // Rename
        // ######################################################################
        table_of_vectorizations_entry_t *tv_pointer;

        void allocate() {
            //this->validation_restrict = false;
            this->tl_pointer = NULL;
            this->to_pointer = NULL;
            this->ts_pointer = NULL;
            this->tv_pointer = NULL;
        }

        void reset_commit_pointers() {
            tl_pointer = NULL;
            to_pointer = NULL;
            ts_pointer = NULL;
        }

        void set_commit_pointers (table_of_loads_entry_t *tl_entry, table_of_operations_entry_t *to_entry,
                                  table_of_stores_entry_t *ts_entry) {
            this->tl_pointer = tl_entry;
            this->to_pointer = to_entry;
            this->ts_pointer = ts_entry;
        }


        void reset_rename_pointers() {
            tv_pointer = NULL;
        }

        void set_rename_pointers(table_of_vectorizations_entry_t *tv_entry) {
            this->tv_pointer = tv_entry;
        }



        /*
        void validation_begin() {
            this->validation_restrict = 
        }

        void validation_end() {

        }
        */

};