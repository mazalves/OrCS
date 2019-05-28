class configure_t {
    private:
        libconfig::Config cfg;
        std::map<std::string, int> conf;

    public:
        configure_t();
        ~configure_t();
        void loadConfig();
        int getSetting (std::string setting);
};