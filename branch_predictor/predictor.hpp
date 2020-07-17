class predictor_t {
    public:
        virtual ~predictor_t(){};
        virtual void allocate (uint32_t processor_id){
            std::ignore = processor_id;
        };
        virtual taken_t predict (uint64_t address){
            std::ignore = address;
            return NOT_TAKEN;
        };
        virtual void train (uint64_t address,taken_t predict, taken_t correct){
            std::ignore = address;
            std::ignore = predict;
            std::ignore = correct;
        };
};