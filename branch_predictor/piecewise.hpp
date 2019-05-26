class piecewise_t {
private:
    uint64_t N = 128;
    uint64_t M = 128;
    uint64_t H = 43;

    uint64_t THETA = ((2.14*(H)) + 20.58);

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