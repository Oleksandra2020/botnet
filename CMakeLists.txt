cmake_minimum_required(VERSION 3.15)
project(test_ping)

set(CMAKE_CXX_STANDARD 14)

add_executable(test_ping main.cpp icmp_header.hpp ipv4_header.hpp)

find_package(Boost REQUIRED COMPONENTS
        iostreams
        locale
        system
        filesystem
        program_options)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})




