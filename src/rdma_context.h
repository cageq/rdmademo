#pragma once 
#include <thread> 
#include <rdma/rdma_cma.h>


const int BUFFER_SIZE = 1024;
struct RdmaContext {
    bool is_inited = false; 
    struct ibv_context *ctx;
    struct ibv_pd *pd;
    struct ibv_cq *cq;
    struct ibv_comp_channel *comp_channel;    
    std::thread poller_worker; 
};
