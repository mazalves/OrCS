class twoBit_t{

    public:
    twoBit_t();
    ~twoBit_t();
    void allocate();
    void statistics();
    uint32_t predict(uint64_t address);
    void train(uint64_t address);
   
};