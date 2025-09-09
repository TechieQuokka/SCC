/**
 * @file algorithm_comparison.c
 * @brief Compare Tarjan vs Kosaraju algorithms
 * 
 * This example demonstrates:
 * - Performance comparison between algorithms
 * - Memory usage analysis
 * - Algorithm selection heuristics
 * - Correctness verification
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "scc.h"
#include "scc_algorithms.h"

// Generate a random graph for testing
graph_t* generate_test_graph(int num_vertices, int num_edges, unsigned int seed) {
    srand(seed);
    
    graph_t* graph = graph_create(num_vertices);
    if (!graph) return NULL;
    
    // Add vertices
    for (int i = 0; i < num_vertices; i++) {
        if (graph_add_vertex(graph) < 0) {
            graph_destroy(graph);
            return NULL;
        }
    }
    
    // Add random edges
    int edges_added = 0;
    int attempts = 0;
    const int max_attempts = num_edges * 10; // Prevent infinite loop
    
    while (edges_added < num_edges && attempts < max_attempts) {
        int src = rand() % num_vertices;
        int dest = rand() % num_vertices;
        
        if (src != dest && !graph_has_edge(graph, src, dest)) {
            if (graph_add_edge(graph, src, dest) == SCC_SUCCESS) {
                edges_added++;
            }
        }
        attempts++;
    }
    
    printf("Generated graph: %d vertices, %d edges (requested %d)\n", 
           graph_get_vertex_count(graph), graph_get_edge_count(graph), num_edges);
    
    return graph;
}

// Create a specific test case with known SCC structure
graph_t* create_structured_graph() {
    graph_t* graph = graph_create(20);
    if (!graph) return NULL;
    
    // Add 12 vertices
    for (int i = 0; i < 12; i++) {
        graph_add_vertex(graph);
    }
    
    // Create multiple SCCs with different sizes
    
    // SCC 1: Large cycle (vertices 0-5)
    for (int i = 0; i < 5; i++) {
        graph_add_edge(graph, i, i + 1);
    }
    graph_add_edge(graph, 5, 0); // Complete the cycle
    
    // SCC 2: Small cycle (vertices 6-7)
    graph_add_edge(graph, 6, 7);
    graph_add_edge(graph, 7, 6);
    
    // SCC 3: Single vertex (vertex 8)
    // No edges needed
    
    // SCC 4: Complex structure (vertices 9-11)
    graph_add_edge(graph, 9, 10);
    graph_add_edge(graph, 10, 11);
    graph_add_edge(graph, 11, 9);
    graph_add_edge(graph, 9, 11); // Additional edge
    
    // Inter-component edges
    graph_add_edge(graph, 2, 6);  // SCC1 -> SCC2
    graph_add_edge(graph, 7, 8);  // SCC2 -> SCC3
    graph_add_edge(graph, 8, 9);  // SCC3 -> SCC4
    
    return graph;
}

// Compare algorithm results for correctness
bool verify_results_match(const scc_result_t* result1, const scc_result_t* result2) {
    if (result1->num_components != result2->num_components) {
        printf("   Different number of components: %d vs %d\n", 
               result1->num_components, result2->num_components);
        return false;
    }
    
    // Check that each vertex maps to the same relative component
    // (component IDs might differ, but the partitioning should be identical)
    for (int v = 0; v < result1->num_components; v++) {
        int comp1 = result1->vertex_to_component[v];
        int comp2 = result2->vertex_to_component[v];
        
        // Find a representative vertex in the same component as v in result1
        int representative = -1;
        for (int u = 0; u < v; u++) {
            if (result1->vertex_to_component[u] == comp1) {
                representative = u;
                break;
            }
        }
        
        if (representative != -1) {
            // Check if the representative is in the same component in result2
            if (result2->vertex_to_component[representative] != comp2) {
                printf("   Partitioning mismatch for vertices %d and %d\n", v, representative);
                return false;
            }
        }
    }
    
    return true;
}

// Measure execution time
double measure_algorithm_time(graph_t* graph, scc_result_t* (*algorithm)(const graph_t*)) {
    clock_t start = clock();
    scc_result_t* result = algorithm(graph);
    clock_t end = clock();
    
    if (result) {
        scc_result_destroy(result);
        return ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0; // milliseconds
    }
    
    return -1.0; // Error
}

int main() {
    printf("=== SCC Algorithm Comparison Example ===\n\n");
    
    // Test 1: Structured graph with known properties
    printf("1. Testing structured graph with known SCCs...\n");
    graph_t* structured_graph = create_structured_graph();
    if (!structured_graph) {
        fprintf(stderr, "Error: Failed to create structured graph\n");
        return EXIT_FAILURE;
    }
    
    printf("   Graph: %d vertices, %d edges\n", 
           graph_get_vertex_count(structured_graph), 
           graph_get_edge_count(structured_graph));
    
    // Run both algorithms
    printf("   Running Tarjan's algorithm...\n");
    scc_result_t* tarjan_result = scc_find_tarjan(structured_graph);
    
    printf("   Running Kosaraju's algorithm...\n");
    scc_result_t* kosaraju_result = scc_find_kosaraju(structured_graph);
    
    if (!tarjan_result || !kosaraju_result) {
        fprintf(stderr, "Error: Algorithm execution failed\n");
        if (tarjan_result) scc_result_destroy(tarjan_result);
        if (kosaraju_result) scc_result_destroy(kosaraju_result);
        graph_destroy(structured_graph);
        return EXIT_FAILURE;
    }
    
    // Verify correctness
    printf("   Verifying results match...\n");
    bool results_match = verify_results_match(tarjan_result, kosaraju_result);
    printf("   Results match: %s\n", results_match ? "YES" : "NO");
    
    printf("   Tarjan found %d components\n", tarjan_result->num_components);
    printf("   Kosaraju found %d components\n", kosaraju_result->num_components);
    
    // Display components from Tarjan (as reference)
    printf("   Component breakdown (Tarjan):\n");
    for (int i = 0; i < tarjan_result->num_components; i++) {
        int size = scc_get_component_size(tarjan_result, i);
        const int* vertices = scc_get_component_vertices(tarjan_result, i);
        
        printf("     Component %d: {", i);
        for (int j = 0; j < size; j++) {
            printf("%d", vertices[j]);
            if (j < size - 1) printf(", ");
        }
        printf("} (size: %d)\n", size);
    }
    
    scc_result_destroy(tarjan_result);
    scc_result_destroy(kosaraju_result);
    graph_destroy(structured_graph);
    
    // Test 2: Performance comparison on larger graphs
    printf("\n2. Performance comparison on random graphs...\n");
    
    struct {
        int vertices;
        int edges;
        const char* description;
    } test_cases[] = {
        {100, 200, "Small sparse graph"},
        {1000, 2000, "Medium sparse graph"},
        {1000, 5000, "Medium dense graph"},
        {5000, 10000, "Large sparse graph"}
    };
    
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("   %-25s %10s %10s %10s %s\n", 
           "Graph Type", "Vertices", "Edges", "Tarjan(ms)", "Kosaraju(ms)");
    printf("   %s\n", "------------------------------------------------------------");
    
    for (int i = 0; i < num_test_cases; i++) {
        printf("   %-25s %10d %10d ", 
               test_cases[i].description,
               test_cases[i].vertices, 
               test_cases[i].edges);
        
        // Generate test graph
        graph_t* test_graph = generate_test_graph(test_cases[i].vertices, 
                                                 test_cases[i].edges, 
                                                 42 + i); // Deterministic seed
        
        if (!test_graph) {
            printf("ERROR (graph creation failed)\n");
            continue;
        }
        
        // Measure Tarjan's algorithm
        double tarjan_time = measure_algorithm_time(test_graph, scc_find_tarjan);
        
        // Measure Kosaraju's algorithm  
        double kosaraju_time = measure_algorithm_time(test_graph, scc_find_kosaraju);
        
        if (tarjan_time >= 0 && kosaraju_time >= 0) {
            printf("%8.2f %10.2f", tarjan_time, kosaraju_time);
            
            if (tarjan_time < kosaraju_time) {
                printf(" (T faster)");
            } else if (kosaraju_time < tarjan_time) {
                printf(" (K faster)");
            } else {
                printf(" (tie)");
            }
        } else {
            printf("ERROR (execution failed)");
        }
        
        printf("\n");
        graph_destroy(test_graph);
    }
    
    // Test 3: Algorithm recommendation system
    printf("\n3. Algorithm recommendation system...\n");
    
    for (int i = 0; i < num_test_cases; i++) {
        graph_t* test_graph = generate_test_graph(test_cases[i].vertices,
                                                 test_cases[i].edges,
                                                 123 + i);
        if (!test_graph) continue;
        
        scc_algorithm_choice_t recommended = scc_recommend_algorithm(test_graph);
        
        printf("   %s (%dV, %dE): Recommended %s\n",
               test_cases[i].description,
               graph_get_vertex_count(test_graph),
               graph_get_edge_count(test_graph),
               scc_algorithm_name(recommended));
        
        graph_destroy(test_graph);
    }
    
    // Test 4: Full benchmark suite
    printf("\n4. Detailed benchmark analysis...\n");
    graph_t* benchmark_graph = generate_test_graph(2000, 5000, 999);
    if (benchmark_graph) {
        scc_benchmark_result_t* benchmark = scc_benchmark_algorithms(benchmark_graph);
        
        if (benchmark) {
            printf("   Comprehensive benchmark results:\n");
            printf("   %-20s %10.2f ms\n", "Tarjan time:", benchmark->tarjan_time_ms);
            printf("   %-20s %10.2f ms\n", "Kosaraju time:", benchmark->kosaraju_time_ms);
            printf("   %-20s %10zu bytes\n", "Tarjan memory:", benchmark->tarjan_memory_peak_bytes);
            printf("   %-20s %10zu bytes\n", "Kosaraju memory:", benchmark->kosaraju_memory_peak_bytes);
            printf("   %-20s %10d\n", "Tarjan stack depth:", benchmark->tarjan_stack_max_depth);
            printf("   %-20s %10d\n", "Transpose edges:", benchmark->kosaraju_transpose_edges);
            printf("   %-20s %10s\n", "Results match:", benchmark->results_match ? "YES" : "NO");
            
            scc_benchmark_result_destroy(benchmark);
        }
        
        graph_destroy(benchmark_graph);
    }
    
    printf("\n=== Algorithm comparison completed ===\n");
    return EXIT_SUCCESS;
}