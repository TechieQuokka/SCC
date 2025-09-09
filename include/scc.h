#ifndef SCC_H
#define SCC_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
typedef enum {
    SCC_SUCCESS = 0,
    SCC_ERROR_NULL_POINTER = -1,
    SCC_ERROR_INVALID_VERTEX = -2,
    SCC_ERROR_MEMORY_ALLOCATION = -3,
    SCC_ERROR_GRAPH_EMPTY = -4,
    SCC_ERROR_INVALID_PARAMETER = -5,
    SCC_ERROR_VERTEX_EXISTS = -6,
    SCC_ERROR_EDGE_EXISTS = -7
} scc_error_t;

// Forward declarations
typedef struct graph graph_t;
typedef struct scc_result scc_result_t;
typedef struct scc_component scc_component_t;

// Graph data structures
typedef struct edge {
    int dest;
    struct edge* next;
} edge_t;

typedef struct vertex {
    int id;
    edge_t* edges;
    int out_degree;
    
    // Algorithm-specific fields
    int index;
    int lowlink;
    bool on_stack;
    bool visited;
    
    // User data
    void* data;
} vertex_t;

typedef struct graph {
    vertex_t** vertices;
    int num_vertices;
    int num_edges;
    int capacity;
    
    // Memory management
    struct memory_pool* vertex_pool;
    struct memory_pool* edge_pool;
} graph_t;

// SCC result structures
typedef struct scc_component {
    int* vertices;
    int size;
    int capacity;
} scc_component_t;

typedef struct scc_result {
    scc_component_t* components;
    int num_components;
    int* vertex_to_component;
    
    // Statistics
    int largest_component_size;
    int smallest_component_size;
    double average_component_size;
} scc_result_t;

// Graph management functions
graph_t* graph_create(int initial_capacity);
void graph_destroy(graph_t* graph);
int graph_add_vertex(graph_t* graph);
int graph_add_edge(graph_t* graph, int src, int dest);
int graph_remove_edge(graph_t* graph, int src, int dest);
bool graph_has_edge(const graph_t* graph, int src, int dest);
int graph_get_out_degree(const graph_t* graph, int vertex);
int graph_get_vertex_count(const graph_t* graph);
int graph_get_edge_count(const graph_t* graph);

// SCC computation functions
scc_result_t* scc_find_tarjan(const graph_t* graph);
scc_result_t* scc_find_kosaraju(const graph_t* graph);
scc_result_t* scc_find(const graph_t* graph);  // Default algorithm

// Result management
void scc_result_destroy(scc_result_t* result);
scc_result_t* scc_result_copy(const scc_result_t* result);

// Result analysis functions
int scc_get_component_count(const scc_result_t* result);
int scc_get_component_size(const scc_result_t* result, int component_id);
int scc_get_vertex_component(const scc_result_t* result, int vertex);
const int* scc_get_component_vertices(const scc_result_t* result, int component_id);

// Graph property functions
bool scc_is_strongly_connected(const graph_t* graph);
graph_t* scc_build_condensation_graph(const graph_t* graph, const scc_result_t* scc);

// Utility functions
const char* scc_error_string(scc_error_t error);
int scc_get_last_error(void);
void scc_clear_error(void);

// Statistics functions
void scc_print_statistics(const scc_result_t* result);
void scc_print_components(const scc_result_t* result);

#ifdef __cplusplus
}
#endif

#endif // SCC_H