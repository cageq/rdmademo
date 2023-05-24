#pragma once
#include "rdma_connection.h"
#include "rdma_context.h"

class RdmaServer {

    public:
      int start()  ; 

      void init_context(struct ibv_context *verbs) ; 
      void init_qp_attr(struct ibv_qp_init_attr *qp_attr)  ; 

      void *poll() ; 
   
      void on_completion(struct ibv_wc *wc) ; 

      int on_accept(struct rdma_cm_id *id) ; 

      int on_connection(void *context) ; 

      int on_disconnect(struct rdma_cm_id *id) ; 

      int on_event(struct rdma_cm_event *event) ; 

    private: 
        RdmaContext rdma_context;
        struct rdma_event_channel *event_channel = NULL;
        struct rdma_cm_id *listener = NULL;
};
