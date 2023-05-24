#include "rdma_server.h"
#include "rdma_connection.h"

int RdmaServer::start() {

#if _USE_IPV6
  struct sockaddr_in6 addr;
#else
  struct sockaddr_in addr;
#endif

 
  memset(&addr, 0, sizeof(addr));
#if _USE_IPV6
  addr.sin6_family = AF_INET6;
#else
  addr.sin_family = AF_INET;
#endif

  event_channel = rdma_create_event_channel();
  rdma_create_id(event_channel, &listener, nullptr, RDMA_PS_TCP);
  rdma_bind_addr(listener, (struct sockaddr *)&addr);
  rdma_listen(listener, 10); /* backlog=10 is arbitrary */

  uint16_t port = ntohs(rdma_get_src_port( listener)); // rdma_get_src_port 返回listener对应的tcp 端口

  printf("listening on port %d.\n", port);
  struct rdma_cm_event *event = nullptr;
  while (rdma_get_cm_event(event_channel, &event) == 0) {
    struct rdma_cm_event event_copy;
    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);

    if (on_event(&event_copy))
      break;
  }

  rdma_destroy_id(listener);
  rdma_destroy_event_channel(event_channel);
  return 0; 
}

int RdmaServer::on_event(struct rdma_cm_event *event) {
    int r = 0;
      switch(event->event ){
        case RDMA_CM_EVENT_CONNECT_REQUEST:
        {
          r = on_accept(event->id);
        }
        
        break; 
        case RDMA_CM_EVENT_ESTABLISHED:
        {
          r = on_connection(event->id->context);
        }
        
        break; 
        case RDMA_CM_EVENT_DISCONNECTED:
        {
          r = on_disconnect(event->id);
        }
        
        break; 
        default:
        printf("unknown event %d\n", event->event ) ;
      }

    return r;
  }


void RdmaServer::init_context(struct ibv_context *verbs) {

    if (rdma_context.ctx != verbs) {
        printf("cannot handle events in more than one context.");
        return;
    }

    rdma_context.ctx = verbs;

    rdma_context.pd = ibv_alloc_pd(rdma_context.ctx);
    rdma_context.comp_channel =
                ibv_create_comp_channel(rdma_context.ctx);
    rdma_context.cq = ibv_create_cq(rdma_context.ctx, 10, NULL,
                                            rdma_context.comp_channel,
                                            0); /* cqe=10 is arbitrary */

    ibv_req_notify_cq(rdma_context.cq, 0); //#完成完成队列与完成通道的关联


    rdma_context.poller_worker  = std::thread([this](){
        this->poll(); 
    }); 

}

  void RdmaServer::init_qp_attr(struct ibv_qp_init_attr *qp_attr) {
    memset(qp_attr, 0, sizeof(*qp_attr));

    qp_attr->send_cq = rdma_context.cq;
    qp_attr->recv_cq = rdma_context.cq;
    qp_attr->qp_type = IBV_QPT_RC;

    qp_attr->cap.max_send_wr = 10;
    qp_attr->cap.max_recv_wr = 10;
    qp_attr->cap.max_send_sge = 1;
    qp_attr->cap.max_recv_sge = 1;
  }

  void * RdmaServer::poll() {
    struct ibv_cq *cq;
    struct ibv_wc wc;
    void *ctx; 
    while (1) {
      ibv_get_cq_event(rdma_context.comp_channel, &cq, &ctx);
      ibv_ack_cq_events(cq, 1);
      ibv_req_notify_cq(cq, 0);

    while (ibv_poll_cq(cq, 1, &wc))
            on_completion(&wc);
    }

    return nullptr;
  }
 
 

  void RdmaServer::on_completion(struct ibv_wc *wc) {
    if (wc->status != IBV_WC_SUCCESS)
    {
         RdmaConnection *conn = (RdmaConnection *)(uintptr_t)wc->wr_id;
         conn->on_event(RdmaConnection::RDMA_EVENT_CLOSE); 
         return ; 
    }      

    if (wc->opcode & IBV_WC_RECV) {
        RdmaConnection *conn = (RdmaConnection *)(uintptr_t)wc->wr_id;
        conn->on_read(wc->byte_len); 
    } else if (wc->opcode == IBV_WC_SEND) {
      printf("send completed successfully.\n");
    }
  }

  int RdmaServer::on_accept(struct rdma_cm_id *id) {
     
    printf("on accept, received connection request.\n");

    init_context(id->verbs);

    struct ibv_qp_init_attr qpAttr;
    init_qp_attr(&qpAttr);

    rdma_create_qp(id, rdma_context.pd, &qpAttr);
    RdmaConnection *conn = new RdmaConnection(&rdma_context ); 
    conn->init(id);   

    struct rdma_conn_param cmParams;
    memset(&cmParams, 0, sizeof(cmParams));
    return rdma_accept(id, &cmParams);    
  }

  int RdmaServer::on_connection(void *context) {
    RdmaConnection *conn = (RdmaConnection *)context; 
    conn->on_event(RdmaConnection::RDMA_EVENT_OPEN); 
 
    return 0;
  }

  int RdmaServer::on_disconnect(struct rdma_cm_id *id) {
    RdmaConnection *conn = (RdmaConnection *)id->context;

    printf("peer disconnected.\n");
    delete conn; 
    return 0;
  }
