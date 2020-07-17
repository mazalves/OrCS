class piecewise_t : public predictor_t {
private:
    uint32_t processor_id;
    uint64_t N;
    uint64_t M;
    uint64_t H;

    uint64_t THETA;

public:
    int8_t ***W;
    uint64_t *GA;
    uint8_t *GHR;
    float saida;
    void allocate (uint32_t processor_id);
    taken_t predict(uint64_t address);
    void train(uint64_t address,taken_t predict, taken_t correct);
    piecewise_t();
    ~piecewise_t();

    INSTANTIATE_GET_SET_ADD(uint32_t, N)
    INSTANTIATE_GET_SET_ADD(uint32_t, M)
    INSTANTIATE_GET_SET_ADD(uint32_t, H)
    
};