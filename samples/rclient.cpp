#include "rdma_client.h"



int main(int argc , char * argv[]){
    std::string host = "127.0.0.1"; 
    int16_t port = 6000; 
    if (argc > 2){
      host = argv[1]; 
      port = atoi(argv[2]); 
    }

    RdmaClient rdmaClient; 
    rdmaClient.connect(host, port); 
    return 0; 
}
