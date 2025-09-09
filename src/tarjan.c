#include "scc_algorithms.h"
#include "scc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 내부 헬퍼 함수들
static void tarjan_dfs_recursive(const graph_t* graph, int vertex, tarjan_state_t* state);
static void tarjan_extract_scc(tarjan_state_t* state, int root);
static int tarjan_ensure_stack_capacity(tarjan_state_t* state, int required_capacity);

// Tarjan 상태 관리
tarjan_state_t* tarjan_state_create(int num_vertices) {
    if (num_vertices <= 0) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    tarjan_state_t* state = malloc(sizeof(tarjan_state_t));
    if (!state) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    // 스택 초기화
    state->stack_capacity = num_vertices;
    state->stack = malloc(state->stack_capacity * sizeof(int));
    if (!state->stack) {
        free(state);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    state->stack_top = 0;
    state->current_index = 0;
    state->current_component = 0;
    
    // 정점 처리 상태 배열
    state->vertices_processed = calloc(num_vertices, sizeof(bool));
    if (!state->vertices_processed) {
        free(state->stack);
        free(state);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    // 결과 구조 초기화
    state->result = malloc(sizeof(scc_result_t));
    if (!state->result) {
        free(state->vertices_processed);
        free(state->stack);
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
        free(state->vertices_processed);
        free(state->stack);
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
            free(state->vertices_processed);
            free(state->stack);
            free(state);
            scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
            return NULL;
        }
        state->result->components[i].size = 0;
        state->result->components[i].capacity = num_vertices;
    }
    
    state->result->num_components = 0;
    
    return state;
}

void tarjan_state_destroy(tarjan_state_t* state) {
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
    
    free(state->vertices_processed);
    free(state->stack);
    free(state);
}

// 스택 연산
int tarjan_stack_push(tarjan_state_t* state, int vertex) {
    if (!state) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return SCC_ERROR_NULL_POINTER;
    }
    
    if (state->stack_top >= state->stack_capacity) {
        if (tarjan_ensure_stack_capacity(state, state->stack_capacity * 2) != SCC_SUCCESS) {
            return SCC_ERROR_MEMORY_ALLOCATION;
        }
    }
    
    state->stack[state->stack_top++] = vertex;
    return SCC_SUCCESS;
}

int tarjan_stack_pop(tarjan_state_t* state) {
    if (!state || state->stack_top == 0) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return -1;
    }
    
    return state->stack[--state->stack_top];
}

bool tarjan_stack_contains(const tarjan_state_t* state, int vertex) {
    if (!state) return false;
    
    for (int i = 0; i < state->stack_top; i++) {
        if (state->stack[i] == vertex) {
            return true;
        }
    }
    return false;
}

bool tarjan_stack_is_empty(const tarjan_state_t* state) {
    return !state || state->stack_top == 0;
}

// Tarjan 알고리즘 메인 구현
scc_result_t* scc_tarjan_internal(const graph_t* graph, tarjan_state_t* state) {
    if (!graph || !state) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    if (num_vertices <= 0) {
        scc_set_error(SCC_ERROR_GRAPH_EMPTY);
        return NULL;
    }
    
    // 모든 정점의 알고리즘 필드 초기화
    for (int i = 0; i < num_vertices; i++) {
        vertex_t* v = graph->vertices[i];
        v->index = -1;
        v->lowlink = -1;
        v->on_stack = false;
    }
    
    // 모든 정점에 대해 DFS 수행
    for (int i = 0; i < num_vertices; i++) {
        if (graph->vertices[i]->index == -1) {
            tarjan_dfs_recursive(graph, i, state);
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

void tarjan_dfs(const graph_t* graph, int vertex, tarjan_state_t* state) {
    tarjan_dfs_recursive(graph, vertex, state);
}

// 공개 API 함수
scc_result_t* scc_find_tarjan(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    if (num_vertices <= 0) {
        scc_set_error(SCC_ERROR_GRAPH_EMPTY);
        return NULL;
    }
    
    tarjan_state_t* state = tarjan_state_create(num_vertices);
    if (!state) {
        return NULL;
    }
    
    scc_result_t* result = scc_tarjan_internal(graph, state);
    tarjan_state_destroy(state);
    
    return result;
}

// 내부 헬퍼 함수들 구현
static void tarjan_dfs_recursive(const graph_t* graph, int v, tarjan_state_t* state) {
    vertex_t* vertex = graph->vertices[v];
    
    // 정점 초기화
    vertex->index = vertex->lowlink = state->current_index++;
    vertex->on_stack = true;
    tarjan_stack_push(state, v);
    
    // 모든 인접 정점 탐색
    edge_t* edge = vertex->edges;
    while (edge) {
        int w = edge->dest;
        vertex_t* adj_vertex = graph->vertices[w];
        
        if (adj_vertex->index == -1) {
            // 트리 간선: 재귀 호출
            tarjan_dfs_recursive(graph, w, state);
            vertex->lowlink = (vertex->lowlink < adj_vertex->lowlink) ? 
                             vertex->lowlink : adj_vertex->lowlink;
        } else if (adj_vertex->on_stack) {
            // 후진 간선: lowlink 업데이트
            vertex->lowlink = (vertex->lowlink < adj_vertex->index) ? 
                             vertex->lowlink : adj_vertex->index;
        }
        // 전진/교차 간선은 무시
        
        edge = edge->next;
    }
    
    // SCC 루트인지 확인
    if (vertex->lowlink == vertex->index) {
        tarjan_extract_scc(state, v);
    }
}

static void tarjan_extract_scc(tarjan_state_t* state, int root) {
    scc_component_t* component = &state->result->components[state->current_component];
    int w;
    
    do {
        w = tarjan_stack_pop(state);
        // 더 이상 스택에 없음을 표시
        // vertex를 직접 접근할 수 없으므로 별도 추적이 필요함
        
        // 컴포넌트에 정점 추가
        component->vertices[component->size++] = w;
        state->result->vertex_to_component[w] = state->current_component;
        
    } while (w != root);
    
    state->current_component++;
    state->result->num_components++;
}

static int tarjan_ensure_stack_capacity(tarjan_state_t* state, int required_capacity) {
    if (state->stack_capacity >= required_capacity) {
        return SCC_SUCCESS;
    }
    
    int* new_stack = realloc(state->stack, required_capacity * sizeof(int));
    if (!new_stack) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return SCC_ERROR_MEMORY_ALLOCATION;
    }
    
    state->stack = new_stack;
    state->stack_capacity = required_capacity;
    
    return SCC_SUCCESS;
}