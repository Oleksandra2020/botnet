cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME botnet)
project(${PROJECT_NAME}
        VERSION 0.0.1
        DESCRIPTION "Botnet"
        HOMEPAGE_URL "https://github.com/Oleksandra2020/botnet"
        LANGUAGES C CXX)

set(CLI server_client)
set(ADMIN manager)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
SET(Boost_USE_STATIC_LIBS OFF)
SET(Boost_USE_MULTITHREAD ON)
set(HEADERS_DIR ${CMAKE_SOURCE_DIR}/inc)
set(SRC ${CMAKE_SOURCE_DIR}/src)
set(CLASSES_DIR ${SRC}/classes)

#====================================
# Include directories
include_directories(${HEADERS_DIR} ${HEADERS_DIR}/${CLI} ${HEADERS_DIR}/${ADMIN})
#====================================
# Need to make shared lib out of this
set(SHARED_CLASSES ${HEADERS_DIR}/msg_parser.h  ${HEADERS_DIR}/session.h ${CLASSES_DIR}/msg_parser.cpp ${CLASSES_DIR}/session.cpp)
#====================================
# Client / server
file(GLOB S_HEADERS ${HEADERS_DIR}/${CLI}/*.h)
file(GLOB S_CLASSES ${CLASSES_DIR}/${CLI}/*.cpp)

add_executable(${CLI} ${SRC}/${CLI}/main.cpp ${S_HEADERS} ${S_CLASSES} ${SHARED_CLASSES})

# Manager
file(GLOB M_HEADERS ${HEADERS_DIR}/${ADMIN}/*.h)
file(GLOB M_CLASSES ${CLASSES_DIR}/${ADMIN}/*.cpp)

add_executable(${ADMIN} ${SRC}/${ADMIN}/main.cpp ${M_HEADERS} ${M_CLASSES} ${SHARED_CLASSES})
#====================================
# Add external libraries
# Threads
find_package(Threads)
target_link_libraries(${CLI} Threads::Threads)
target_link_libraries(${ADMIN} Threads::Threads)


# Boost.Asio
find_package(Boost 1.71 REQUIRED COMPONENTS
            system)       
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${CLI} ${Boost_LIBRARIES} Threads::Threads)
target_link_libraries(${ADMIN} ${Boost_LIBRARIES} Threads::Threads)

# Ncurses
find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})
target_link_libraries(${ADMIN} ${CURSES_LIBRARY})