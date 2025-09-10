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
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Compiler-specific flags
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wstrict-prototypes")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native -flto")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")

# Optional features
option(SCC_BUILD_SHARED "Build shared library" ON)
option(SCC_BUILD_STATIC "Build static library" ON)
option(SCC_BUILD_TESTS "Build test suite" ON)
option(SCC_BUILD_EXAMPLES "Build examples" ON)
option(SCC_BUILD_BENCHMARKS "Build benchmark suite" OFF)
option(SCC_ENABLE_PARALLEL "Enable parallel algorithms" OFF)
option(SCC_ENABLE_VISUALIZATION "Enable graph visualization" OFF)
option(SCC_ENABLE_PROFILING "Enable profiling support" OFF)

# Dependency checks
if(SCC_ENABLE_PARALLEL)
    find_package(OpenMP REQUIRED)
endif()

if(SCC_ENABLE_VISUALIZATION)
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(CAIRO cairo)
        pkg_check_modules(GRAPHVIZ libgvc)
    endif()
endif()

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

set(SCC_HEADERS
    include/scc.h
    include/graph.h
    include/scc_algorithms.h
)

# Optional sources
if(SCC_ENABLE_PARALLEL)
    list(APPEND SCC_SOURCES src/parallel.c)
    list(APPEND SCC_HEADERS include/scc_parallel.h)
endif()

if(SCC_ENABLE_VISUALIZATION)
    list(APPEND SCC_SOURCES src/visualize.c)
    list(APPEND SCC_HEADERS include/scc_visualize.h)
endif()

# Library targets
if(SCC_BUILD_SHARED)
    add_library(scc_shared SHARED ${SCC_SOURCES})
    set_target_properties(scc_shared PROPERTIES
        OUTPUT_NAME scc
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )
    
    target_include_directories(scc_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

if(SCC_BUILD_STATIC)
    add_library(scc_static STATIC ${SCC_SOURCES})
    set_target_properties(scc_static PROPERTIES
        OUTPUT_NAME scc
        POSITION_INDEPENDENT_CODE ON
    )
    
    target_include_directories(scc_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Create alias target for easier consumption
if(SCC_BUILD_SHARED)
    add_library(scc::scc ALIAS scc_shared)
    set(SCC_MAIN_TARGET scc_shared)
elseif(SCC_BUILD_STATIC)
    add_library(scc::scc ALIAS scc_static)  
    set(SCC_MAIN_TARGET scc_static)
endif()

# Apply common configurations to library targets
foreach(target IN LISTS SCC_MAIN_TARGET)
    if(TARGET ${target})
        # Compile definitions
        target_compile_definitions(${target} PRIVATE SCC_BUILDING)
        
        if(SCC_ENABLE_PARALLEL)
            target_link_libraries(${target} PUBLIC OpenMP::OpenMP_C)
            target_compile_definitions(${target} PUBLIC SCC_ENABLE_PARALLEL)
        endif()
        
        if(SCC_ENABLE_VISUALIZATION)
            if(CAIRO_FOUND)
                target_link_libraries(${target} PRIVATE ${CAIRO_LIBRARIES})
                target_include_directories(${target} PRIVATE ${CAIRO_INCLUDE_DIRS})
                target_compile_definitions(${target} PRIVATE SCC_ENABLE_CAIRO)
            endif()
            
            if(GRAPHVIZ_FOUND)
                target_link_libraries(${target} PRIVATE ${GRAPHVIZ_LIBRARIES})
                target_include_directories(${target} PRIVATE ${GRAPHVIZ_INCLUDE_DIRS})
                target_compile_definitions(${target} PRIVATE SCC_ENABLE_GRAPHVIZ)
            endif()
        endif()
        
        if(SCC_ENABLE_PROFILING)
            target_compile_definitions(${target} PRIVATE SCC_ENABLE_PROFILING)
        endif()
        
        # Platform-specific configurations
        if(WIN32)
            target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)
        endif()
        
        if(UNIX)
            target_link_libraries(${target} PRIVATE m) # Link math library
        endif()
    endif()
endforeach()

# Testing
if(SCC_BUILD_TESTS)
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
    add_test(NAME TarjanTests COMMAND scc_test tarjan)
    add_test(NAME KosarajuTests COMMAND scc_test kosaraju)
    add_test(NAME MemoryTests COMMAND scc_test memory)
    add_test(NAME UtilsTests COMMAND scc_test utils)
    add_test(NAME IOTests COMMAND scc_test io)
    add_test(NAME IntegrationTests COMMAND scc_test integration)
    add_test(NAME PerformanceTests COMMAND scc_test performance)
    
    # 테스트 타임아웃 설정
    set_tests_properties(AllTests PROPERTIES TIMEOUT 300)
    set_tests_properties(IntegrationTests PROPERTIES TIMEOUT 120)
    set_tests_properties(PerformanceTests PROPERTIES TIMEOUT 180)
    
    # 커스텀 타겟들
    add_custom_target(check
        COMMAND ${CMAKE_CTEST_COMMAND} --verbose
        DEPENDS scc_test
        COMMENT "Running all tests"
    )
    
    add_custom_target(check-quick
        COMMAND scc_test graph scc tarjan kosaraju memory
        DEPENDS scc_test
        COMMENT "Running core functionality tests"
    )
    
    add_custom_target(check-performance
        COMMAND scc_test performance
        DEPENDS scc_test
        COMMENT "Running performance benchmarks"
    )
endif()

# Examples
if(SCC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Benchmarks
if(SCC_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

# Installation
include(GNUInstallDirs)

# Install headers
install(FILES ${SCC_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install libraries
if(SCC_BUILD_SHARED)
    install(TARGETS scc_shared
            EXPORT scc-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(SCC_BUILD_STATIC)
    install(TARGETS scc_static
            EXPORT scc-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Install CMake config files
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/scc-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/scc-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scc
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scc-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/scc-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/scc-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scc)

install(EXPORT scc-targets
        FILE scc-targets.cmake
        NAMESPACE scc::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scc)

# Generate pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/scc.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/scc.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/scc.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# Documentation
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        @ONLY
    )
    
    add_custom_target(docs
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation"
    )
    
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html/
            DESTINATION ${CMAKE_INSTALL_DOCDIR}
            OPTIONAL)
endif()

# Uninstall target
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    COMMENT "Uninstalling SCC library"
)

# Summary
message(STATUS "SCC Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build shared: ${SCC_BUILD_SHARED}")
message(STATUS "  Build static: ${SCC_BUILD_STATIC}")
message(STATUS "  Build tests: ${SCC_BUILD_TESTS}")
message(STATUS "  Build examples: ${SCC_BUILD_EXAMPLES}")
message(STATUS "  Build benchmarks: ${SCC_BUILD_BENCHMARKS}")
message(STATUS "  Enable parallel: ${SCC_ENABLE_PARALLEL}")
message(STATUS "  Enable visualization: ${SCC_ENABLE_VISUALIZATION}")
message(STATUS "  Enable profiling: ${SCC_ENABLE_PROFILING}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")