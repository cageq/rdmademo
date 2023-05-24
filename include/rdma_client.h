#pragma once 

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread> 
#include <string> 
#include <rdma/rdma_cma.h>


#include "rdma_context.h"
#include "rdma_connection.h"
 
const int TIMEOUT_IN_MS = 500; /* ms */


class RdmaClient{

    public: 
        RdmaClient(); 

            int connect(const std::string &host  = "127.0.0.1",uint16_t port  = 6000) ; 
            
            void run(); 

            int on_event(struct rdma_cm_event *event) ; 
        

            void stop() ; 


            void poll(); 

                
            void init_context(struct ibv_context *verbs); 

            
            void init_qp_attr(struct ibv_qp_init_attr *qpAttr); 

            int on_addr_resolved(struct rdma_cm_id *id) ; 
            

            int on_route_resolved(struct rdma_cm_id *id) ; 

            void on_completion(struct ibv_wc *wc); 

            int on_connection(void *context); 
 
            int on_disconnect(struct rdma_cm_id *id); 


    private: 
        
            RdmaContext rdma_context; 
            struct rdma_event_channel *event_channel = nullptr ; 
}; 
 

 
  