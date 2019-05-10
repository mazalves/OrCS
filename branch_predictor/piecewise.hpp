class piecewise_t{

public:
    int8_t ***W;
    uint64_t *GA;
    uint8_t *GHR;
    float saida;
    void allocate();
    taken_t predict(uint64_t address);
    void train(uint64_t address,taken_t predict, taken_t correct);
    piecewise_t();
    ~piecewise_t();
    
};