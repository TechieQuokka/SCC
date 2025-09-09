# Examples CMake configuration
cmake_minimum_required(VERSION 3.15)

# Find required dependencies
find_package(scc REQUIRED)

# Compiler settings for examples
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Example executables
set(EXAMPLES
    basic_usage
    algorithm_comparison
    memory_management
    graph_io
    incremental_scc
)

# Create executable for each example
foreach(EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE} ${EXAMPLE}.c)
    target_link_libraries(${EXAMPLE} PRIVATE scc)
    
    # Set output directory
    set_target_properties(${EXAMPLE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
    
    # Add to install target
    install(TARGETS ${EXAMPLE}
        RUNTIME DESTINATION bin/examples
    )
endforeach()

# Copy example data files if they exist
file(GLOB EXAMPLE_DATA_FILES "data/*")
if(EXAMPLE_DATA_FILES)
    file(COPY ${EXAMPLE_DATA_FILES} 
         DESTINATION ${CMAKE_BINARY_DIR}/examples/data)
    
    install(FILES ${EXAMPLE_DATA_FILES}
            DESTINATION bin/examples/data)
endif()

# Add custom target to run all examples
add_custom_target(run_examples
    COMMENT "Running all SCC examples"
)

foreach(EXAMPLE ${EXAMPLES})
    add_custom_target(run_${EXAMPLE}
        COMMAND ${EXAMPLE}
        DEPENDS ${EXAMPLE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/examples
        COMMENT "Running ${EXAMPLE}"
    )
    add_dependencies(run_examples run_${EXAMPLE})
endforeach()

# Example-specific configurations
if(TARGET memory_management)
    # Enable additional debugging for memory example
    target_compile_definitions(memory_management PRIVATE SCC_DEBUG_MEMORY)
endif()

if(TARGET graph_io AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data")
    # Add data directory for graph I/O example
    target_compile_definitions(graph_io PRIVATE 
        EXAMPLE_DATA_DIR="${CMAKE_BINARY_DIR}/examples/data")
endif()