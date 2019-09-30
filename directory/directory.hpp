class directory_t {
    
    public:
        directory_line_t *directory_lines;

        directory_t();
        ~directory_t();
        void allocate();
}