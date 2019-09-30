class memory_request_client_t {
    public:
        package_state_t status;
        uint32_t readyAt;

        void updatePackageUntreated(uint32_t stallTime);
        void updatePackageReady(uint32_t stallTime);
        void updatePackageWait(uint32_t stallTime);
        void updatePackageFree(uint32_t stallTime);
};