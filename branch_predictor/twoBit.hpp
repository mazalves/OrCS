class twoBit_t: public predictor_t {

    public:
    twoBit_t();
    ~twoBit_t();
    void statistics();
    void allocate();
    taken_t predict (uint64_t address);
    void train(uint64_t address,taken_t predict, taken_t correct);
   
};