class btb_t{

    public:
        btb_line_t *btb_entry;
    btb_t(){
        this->btb_entry = NULL;
    };
    ~btb_t(){
        if(this->btb_entry) delete [] btb_entry;
    };
};