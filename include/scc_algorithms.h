#ifndef SCC_ALGORITHMS_H
#define SCC_ALGORITHMS_H

#include "scc.h"

#ifdef __cplusplus
extern "C" {
#endif

// Algorithm-specific state structures

// Tarjan's algorithm state
typedef struct tarjan_state {
    int* stack;
    int stack_top;
    int stack_capacity;
    int current_index;
    
    scc_result_t* result;
    int current_component;
    
    // Temporary arrays for algorithm state
    bool* vertices_processed;
} tarjan_state_t;

// Kosaraju's algorithm state  
typedef struct kosaraju_state {
    int* finish_order;
    int finish_index;
    int finish_capacity;
    graph_t* transpose_graph;
    
    scc_result_t* result;
    int current_component;
    
    // DFS state
    bool* visited_first_pass;
    bool* visited_second_pass;
} kosaraju_state_t;

// Algorithm state management
tarjan_state_t* tarjan_state_create(int num_vertices);
void tarjan_state_destroy(tarjan_state_t* state);

kosaraju_state_t* kosaraju_state_create(int num_vertices);
void kosaraju_state_destroy(kosaraju_state_t* state);

// Core algorithm implementations
scc_result_t* scc_tarjan_internal(const graph_t* graph, tarjan_state_t* state);
scc_result_t* scc_kosaraju_internal(const graph_t* graph, kosaraju_state_t* state);

// Algorithm-specific utility functions
void tarjan_dfs(const graph_t* graph, int vertex, tarjan_state_t* state);
void kosaraju_dfs_first(const graph_t* graph, int vertex, kosaraju_state_t* state);
void kosaraju_dfs_second(const graph_t* graph, int vertex, kosaraju_state_t* state);

// Stack operations for Tarjan
int tarjan_stack_push(tarjan_state_t* state, int vertex);
int tarjan_stack_pop(tarjan_state_t* state);
bool tarjan_stack_contains(const tarjan_state_t* state, int vertex);
bool tarjan_stack_is_empty(const tarjan_state_t* state);

// Incremental/Dynamic SCC support
typedef struct scc_incremental {
    graph_t* graph;
    scc_result_t* current_result;
    bool needs_recomputation;
    
    // Change tracking
    struct {
        int* added_edges_src;
        int* added_edges_dest;
        int num_added_edges;
        int capacity;
    } changes;
    
    // Algorithm preference
    enum {
        SCC_INCREMENTAL_TARJAN,
        SCC_INCREMENTAL_KOSARAJU,
        SCC_INCREMENTAL_AUTO
    } preferred_algorithm;
} scc_incremental_t;

// Incremental SCC functions
scc_incremental_t* scc_incremental_create(int initial_capacity);
void scc_incremental_destroy(scc_incremental_t* scc_inc);

int scc_incremental_add_edge(scc_incremental_t* scc_inc, int src, int dest);
int scc_incremental_remove_edge(scc_incremental_t* scc_inc, int src, int dest);
const scc_result_t* scc_incremental_get_result(scc_incremental_t* scc_inc);

void scc_incremental_force_recompute(scc_incremental_t* scc_inc);
bool scc_incremental_needs_update(const scc_incremental_t* scc_inc);

// Parallel SCC support (future extension)
#ifdef SCC_ENABLE_PARALLEL
typedef struct scc_parallel_config {
    int num_threads;
    int chunk_size;
    bool use_work_stealing;
} scc_parallel_config_t;

scc_result_t* scc_find_parallel(const graph_t* graph, const scc_parallel_config_t* config);
#endif

// Algorithm benchmarking and profiling
typedef struct scc_benchmark_result {
    double tarjan_time_ms;
    double kosaraju_time_ms;
    
    size_t tarjan_memory_peak_bytes;
    size_t kosaraju_memory_peak_bytes;
    
    int tarjan_stack_max_depth;
    int kosaraju_transpose_edges;
    
    bool results_match;  // Verify algorithms produce same result
} scc_benchmark_result_t;

scc_benchmark_result_t* scc_benchmark_algorithms(const graph_t* graph);
void scc_benchmark_result_destroy(scc_benchmark_result_t* benchmark);

// Algorithm selection heuristics
typedef enum {
    SCC_ALGORITHM_AUTO,
    SCC_ALGORITHM_TARJAN,
    SCC_ALGORITHM_KOSARAJU
} scc_algorithm_choice_t;

scc_algorithm_choice_t scc_recommend_algorithm(const graph_t* graph);
const char* scc_algorithm_name(scc_algorithm_choice_t algorithm);

#ifdef __cplusplus
}
#endif

#endif // SCC_ALGORITHMS_H