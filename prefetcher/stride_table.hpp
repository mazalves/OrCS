#ifndef STRIDE_TABLE_H
#define STRIDE_TABLE_H

class stride_table_t{

    public:
    // table Itens
    uint64_t tag;
    uint64_t last_address;
    uint32_t stride;
    status_stride_prefetcher_t status;
    uint64_t lru;
    stride_table_t(){
        this->clean_line();
    };
    ~stride_table_t(){};
    void clean_line(){
        this->tag=0;
        this->last_address=0;
        this->stride=0;
        this->status=INVALID;
        this->lru=0;
    };
};
#endif