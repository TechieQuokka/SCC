#include "scc.h"
#include "scc_algorithms.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

// 그래프 순회 함수들
void graph_dfs(const graph_t* graph, int start_vertex, 
               vertex_visit_func_t visit_func, void* user_data) {
    if (!graph || !visit_func || start_vertex < 0 || 
        start_vertex >= graph_get_vertex_count(graph)) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return;
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    bool* visited = calloc(num_vertices, sizeof(bool));
    if (!visited) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return;
    }
    
    // DFS 스택 (재귀 대신 명시적 스택 사용)
    int* stack = malloc(num_vertices * sizeof(int));
    if (!stack) {
        free(visited);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return;
    }
    
    int stack_top = 0;
    stack[stack_top++] = start_vertex;
    
    while (stack_top > 0) {
        int current = stack[--stack_top];
        
        if (!visited[current]) {
            visited[current] = true;
            visit_func(current, user_data);
            
            // 모든 인접 정점을 스택에 추가
            vertex_t* vertex = graph->vertices[current];
            edge_t* edge = vertex->edges;
            while (edge) {
                if (!visited[edge->dest]) {
                    stack[stack_top++] = edge->dest;
                }
                edge = edge->next;
            }
        }
    }
    
    free(stack);
    free(visited);
}

void graph_bfs(const graph_t* graph, int start_vertex,
               vertex_visit_func_t visit_func, void* user_data) {
    if (!graph || !visit_func || start_vertex < 0 || 
        start_vertex >= graph_get_vertex_count(graph)) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return;
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    bool* visited = calloc(num_vertices, sizeof(bool));
    if (!visited) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return;
    }
    
    // BFS 큐
    int* queue = malloc(num_vertices * sizeof(int));
    if (!queue) {
        free(visited);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return;
    }
    
    int front = 0, rear = 0;
    queue[rear++] = start_vertex;
    visited[start_vertex] = true;
    
    while (front < rear) {
        int current = queue[front++];
        visit_func(current, user_data);
        
        // 모든 인접 정점을 큐에 추가
        vertex_t* vertex = graph->vertices[current];
        edge_t* edge = vertex->edges;
        while (edge) {
            if (!visited[edge->dest]) {
                visited[edge->dest] = true;
                queue[rear++] = edge->dest;
            }
            edge = edge->next;
        }
    }
    
    free(queue);
    free(visited);
}

// 그래프 검증 함수
int graph_verify_integrity(const graph_t* graph) {
    if (!graph) {
        return SCC_ERROR_NULL_POINTER;
    }
    
    if (!graph_is_valid(graph)) {
        return SCC_ERROR_INVALID_PARAMETER;
    }
    
    // 추가적인 무결성 검사들
    int calculated_edges = 0;
    
    for (int i = 0; i < graph->num_vertices; i++) {
        vertex_t* vertex = graph->vertices[i];
        
        // 정점 ID 검사
        if (vertex->id != i) {
            return SCC_ERROR_INVALID_VERTEX;
        }
        
        // 간선 유효성 검사
        int edge_count = 0;
        edge_t* edge = vertex->edges;
        while (edge) {
            if (edge->dest < 0 || edge->dest >= graph->num_vertices) {
                return SCC_ERROR_INVALID_VERTEX;
            }
            edge_count++;
            calculated_edges++;
            edge = edge->next;
        }
        
        if (vertex->out_degree != edge_count) {
            return SCC_ERROR_INVALID_PARAMETER;
        }
    }
    
    if (calculated_edges != graph->num_edges) {
        return SCC_ERROR_INVALID_PARAMETER;
    }
    
    return SCC_SUCCESS;
}

// 간선 반복자 구현
graph_edge_iterator_t* graph_edge_iterator_create(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    graph_edge_iterator_t* iter = malloc(sizeof(graph_edge_iterator_t));
    if (!iter) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    iter->graph = graph;
    iter->current_vertex = 0;
    iter->current_edge = NULL;
    
    // 첫 번째 간선 찾기
    while (iter->current_vertex < graph->num_vertices) {
        if (graph->vertices[iter->current_vertex]->edges) {
            iter->current_edge = graph->vertices[iter->current_vertex]->edges;
            break;
        }
        iter->current_vertex++;
    }
    
    return iter;
}

void graph_edge_iterator_destroy(graph_edge_iterator_t* iter) {
    free(iter);
}

