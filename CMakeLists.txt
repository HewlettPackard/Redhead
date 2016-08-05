cmake_minimum_required(VERSION 3.5)
project(FASD)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
include_directories(${CMAKE_SOURCE_DIR})

find_package(MPI)
include_directories(${MPI_INCLUDE_PATH})

#include_directories($(FASD_SOURCE_DIR)/src/applications/)

find_package(Boost REQUIRED COMPONENTS system iostreams filesystem regex)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${FASD_SOURCE_DIR}/bin/)

#StencilForStencilForTxHPC
#JERASURE
include_directories(${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/jerasure/include/)
set(JERASURE_HEADERS_PATH ${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/jerasure)
set(JERASURE_HEADERS ${JERASURE_HEADERS_PATH}/include/cauchy.h ${JERASURE_HEADERS_PATH}/include/galois.h  ${JERASURE_HEADERS_PATH}/include/jerasure.h ${JERASURE_HEADERS_PATH}/include/liberation.h ${JERASURE_HEADERS_PATH}/include/reed_sol.h)
set(JERASURE_SOURCES ${JERASURE_HEADERS_PATH}/src/cauchy.c ${JERASURE_HEADERS_PATH}/src/galois.c ${JERASURE_HEADERS_PATH}/src/jerasure.c ${JERASURE_HEADERS_PATH}/src/liberation.c ${JERASURE_HEADERS_PATH}/src/reed_sol.c)
set(JERASURE ${JERASURE_SOURCES} ${JERASURE_HEADERS})

#TxHPC
include_directories(${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/raid6/include/)
set(TxHPC ${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/raid6/src/r6.c ${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/raid6/include/r6_utils.h)

#GF
include_directories(${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/gf-complete/include)
set(GF_PATH ${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC4TM/gf-complete)
set(GF_HEADERS ${GF_PATH}/include/gf_complete.h ${GF_PATH}/include/gf_general.h ${GF_PATH}/include/gf_int.h ${GF_PATH}/include/gf_method.h ${GF_PATH}/include/gf_rand.h)
set(GF_SOURCES ${GF_PATH}/src/gf.c ${GF_PATH}/src/gf_general.c ${GF_PATH}/src/gf_method.c ${GF_PATH}/src/gf_rand.c ${GF_PATH}/src/gf_w4.c ${GF_PATH}/src/gf_w8.c ${GF_PATH}/src/gf_w16.c ${GF_PATH}/src/gf_w32.c ${GF_PATH}/src/gf_w64.c ${GF_PATH}/src/gf_w128.c ${GF_PATH}/src/gf_wgen.c)
set(GF ${GF_HEADERS} ${GF_SOURCES})


set(EXAMPLE_APPLICATIONS src/applications/jacobian.cpp src/applications/jacobian.h src/applications/gameOfLife.cpp src/applications/gameOfLife.h src/applications/sparseMatrixMultiplication.cpp src/applications/sparseMatrixMultiplication.h src/applications/v2_SparseM.cpp src/applications/v2_SparseM.h src/applications/access_example.cpp src/applications/access_example.h)
add_executable(FASD src/measurements/run_applications.cpp ${EXAMPLE_APPLICATIONS})
target_link_libraries(FASD ${Boost_LIBRARIES})

#set(StencilForTxHPC_RAID6_DIR "/home/kuscum/workspace/TAS/include/StencilForStencilForTxHPC/TxHPC/raid6")
#include_directories(${StencilForTxHPC_RAID6_DIR})
#
#ADD_LIBRARY(
#        StencilForTxHPC_RAID6_LIB
#        ${StencilForTxHPC_RAID6_DIR}/test.c
#)

#ADDING COMPILER LINKER FLAGS

SET(GCC_SSE2_FLAGS   "-msse2 -DINTEL_SSE2")
SET(GCC_SSE3_FLAGS   "-msse3 -DINTEL_SSE3")
SET(GCC_SSSE3_FLAGS   "-mssse3 -DINTEL_SSSE3")
SET(GCC_SSSE4.1_FLAGS   "-msse4.1 -DINTEL_SSE4")
SET(GCC_SSE4.2_FLAGS   "-msse4.2 -DINTEL_SSE4")
SET(GCC_MPCL_MUL   "-mpclmul -DINTEL_SSE4_PCLMUL")

SET(CMAKE_CXX_FLAGS "-DJERASURE ${CMAKE_EXE_LINKER_FLAGS} ${GCC_SSE2_FLAGS} ${GCC_SSE3_FLAGS} ${GCC_SSSE3_FLAGS} ${GCC_SSSE4.1_FLAGS} ${GCC_SSE4.2_FLAGS} ${GCC_MPCL_MUL}" )
SET(CMAKE_C_FLAGS "-DJERASURE ${GCC_SSE2_FLAGS} ${GCC_SSE3_FLAGS} ${GCC_SSSE3_FLAGS} ${GCC_SSSE4.1_FLAGS} ${GCC_SSE4.2_FLAGS} ${GCC_MPCL_MUL}" )


set(StencilForTxHPC ${JERASURE} ${GF} ${TxHPC} ${EXAMPLE_APPLICATIONS})
add_executable(HELLOWORLD_StencilForTxHPC ${FASD_SOURCE_DIR}/src/helloWorlds/TxHPC_HelloWorld.cpp ${StencilForTxHPC} ${FASD_SOURCE_DIR}/include/StencilForTxHPC/TxHPC.h)
target_link_libraries(HELLOWORLD_StencilForTxHPC ${Boost_LIBRARIES})
target_compile_features(HELLOWORLD_StencilForTxHPC PRIVATE cxx_range_for)
#[[target_link_libraries(HELLOWORLD_StencilForTxHPC StencilForTxHPC_RAID6_LIB)]]
