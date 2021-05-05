cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME botnet)
project(${PROJECT_NAME}
        VERSION 0.0.1
        DESCRIPTION "Botnet"
        HOMEPAGE_URL "https://github.com/Oleksandra2020/botnet"
        LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
set(HEADERS_DIR ${CMAKE_SOURCE_DIR}/inc)
set(SRC ${CMAKE_SOURCE_DIR}/src)
set(CLASSES_DIR ${SRC}/classes)

#====================================
# Include directories
include_directories(${HEADERS_DIR})
#====================================

file(GLOB HEADERS ${HEADERS_DIR}/*.h)
file(GLOB CLASSES ${CLASSES_DIR}/*.cpp)

add_executable(${PROJECT_NAME} ${SRC}/main.cpp ${HEADERS} ${CLASSES})

#====================================
# Add external libraries
# Threads
find_package(Threads)
target_link_libraries(${PROJECT_NAME} Threads::Threads)


# Boost.Asio
find_package(Boost 1.71 REQUIRED COMPONENTS
            system)       
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} Threads::Threads)

# Ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${CURSES_LIBRARY})