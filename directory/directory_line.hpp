class directory_line_t {

    private:
        uint32_t POINTER_LEVELS;

    public:
        line_t *cache_lines;
        uint32_t shared;
        uint32_t status;
        uint32_t level;
        uint32_t id;

        directory_line_t();
        ~directory_line_t();
        void allocate();

        INSTANTIATE_GET_SET_ADD(uint32_t, POINTER_LEVELS)
};