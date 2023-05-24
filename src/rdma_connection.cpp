#include <string.h> 
#include <stdlib.h> 
#include "rdma_connection.h"
#include "rdma_context.h"

RdmaConnection::RdmaConnection(RdmaContext * ctx) {
    rdma_context = ctx;      

}

RdmaConnection::~RdmaConnection() {

    rdma_destroy_qp(conn_id);
    ibv_dereg_mr(this->send_mr);
    ibv_dereg_mr(this->recv_mr);
    free(this->send_region);
    free(this->recv_region);
    rdma_destroy_id(conn_id);
}

void RdmaConnection::init(rdma_cm_id *id ){

    id->context = this;    
    this->conn_id = id;
    this->qp = conn_id->qp;


    this->send_region = (char *)malloc(BUFFER_SIZE);
    this->recv_region = (char *)malloc(BUFFER_SIZE);

    this->send_mr =
               ibv_reg_mr(rdma_context->pd, this->send_region, BUFFER_SIZE, 0);

    this->recv_mr = ibv_reg_mr(rdma_context->pd, this->recv_region,
                                      BUFFER_SIZE, IBV_ACCESS_LOCAL_WRITE);

    do_read(); 
}

int RdmaConnection::send(const char * data, uint32_t dataLen){            

    struct ibv_send_wr wr, *bad_wr = NULL;

    memset(&wr, 0, sizeof(wr)); 
    
    memcpy(this->send_region, data, dataLen); 

        
    struct ibv_sge sge;

    wr.opcode = IBV_WR_SEND;
    wr.sg_list = &sge;
    wr.num_sge = 1;
    wr.send_flags = IBV_SEND_SIGNALED;

    sge.addr = (uintptr_t)this->send_region;
    sge.length = BUFFER_SIZE;
    sge.lkey = this->send_mr->lkey;

    return ibv_post_send(this->qp, &wr, &bad_wr);
}

int RdmaConnection::do_read(){

      
    struct ibv_recv_wr wr, *bad_wr = nullptr;
    struct ibv_sge sge;

    wr.wr_id = (uintptr_t)this;
    wr.next = nullptr ;
    wr.sg_list = &sge;
    wr.num_sge = 1;

    sge.addr = (uintptr_t)this->recv_region;
    sge.length = BUFFER_SIZE;
    sge.lkey = this->recv_mr->lkey;

    return ibv_post_recv(this->qp, &wr, &bad_wr);  
}
