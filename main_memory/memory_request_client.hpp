#ifndef MEMORY_REQUEST_CLIENT_H
#define MEMORY_REQUEST_CLIENT_H
class memory_request_client_t {
    public:
        package_state_t status;
        uint32_t readyAt;
        memory_operation_t memory_operation;
        bool waiting_DRAM;
        
        
        memory_request_client_t();
        virtual ~memory_request_client_t();
        #ifndef __PIN__
        virtual void updatePackageUntreated(uint32_t stallTime);
        virtual void updatePackageReady(uint32_t stallTime);
        virtual void updatePackageWait(uint32_t stallTime);
        virtual void updatePackageFree(uint32_t stallTime);
        #endif
};
#endif
