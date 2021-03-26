cmake_minimum_required(VERSION 3.16)
project(example_syn)

set(CMAKE_CXX_STANDARD 14)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)


find_package( Boost REQUIRED COMPONENTS
        filesystem locale system)
find_library(TINS_LIBRARY tins)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_library(tcp inc/tcp_send.h src/tcp_send.cpp)
#target_include_directories(tcp inc)

add_library(udp inc/udp_send.h src/udp_send.cpp)
#target_include_directories(udp inc)


add_executable(example_syn src/main.cpp ${Boost_INCLUDE_DIR} )
target_include_directories(example_syn PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(example_syn tcp udp ${Boost_LIBRARIES} tins)