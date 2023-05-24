#pragma once 
#include <memory>
#include <stdio.h> 
#include "rdma_context.h"

class RdmaConnection{

public: 

    enum RdmaEvent{
        RDMA_EVENT_INIT, 
        RDMA_EVENT_OPEN, 
        RDMA_EVENT_CLOSE, 
    }; 

        RdmaConnection(RdmaContext * ctx ); 

        ~RdmaConnection() ; 
        void init(rdma_cm_id *id ); 
 
        

        int do_read();
        void on_event(int evt){}
        void on_read( uint32_t len){
            printf("received message: %d\n", len);
        }

 
        int send(const char * data, uint32_t dataLen) ; 


        int num_completions = 0 ;

        struct ibv_qp *qp;
        struct ibv_mr *recv_mr;
        struct ibv_mr *send_mr;

        char *recv_region;
        char *send_region;
        struct rdma_cm_id *conn_id = nullptr ;
        RdmaContext * rdma_context; 
}; 


using RdmaConnectionPtr = std::shared_ptr<RdmaConnection>; 