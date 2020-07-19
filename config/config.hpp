class configure_t {
    private:
        libconfig::Config* cfg;

    public:
        configure_t();
        ~configure_t();
        void loadConfig();
        libconfig::Setting &getConfig();
};