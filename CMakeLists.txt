cmake_minimum_required(VERSION 3.13)
 
project (rdmanet VERSION 0.6 )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -g  -Wall -Wno-unused-function -Wl,-rpath,. -Wl,-rpath,bin  ")
elseif(UNIX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread  -O2 -g    -Wno-unused-function  -Wno-deprecated-declarations -Wl,-rpath,. -Wl,-rpath,bin"  )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread  -g  -Wno-unused-function  -Wno-deprecated-declarations -Wl,-rpath,. -Wl,-rpath,bin  -fPIC  -fvisibility=default "  )
elseif(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
endif()


string(TIMESTAMP COMPILE_TIME "%Y/%m/%d %H:%M:%S")
set(RDMA_BUILD_TIME  ${COMPILE_TIME})
set(RDMA_VERSION_MAJOR  3) 
set(RDMA_VERSION_MINOR  0) 
set(RDMA_VERSION_BUILD  317) 
set(RDMA_VERSION_FLAG   beta)

configure_file(src/version.h.in src/version.h)



include_directories(include)
include_directories(src)

include_directories(opt/rdma-core/build/include/)
add_library(rdmanet src/rdma_connection.cpp src/rdma_server.cpp src/rdma_client.cpp)


link_directories(opt/rdma-core/build/lib )
link_directories(opt/rdma-core/build/util)

add_executable(rclient samples/rclient.cpp)
target_link_libraries(rclient rdmanet rdma_util ibverbs rdmacm )
add_executable(rserver samples/rserver.cpp)
target_link_libraries(rserver rdmanet rdma_util ibverbs rdmacm )
