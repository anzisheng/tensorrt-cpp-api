cmake_minimum_required(VERSION 3.18)
project(tensorrt_cpp_api)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_BUILD_TYPE Release)
#add_definitions(-DSHOW)
set(CMAKE_BUILD_TYPE Release)
# TODO: Specify the path to TensorRT root dir
set(TensorRT_DIR /usr/local/TensorRT-8.6.1.6/)
# Use ccache to speed up rebuilds
include(cmake/ccache.cmake)

# Set C++ version and optimization level
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Ofast -DNDEBUG -Wno-deprecated-declarations")

# For finding FindTensorRT.cmake
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# TODO: Specify the path to TensorRT root dir
if (NOT TensorRT_DIR)
    set(TensorRT_DIR /usr/local/TensorRT-8.6.1.6/)
endif()
# Use the correct version of CUDA
set(CUDA_TOOLKIT_ROOT_DIR /usr/local/cuda)

# We require CUDA, OpenCV, and TensorRT
find_package(TensorRT REQUIRED)
find_package(CUDA REQUIRED)
find_package(OpenCV REQUIRED)
find_package(fmt REQUIRED)
find_package(nlohmann_json  REQUIRED)
find_package(FMT REQUIRED)
find_package(Boost)
find_package(OpenSSL)
find_package(websocketpp)
find_package(jsoncpp)

add_library(tensorrt_cpp_api
        STATIC                     #SHARED
        src/engine.cpp
        src/yolov8.cpp
        src/utils.cpp
        src/utile.cpp
        src/Face68Landmarks_trt.cpp
        src/facerecognizer_trt.cpp
        src/faceswap_trt.cpp
        src/faceenhancer_trt.cpp
        #src/json_info.cpp
        )
include_directories(/usr/include/jsoncpp)
target_include_directories(tensorrt_cpp_api PUBLIC ${OpenCV_INCLUDE_DIRS} ${CUDA_INCLUDE_DIRS} ${TensorRT_INCLUDE_DIRS} 
/usr/local/TensorRT-8.6.1.6/include
/usr/local/TensorRT-8.6.1.6/samples/common/
include . include/interfaces)
target_link_libraries(tensorrt_cpp_api PUBLIC ${OpenCV_LIBS} ${CUDA_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${TensorRT_LIBRARIES} fmt::fmt)

#add_executable(run_inference_benchmark src/main.cpp)
add_executable(ss src/server_ws.cpp)
target_link_libraries(ss PUBLIC tensorrt_cpp_api  fmt::fmt -lssl -lcrypto  ${Boost_LIBS} ${OpenSSL_LIBS} nlohmann_json::nlohmann_json /usr/lib/x86_64-linux-gnu/libjsoncpp.so.1.9.5)
#target_link_libraries(server_face /usr/lib/x86_64-linux-gnu/libjsoncpp.so.1.9.5)
