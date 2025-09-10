cmake_minimum_required(VERSION 3.15)
project(SCC 
    VERSION 1.0.0 
    DESCRIPTION "High-performance Strongly Connected Components library"
    LANGUAGES C
)

# Project configuration
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type" FORCE)
endif()

# Compiler-specific flags
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SCC_SOURCES
    src/scc.c
    src/graph.c  
    src/tarjan.c
    src/kosaraju.c
    src/memory.c
    src/utils.c
    src/graph_io.c
)

# Library targets
add_library(scc_static STATIC ${SCC_SOURCES})
set_target_properties(scc_static PROPERTIES OUTPUT_NAME scc)

target_include_directories(scc_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create alias target
add_library(scc::scc ALIAS scc_static)
set(SCC_MAIN_TARGET scc_static)

# Platform-specific configurations
if(WIN32)
    target_compile_definitions(${SCC_MAIN_TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if(UNIX)
    target_link_libraries(${SCC_MAIN_TARGET} PRIVATE m)
endif()

# Testing
enable_testing()

# 테스트 소스 파일들
set(TEST_SOURCES
    tests/test_framework.c
    tests/test_graph.c
    tests/test_scc.c
    tests/test_tarjan.c
    tests/test_kosaraju.c
    tests/test_memory.c
    tests/test_utils.c
    tests/test_io.c
    tests/test_integration.c
    tests/test_performance.c
    tests/test_main.c
)

# 테스트 실행 파일
add_executable(scc_test ${TEST_SOURCES})
target_link_libraries(scc_test ${SCC_MAIN_TARGET})
target_include_directories(scc_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 수학 라이브러리 링크 (성능 테스트용)
if(UNIX)
    target_link_libraries(scc_test m)
endif()

# CTest 테스트들 정의
add_test(NAME AllTests COMMAND scc_test)
add_test(NAME GraphTests COMMAND scc_test graph)
add_test(NAME SCCTests COMMAND scc_test scc)

# 커스텀 타겟
add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS scc_test
    COMMENT "Running all tests"
)

# Summary
message(STATUS "SCC Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Tests enabled: ON")