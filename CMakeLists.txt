cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME botnet)
project(${PROJECT_NAME}
        VERSION 0.0.1
        DESCRIPTION "Botnet"
        HOMEPAGE_URL "https://github.com/Oleksandra2020/botnet"
        LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(HEADERS ${CMAKE_SOURCE_DIR}/inc)
set(SRC ${CMAKE_SOURCE_DIR}/src)
set(CLASSES_DIR ${SRC}/classes)

#====================================
# Project source compilation
include_directories(${HEADERS})


#====================================
# Adding classes as shared libraries
set(CLASSES_LIST session server)

# Create classes (including headers & cpp files) (if needed, using bash script)
execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/scripts/add_class.sh ${CLASSES_LIST} OUTPUT_VARIABLE add_classes)
message("${add_classes}")

# foreach(CLASS ${CLASSES_LIST})
#     add_library(${CLASS} SHARED ${CLASSES_DIR}/${CLASS}.cpp)
#     message("+- CLASS ('${CLASS}') WAS ADDED AS LIBRARY") 
# endforeach()

# Add main src file
# add_executable(${PROJECT_NAME} ${SRC}/main.cpp)

add_executable(${PROJECT_NAME} ${SRC}/main.cpp 
                ${CLASSES_DIR}/session.cpp 
                ${CLASSES_DIR}/server.cpp)

# Linking libraries to executable
# FOREACH(CLASS ${CLASSES_LIST})
#     TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${CLASS})
#     MESSAGE("*- '${CLASS}' library WAS LINKED TO EXECUTABLE") 
# ENDFOREACH()


#====================================
# Add external libraries

# Threads
find_package(Threads)
target_link_libraries(${PROJECT_NAME} Threads::Threads)

# Boost.Asio
find_package(Boost 1.71 REQUIRED COMPONENTS
            system)       
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})


