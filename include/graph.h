#ifndef GRAPH_H
#define GRAPH_H

#include "scc.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Memory pool for efficient allocation
typedef struct memory_block {
    void* data;
    size_t size;
    bool is_free;
    struct memory_block* next;
} memory_block_t;

typedef struct memory_pool {
    memory_block_t* blocks;
    size_t block_size;
    size_t total_allocated;
    size_t total_used;
    size_t alignment;
} memory_pool_t;

// Graph creation with custom memory pools
graph_t* graph_create_with_pools(int initial_capacity, 
                                 memory_pool_t* vertex_pool,
                                 memory_pool_t* edge_pool);

// Memory pool functions
memory_pool_t* memory_pool_create(size_t block_size, size_t alignment);
void memory_pool_destroy(memory_pool_t* pool);
void* memory_pool_alloc(memory_pool_t* pool, size_t size);
void memory_pool_free(memory_pool_t* pool, void* ptr);
void memory_pool_reset(memory_pool_t* pool);

// Advanced graph operations
int graph_resize(graph_t* graph, int new_capacity);
graph_t* graph_copy(const graph_t* graph);
graph_t* graph_transpose(const graph_t* graph);

// Graph I/O functions
typedef enum {
    GRAPH_FORMAT_EDGE_LIST,
    GRAPH_FORMAT_ADJACENCY_LIST,
    GRAPH_FORMAT_MATRIX,
    GRAPH_FORMAT_DOT
} graph_format_t;

int graph_load_from_file(graph_t** graph, const char* filename, graph_format_t format);
int graph_save_to_file(const graph_t* graph, const char* filename, graph_format_t format);

// Graph traversal utilities
typedef void (*vertex_visit_func_t)(int vertex, void* user_data);
typedef bool (*edge_visit_func_t)(int src, int dest, void* user_data);

void graph_dfs(const graph_t* graph, int start_vertex, 
               vertex_visit_func_t visit_func, void* user_data);
void graph_bfs(const graph_t* graph, int start_vertex,
               vertex_visit_func_t visit_func, void* user_data);

// Iterator interface
typedef struct graph_edge_iterator {
    const graph_t* graph;
    int current_vertex;
    edge_t* current_edge;
} graph_edge_iterator_t;

graph_edge_iterator_t* graph_edge_iterator_create(const graph_t* graph);
void graph_edge_iterator_destroy(graph_edge_iterator_t* iter);
bool graph_edge_iterator_next(graph_edge_iterator_t* iter, int* src, int* dest);
void graph_edge_iterator_reset(graph_edge_iterator_t* iter);

// Vertex data management
int graph_set_vertex_data(graph_t* graph, int vertex, void* data);
void* graph_get_vertex_data(const graph_t* graph, int vertex);

// Graph validation and debugging
bool graph_is_valid(const graph_t* graph);
void graph_print_debug(const graph_t* graph);
int graph_verify_integrity(const graph_t* graph);

#ifdef __cplusplus
}
#endif

#endif // GRAPH_H