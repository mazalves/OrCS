class directory_manager_t {

    private:
        uint32_t LLC_CACHES;
        
    public:
        directory_t *directory;

        directory_manager_t();
        ~directory_manager_t();
        void allocate();

        INSTANTIATE_GET_SET_ADD(uint32_t, LLC_CACHES)
};