bool graph_edge_iterator_next(graph_edge_iterator_t* iter, int* src, int* dest) {
    if (!iter || !src || !dest) {
        return false;
    }
    
    if (iter->current_vertex >= iter->graph->num_vertices || !iter->current_edge) {
        return false;
    }
    
    *src = iter->current_vertex;
    *dest = iter->current_edge->dest;
    
    // 다음 간선으로 이동
    iter->current_edge = iter->current_edge->next;
    
    // 현재 정점의 간선이 끝나면 다음 정점으로
    while (!iter->current_edge && iter->current_vertex < iter->graph->num_vertices - 1) {
        iter->current_vertex++;
        if (iter->current_vertex < iter->graph->num_vertices) {
            iter->current_edge = iter->graph->vertices[iter->current_vertex]->edges;
        }
    }
    
    return true;
}

void graph_edge_iterator_reset(graph_edge_iterator_t* iter) {
    if (!iter) return;
    
    iter->current_vertex = 0;
    iter->current_edge = NULL;
    
    // 첫 번째 간선 찾기
    while (iter->current_vertex < iter->graph->num_vertices) {
        if (iter->graph->vertices[iter->current_vertex]->edges) {
            iter->current_edge = iter->graph->vertices[iter->current_vertex]->edges;
            break;
        }
        iter->current_vertex++;
    }
}

// 그래프 리사이징
int graph_resize(graph_t* graph, int new_capacity) {
    if (!graph || new_capacity < graph->num_vertices) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return SCC_ERROR_INVALID_PARAMETER;
    }
    
    if (new_capacity == graph->capacity) {
        return SCC_SUCCESS;
    }
    
    vertex_t** new_vertices = realloc(graph->vertices, 
                                     new_capacity * sizeof(vertex_t*));
    if (!new_vertices) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return SCC_ERROR_MEMORY_ALLOCATION;
    }
    
    // 새로운 공간을 NULL로 초기화
    for (int i = graph->capacity; i < new_capacity; i++) {
        new_vertices[i] = NULL;
    }
    
    graph->vertices = new_vertices;
    graph->capacity = new_capacity;
    
    return SCC_SUCCESS;
}

// 벤치마킹 함수
scc_benchmark_result_t* scc_benchmark_algorithms(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    scc_benchmark_result_t* benchmark = malloc(sizeof(scc_benchmark_result_t));
    if (!benchmark) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    // 초기화
    benchmark->tarjan_time_ms = 0.0;
    benchmark->kosaraju_time_ms = 0.0;
    benchmark->tarjan_memory_peak_bytes = 0;
    benchmark->kosaraju_memory_peak_bytes = 0;
    benchmark->tarjan_stack_max_depth = 0;
    benchmark->kosaraju_transpose_edges = graph_get_edge_count(graph);
    benchmark->results_match = true;
    
    clock_t start, end;
    
    // Tarjan 알고리즘 벤치마크
    start = clock();
    scc_result_t* tarjan_result = scc_find_tarjan(graph);
    end = clock();
    
    if (tarjan_result) {
        benchmark->tarjan_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        
        // 메모리 사용량 추정 (정확한 측정은 복잡하므로 근사치 사용)
        int num_vertices = graph_get_vertex_count(graph);
        benchmark->tarjan_memory_peak_bytes = 
            sizeof(tarjan_state_t) + 
            num_vertices * sizeof(int) + // 스택
            num_vertices * sizeof(bool) + // vertices_processed
            sizeof(scc_result_t) +
            tarjan_result->num_components * sizeof(scc_component_t);
    }
    
    // Kosaraju 알고리즘 벤치마크
    start = clock();
    scc_result_t* kosaraju_result = scc_find_kosaraju(graph);
    end = clock();
    
    if (kosaraju_result) {
        benchmark->kosaraju_time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
        
        // 메모리 사용량 추정
        int num_vertices = graph_get_vertex_count(graph);
        int num_edges = graph_get_edge_count(graph);
        benchmark->kosaraju_memory_peak_bytes = 
            sizeof(kosaraju_state_t) +
            num_vertices * sizeof(int) + // finish_order
            2 * num_vertices * sizeof(bool) + // visited arrays
            sizeof(graph_t) + num_vertices * sizeof(vertex_t*) + // transpose graph
            num_edges * sizeof(edge_t) + // transpose edges
            sizeof(scc_result_t) +
            kosaraju_result->num_components * sizeof(scc_component_t);
    }
    
    // 결과 비교
    if (tarjan_result && kosaraju_result) {
        benchmark->results_match = (tarjan_result->num_components == kosaraju_result->num_components);
        
        // 더 자세한 비교 (선택사항)
        if (benchmark->results_match) {
            // 컴포넌트 크기별로 정렬하여 비교할 수 있지만, 
            // 여기서는 간단히 컴포넌트 수만 비교
        }
    }
    
    // 정리
    scc_result_destroy(tarjan_result);
    scc_result_destroy(kosaraju_result);
    
    return benchmark;
}

void scc_benchmark_result_destroy(scc_benchmark_result_t* benchmark) {
    free(benchmark);
}