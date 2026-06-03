cmake_minimum_required(VERSION 3.13)
project(OpenCalcypher VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra")

find_package(OpenSSL REQUIRED)

set(OPENFHE_INCLUDE_DIRS /usr/local/include/openfhe)
find_library(OPENFHE_PKE_LIB OPENFHEpke PATHS /usr/local/lib NO_DEFAULT_PATH)
find_library(OPENFHE_CORE_LIB OPENFHEcore PATHS /usr/local/lib NO_DEFAULT_PATH)

if(NOT OPENFHE_PKE_LIB OR NOT OPENFHE_CORE_LIB)
    message(FATAL_ERROR "OpenFHE libraries not found in /usr/local/lib")
endif()

add_executable(opencalcypher src/opencalcypher.cpp)
target_include_directories(opencalcypher SYSTEM PRIVATE
    ${OPENFHE_INCLUDE_DIRS}
    /usr/local/include/openfhe/core
    /usr/local/include/openfhe/pke
    /usr/local/include/openfhe/binfhe
    /usr/local/include/openfhe/lattice
)
target_link_libraries(opencalcypher
    ${OPENFHE_PKE_LIB}
    ${OPENFHE_CORE_LIB}
    OpenSSL::Crypto
)

install(TARGETS opencalcypher DESTINATION bin)
