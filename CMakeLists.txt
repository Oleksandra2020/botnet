cmake_minimum_required(VERSION 3.16)
project(example_syn)

set(CMAKE_CXX_STANDARD 14)

SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)


find_package( Boost REQUIRED COMPONENTS
        filesystem locale system)

find_package(Threads)

LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_executable(example_syn src/main.cpp ${Boost_INCLUDE_DIR} inc/thread_pool.h
               inc/tcp_send.h src/tcp_send.cpp
               inc/victim_manipulation.h src/victim_manipulation.cpp)

target_include_directories(example_syn PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(example_syn ${Boost_LIBRARIES} Threads::Threads)