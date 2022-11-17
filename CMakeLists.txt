cmake_minimum_required(VERSION 3.19)

project("ElectricityNets" C)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# file search
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_precompile_headers(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# add_subdirectory(res)
# add_dependencies(${PROJECT_NAME} Resources)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION .)

set(METACOMPILER_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "")