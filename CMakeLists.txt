cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME text_processing_1)
project(${PROJECT_NAME}
        VERSION 0.0.1
        DESCRIPTION "botnet"
        HOMEPAGE_URL ""
        LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PROJECT_SOURCE_DECLARATION_DIRECTORY ${PROJECT_SOURCE_DIR}/inc)
set(PROJECT_SOURCE_DEFINITION_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set(CLASSES_DIR ${PROJECT_SOURCE_DEFINITION_DIRECTORY}/classes)

#====================================
# Project source compilation
include_directories(${PROJECT_SOURCE_DECLARATION_DIRECTORY})


#====================================
# Adding classes as shared libraries
set(CLASSES_LIST)

foreach(CLASS ${CLASSES_LIST})
    add_library(${CLASS} SHARED ${CLASSES_DIR}/${CLASS}.cpp)
    message("+- CLASS ('${CLASS}') WAS ADDED AS LIBRARY") 
endforeach()


# Add main src file
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DEFINITION_DIRECTORY}/main.cpp)

# Linking libraries to executable
FOREACH(CLASS ${CLASSES_LIST})
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${CLASS})
    MESSAGE("*- '${CLASS}' library WAS LINKED TO EXECUTABLE") 
ENDFOREACH()


#====================================
# Add external libraries



