/**
 * @file basic_usage.c
 * @brief Basic SCC library usage example
 * 
 * This example demonstrates fundamental SCC operations:
 * - Graph creation and edge addition
 * - SCC computation using default algorithm
 * - Result processing and display
 */

#include <stdio.h>
#include <stdlib.h>
#include "scc.h"

int main() {
    printf("=== Basic SCC Library Usage Example ===\n\n");
    
    // Create a graph with initial capacity for 10 vertices
    graph_t* graph = graph_create(10);
    if (!graph) {
        fprintf(stderr, "Error: Failed to create graph\n");
        return EXIT_FAILURE;
    }
    
    printf("1. Creating graph with vertices and edges...\n");
    
    // Add 6 vertices to the graph
    for (int i = 0; i < 6; i++) {
        int vertex_id = graph_add_vertex(graph);
        if (vertex_id < 0) {
            fprintf(stderr, "Error: Failed to add vertex %d\n", i);
            graph_destroy(graph);
            return EXIT_FAILURE;
        }
        printf("   Added vertex %d\n", vertex_id);
    }
    
    // Define edges for a test graph with multiple SCCs
    // This creates: SCC1: {0,1,2}, SCC2: {3,4}, SCC3: {5}
    int edges[][2] = {
        {0, 1}, {1, 2}, {2, 0},  // First SCC (cycle)
        {2, 3},                  // Connection between SCCs
        {3, 4}, {4, 3},          // Second SCC (2-cycle)  
        {4, 5}                   // Connection to singleton SCC
    };
    int num_edges = sizeof(edges) / sizeof(edges[0]);
    
    printf("   Adding %d edges:\n", num_edges);
    for (int i = 0; i < num_edges; i++) {
        int src = edges[i][0];
        int dest = edges[i][1];
        
        int result = graph_add_edge(graph, src, dest);
        if (result != SCC_SUCCESS) {
            fprintf(stderr, "Error: Failed to add edge %d->%d: %s\n", 
                   src, dest, scc_error_string(result));
            graph_destroy(graph);
            return EXIT_FAILURE;
        }
        printf("     %d -> %d\n", src, dest);
    }
    
    printf("\n2. Graph statistics:\n");
    printf("   Vertices: %d\n", graph_get_vertex_count(graph));
    printf("   Edges: %d\n", graph_get_edge_count(graph));
    
    // Compute strongly connected components
    printf("\n3. Computing strongly connected components...\n");
    scc_result_t* result = scc_find(graph);
    if (!result) {
        fprintf(stderr, "Error: SCC computation failed: %s\n", 
               scc_error_string(scc_get_last_error()));
        graph_destroy(graph);
        return EXIT_FAILURE;
    }
    
    // Display results
    printf("   Algorithm completed successfully!\n");
    printf("   Found %d strongly connected components\n\n", 
           scc_get_component_count(result));
    
    printf("4. Component details:\n");
    for (int i = 0; i < scc_get_component_count(result); i++) {
        int size = scc_get_component_size(result, i);
        const int* vertices = scc_get_component_vertices(result, i);
        
        printf("   Component %d (%d vertices): {", i, size);
        for (int j = 0; j < size; j++) {
            printf("%d", vertices[j]);
            if (j < size - 1) printf(", ");
        }
        printf("}\n");
    }
    
    printf("\n5. Vertex-to-component mapping:\n");
    for (int v = 0; v < graph_get_vertex_count(graph); v++) {
        int component = scc_get_vertex_component(result, v);
        printf("   Vertex %d belongs to component %d\n", v, component);
    }
    
    // Display statistics
    printf("\n6. Statistical summary:\n");
    scc_print_statistics(result);
    
    // Test graph properties
    printf("\n7. Graph properties:\n");
    bool is_strongly_connected = scc_is_strongly_connected(graph);
    printf("   Is strongly connected: %s\n", 
           is_strongly_connected ? "Yes" : "No");
    
    // Clean up resources
    printf("\n8. Cleaning up resources...\n");
    scc_result_destroy(result);
    graph_destroy(graph);
    
    printf("   Done!\n\n");
    printf("=== Example completed successfully ===\n");
    
    return EXIT_SUCCESS;
}