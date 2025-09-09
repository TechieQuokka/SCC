#include "scc_algorithms.h"
#include "scc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 내부 헬퍼 함수들
static void kosaraju_dfs_first_recursive(const graph_t* graph, int vertex, kosaraju_state_t* state);
static void kosaraju_dfs_second_recursive(const graph_t* graph, int vertex, kosaraju_state_t* state);

// Kosaraju 상태 관리
kosaraju_state_t* kosaraju_state_create(int num_vertices) {
    if (num_vertices <= 0) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    kosaraju_state_t* state = malloc(sizeof(kosaraju_state_t));
    if (!state) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    // 완료 순서 배열 초기화
    state->finish_capacity = num_vertices;
    state->finish_order = malloc(state->finish_capacity * sizeof(int));
    if (!state->finish_order) {
        free(state);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    state->finish_index = 0;
    state->transpose_graph = NULL;
    state->current_component = 0;
    
    // 방문 상태 배열들
    state->visited_first_pass = calloc(num_vertices, sizeof(bool));
    state->visited_second_pass = calloc(num_vertices, sizeof(bool));
    if (!state->visited_first_pass || !state->visited_second_pass) {
        free(state->visited_second_pass);
        free(state->visited_first_pass);
        free(state->finish_order);
        free(state);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    // 결과 구조 초기화
    state->result = malloc(sizeof(scc_result_t));
    if (!state->result) {
        free(state->visited_second_pass);
        free(state->visited_first_pass);
        free(state->finish_order);
        free(state);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    state->result->components = malloc(num_vertices * sizeof(scc_component_t));
    state->result->vertex_to_component = malloc(num_vertices * sizeof(int));
    if (!state->result->components || !state->result->vertex_to_component) {
        free(state->result->vertex_to_component);
        free(state->result->components);
        free(state->result);
        free(state->visited_second_pass);
        free(state->visited_first_pass);
        free(state->finish_order);
        free(state);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    // 컴포넌트 초기화
    for (int i = 0; i < num_vertices; i++) {
        state->result->components[i].vertices = malloc(num_vertices * sizeof(int));
        if (!state->result->components[i].vertices) {
            // 이전에 할당된 컴포넌트들 정리
            for (int j = 0; j < i; j++) {
                free(state->result->components[j].vertices);
            }
            free(state->result->vertex_to_component);
            free(state->result->components);
            free(state->result);
            free(state->visited_second_pass);
            free(state->visited_first_pass);
            free(state->finish_order);
            free(state);
            scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
            return NULL;
        }
        state->result->components[i].size = 0;
        state->result->components[i].capacity = num_vertices;
        state->result->vertex_to_component[i] = -1;
    }
    
    state->result->num_components = 0;
    
    return state;
}

void kosaraju_state_destroy(kosaraju_state_t* state) {
    if (!state) return;
    
    if (state->result) {
        if (state->result->components) {
            for (int i = 0; i < state->result->num_components; i++) {
                free(state->result->components[i].vertices);
            }
            free(state->result->components);
        }
        free(state->result->vertex_to_component);
        free(state->result);
    }
    
    if (state->transpose_graph) {
        graph_destroy(state->transpose_graph);
    }
    
    free(state->visited_second_pass);
    free(state->visited_first_pass);
    free(state->finish_order);
    free(state);
}

// Kosaraju 알고리즘 메인 구현
scc_result_t* scc_kosaraju_internal(const graph_t* graph, kosaraju_state_t* state) {
    if (!graph || !state) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    if (num_vertices <= 0) {
        scc_set_error(SCC_ERROR_GRAPH_EMPTY);
        return NULL;
    }
    
    // 1단계: 원본 그래프에서 첫 번째 DFS 수행하여 완료 순서 계산
    for (int i = 0; i < num_vertices; i++) {
        if (!state->visited_first_pass[i]) {
            kosaraju_dfs_first_recursive(graph, i, state);
        }
    }
    
    // 2단계: 전치 그래프 생성
    state->transpose_graph = graph_transpose(graph);
    if (!state->transpose_graph) {
        return NULL;
    }
    
    // 3단계: 전치 그래프에서 완료 순서의 역순으로 두 번째 DFS 수행
    for (int i = state->finish_index - 1; i >= 0; i--) {
        int vertex = state->finish_order[i];
        if (!state->visited_second_pass[vertex]) {
            kosaraju_dfs_second_recursive(state->transpose_graph, vertex, state);
            state->current_component++;
            state->result->num_components++;
        }
    }
    
    // 통계 계산
    int largest = 0, smallest = num_vertices + 1;
    int total_vertices = 0;
    
    for (int i = 0; i < state->result->num_components; i++) {
        int size = state->result->components[i].size;
        if (size > largest) largest = size;
        if (size < smallest) smallest = size;
        total_vertices += size;
    }
    
    state->result->largest_component_size = largest;
    state->result->smallest_component_size = (state->result->num_components > 0) ? smallest : 0;
    state->result->average_component_size = (state->result->num_components > 0) ? 
        (double)total_vertices / state->result->num_components : 0.0;
    
    // 결과 반환 (상태에서 분리하여 반환)
    scc_result_t* result = state->result;
    state->result = NULL; // 이중 해제 방지
    
    return result;
}

void kosaraju_dfs_first(const graph_t* graph, int vertex, kosaraju_state_t* state) {
    kosaraju_dfs_first_recursive(graph, vertex, state);
}

void kosaraju_dfs_second(const graph_t* graph, int vertex, kosaraju_state_t* state) {
    kosaraju_dfs_second_recursive(graph, vertex, state);
}

// 공개 API 함수
scc_result_t* scc_find_kosaraju(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    if (num_vertices <= 0) {
        scc_set_error(SCC_ERROR_GRAPH_EMPTY);
        return NULL;
    }
    
    kosaraju_state_t* state = kosaraju_state_create(num_vertices);
    if (!state) {
        return NULL;
    }
    
    scc_result_t* result = scc_kosaraju_internal(graph, state);
    kosaraju_state_destroy(state);
    
    return result;
}

// 내부 헬퍼 함수들 구현
static void kosaraju_dfs_first_recursive(const graph_t* graph, int vertex, kosaraju_state_t* state) {
    state->visited_first_pass[vertex] = true;
    
    vertex_t* v = graph->vertices[vertex];
    edge_t* edge = v->edges;
    
    while (edge) {
        if (!state->visited_first_pass[edge->dest]) {
            kosaraju_dfs_first_recursive(graph, edge->dest, state);
        }
        edge = edge->next;
    }
    
    // 완료 시간 순서로 기록 (후위 순서)
    state->finish_order[state->finish_index++] = vertex;
}

static void kosaraju_dfs_second_recursive(const graph_t* graph, int vertex, kosaraju_state_t* state) {
    state->visited_second_pass[vertex] = true;
    
    // 현재 컴포넌트에 정점 추가
    scc_component_t* component = &state->result->components[state->current_component];
    component->vertices[component->size++] = vertex;
    state->result->vertex_to_component[vertex] = state->current_component;
    
    vertex_t* v = graph->vertices[vertex];
    edge_t* edge = v->edges;
    
    while (edge) {
        if (!state->visited_second_pass[edge->dest]) {
            kosaraju_dfs_second_recursive(graph, edge->dest, state);
        }
        edge = edge->next;
    }
}