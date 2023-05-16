cmake_minimum_required(VERSION 3.13)
project (rdma VERSION 0.6 )
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
set(HDS_BUILD_TIME  ${COMPILE_TIME})
set(HDS_VERSION_MAJOR  3) 
set(HDS_VERSION_MINOR  0) 
set(HDS_VERSION_BUILD  317) 
set(HDS_VERSION_FLAG   beta)

configure_file(src/version.h.in src/version.h)


include_directories(opt/asio-1.22.1/include)
include_directories(opt/knet/include)

include_directories(opt/fmt-9.1.0/include)
include_directories(include)
include_directories(src)
include_directories(opt/htflog/include)
include_directories(opt/concurrentqueue/)

include_directories(./opt/rdma-core/build/include)
link_directories(./opt/rdma-core/build/)

link_directories(bin)
set(KBUSVER  kbus-v1.0.1208) 
include_directories(opt/${KBUSVER}/include )
link_directories(opt/${KBUSVER}/lib)

if(WITH_PYHDS)
include_directories(bin/python/include/python3.9)
link_directories(bin/python/lib) 
include_directories(opt/pybind11/include)
add_library(pyhds SHARED src/pyhds.cpp)
target_compile_options(pyhds PRIVATE -frtti  -fvisibility=default)
target_link_libraries(pyhds hds ) 
endif() 




link_directories(opt/core) 

add_executable(rdmasrv src/rdma_server.cpp)
target_link_libraries(rdmasrv rdma_util ibverbs rdmacm) 
add_executable(rdmacli src/rdma_client.cpp)
target_link_libraries(rdmacli rdma_util ibverbs rdmacm) 


