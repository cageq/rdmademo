#include "rdma_client.h"

RdmaClient::RdmaClient() {
     event_channel = rdma_create_event_channel(); 
}

int RdmaClient::connect(const std::string &host ,uint16_t port ) {
    
    struct addrinfo *addr;
    getaddrinfo(host.c_str(), std::to_string(port).c_str(), nullptr, &addr);

    struct rdma_cm_id *cmId = nullptr;
    rdma_create_id(event_channel, &cmId, nullptr, RDMA_PS_TCP);
    rdma_resolve_addr(cmId, nullptr, addr->ai_addr, TIMEOUT_IN_MS);
    freeaddrinfo(addr);
    run();
  return 0;
}

void RdmaClient::run() {
  struct rdma_cm_event *event = nullptr;
  while (rdma_get_cm_event(event_channel, &event) == 0) {
    struct rdma_cm_event event_copy;
    memcpy(&event_copy, event, sizeof(*event));
    rdma_ack_cm_event(event);
    if (on_event(&event_copy)) {
      break;
    }
  }
}

int RdmaClient::on_event(struct rdma_cm_event *event) {

    int r = 0;
    switch (event->event) {
    case RDMA_CM_EVENT_ADDR_RESOLVED: {
        r = on_addr_resolved(event->id);
    } break;
    case RDMA_CM_EVENT_ROUTE_RESOLVED: {
        r = on_route_resolved(event->id);
    } break;
    case RDMA_CM_EVENT_ESTABLISHED: {
        r = on_connection(event->id->context);
    } break;
    case RDMA_CM_EVENT_DISCONNECTED: {
        r = on_disconnect(event->id);
    } break;
    default:;
    }

  return r;
}

void RdmaClient::stop() { rdma_destroy_event_channel(event_channel); }

void RdmaClient::poll() {
  struct ibv_cq *cq;
  struct ibv_wc wc;
  void *ctx = nullptr;
  while (1) {
    ibv_get_cq_event(rdma_context.comp_channel, &cq, &ctx);
    ibv_ack_cq_events(cq, 1);
    ibv_req_notify_cq(cq, 0);

    while (ibv_poll_cq(cq, 1, &wc))
      on_completion(&wc);
  }  
}

void RdmaClient::init_context(struct ibv_context *verbs) {
  rdma_context.ctx = verbs;
  rdma_context.pd = ibv_alloc_pd(rdma_context.ctx);
  rdma_context.comp_channel = ibv_create_comp_channel(rdma_context.ctx);

  rdma_context.cq =
      ibv_create_cq(rdma_context.ctx, 10, nullptr, rdma_context.comp_channel,
                    0); /* cqe=10 is arbitrary */
  ibv_req_notify_cq(rdma_context.cq, 0);

  rdma_context.poller_worker = std::thread([this]() { poll(); });
}

void RdmaClient::init_qp_attr(struct ibv_qp_init_attr *qpAttr) {
  memset(qpAttr, 0, sizeof(*qpAttr));

  qpAttr->send_cq = rdma_context.cq;
  qpAttr->recv_cq = rdma_context.cq;
  qpAttr->qp_type = IBV_QPT_RC;

  qpAttr->cap.max_send_wr = 10;
  qpAttr->cap.max_recv_wr = 10;
  qpAttr->cap.max_send_sge = 1;
  qpAttr->cap.max_recv_sge = 1;
}

int RdmaClient::on_addr_resolved(struct rdma_cm_id *id) {
  RdmaConnection *conn = new RdmaConnection(&rdma_context );
  
  printf("address resolved.\n");
  
  init_context(id->verbs);
  struct ibv_qp_init_attr qpAttr;
  init_qp_attr(&qpAttr);
  rdma_create_qp(id, rdma_context.pd, &qpAttr);
  conn->init(id);  

  rdma_resolve_route(id, TIMEOUT_IN_MS);
  return 0;
}
 

int RdmaClient::on_route_resolved(struct rdma_cm_id *id)
{
    struct rdma_conn_param cm_params;
    printf("route resolved.\n");

    memset(&cm_params, 0, sizeof(cm_params));
    return rdma_connect(id, &cm_params);    
}

int RdmaClient::on_connection(void *context) {
    RdmaConnection *conn = (RdmaConnection *)context;
 
    conn->on_event(RdmaConnection::RDMA_EVENT_OPEN); 

  return 0;
}



void RdmaClient::on_completion(struct ibv_wc *wc)
{
    auto conn = (RdmaConnection *)(uintptr_t)wc->wr_id;

    if (wc->status != IBV_WC_SUCCESS)
    {
            printf("on_completion: status is not IBV_WC_SUCCESS.");
            return ; 
    }
    

    if (wc->opcode & IBV_WC_RECV)
    {
        printf("received message: %s\n", conn->recv_region);
    }        
    else if (wc->opcode == IBV_WC_SEND)
    {
        printf("send completed successfully.\n");
    }        
    else
    {
        printf("on_completion: completion isn't a send or a receive.");
    }
        

    if (++conn->num_completions == 2)
        rdma_disconnect(conn->conn_id);
}

 

int RdmaClient::on_disconnect(struct rdma_cm_id *id) {
  auto conn = (RdmaConnection *)id->context;
  delete conn; 

  return 0; /* exit event loop */
}
