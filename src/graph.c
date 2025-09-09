#include "graph.h"
#include "scc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 내부 헬퍼 함수들
static int graph_ensure_capacity(graph_t* graph, int required_capacity);
static edge_t* edge_create(int dest);
static void edge_destroy(edge_t* edge);
static vertex_t* vertex_create(int id);
static void vertex_destroy(vertex_t* vertex);

// 그래프 생성 및 소멸
graph_t* graph_create(int initial_capacity) {
    if (initial_capacity < 0) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    if (initial_capacity == 0) initial_capacity = 16; // 기본 용량
    
    graph_t* graph = malloc(sizeof(graph_t));
    if (!graph) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    graph->vertices = calloc(initial_capacity, sizeof(vertex_t*));
    if (!graph->vertices) {
        free(graph);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    graph->num_vertices = 0;
    graph->num_edges = 0;
    graph->capacity = initial_capacity;
    graph->vertex_pool = NULL;
    graph->edge_pool = NULL;
    
    return graph;
}

graph_t* graph_create_with_pools(int initial_capacity, 
                                 memory_pool_t* vertex_pool,
                                 memory_pool_t* edge_pool) {
    graph_t* graph = graph_create(initial_capacity);
    if (!graph) return NULL;
    
    graph->vertex_pool = vertex_pool;
    graph->edge_pool = edge_pool;
    
    return graph;
}

void graph_destroy(graph_t* graph) {
    if (!graph) return;
    
    // 모든 정점과 간선 정리
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph->vertices[i]) {
            vertex_destroy(graph->vertices[i]);
        }
    }
    
    free(graph->vertices);
    free(graph);
}

// 그래프 수정 함수들
int graph_add_vertex(graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return -1;
    }
    
    // 용량 확인 및 확장
    if (graph->num_vertices >= graph->capacity) {
        if (graph_ensure_capacity(graph, graph->capacity * 2) != SCC_SUCCESS) {
            return -1;
        }
    }
    
    int vertex_id = graph->num_vertices;
    vertex_t* vertex = vertex_create(vertex_id);
    if (!vertex) {
        return -1;
    }
    
    graph->vertices[vertex_id] = vertex;
    graph->num_vertices++;
    
    return vertex_id;
}

int graph_add_edge(graph_t* graph, int src, int dest) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return SCC_ERROR_NULL_POINTER;
    }
    
    if (src < 0 || src >= graph->num_vertices || 
        dest < 0 || dest >= graph->num_vertices) {
        scc_set_error(SCC_ERROR_INVALID_VERTEX);
        return SCC_ERROR_INVALID_VERTEX;
    }
    
    // 간선이 이미 존재하는지 확인
    if (graph_has_edge(graph, src, dest)) {
        scc_set_error(SCC_ERROR_EDGE_EXISTS);
        return SCC_ERROR_EDGE_EXISTS;
    }
    
    vertex_t* src_vertex = graph->vertices[src];
    edge_t* new_edge = edge_create(dest);
    if (!new_edge) {
        return SCC_ERROR_MEMORY_ALLOCATION;
    }
    
    // 간선을 리스트 앞에 추가
    new_edge->next = src_vertex->edges;
    src_vertex->edges = new_edge;
    src_vertex->out_degree++;
    graph->num_edges++;
    
    return SCC_SUCCESS;
}

int graph_remove_edge(graph_t* graph, int src, int dest) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return SCC_ERROR_NULL_POINTER;
    }
    
    if (src < 0 || src >= graph->num_vertices || 
        dest < 0 || dest >= graph->num_vertices) {
        scc_set_error(SCC_ERROR_INVALID_VERTEX);
        return SCC_ERROR_INVALID_VERTEX;
    }
    
    vertex_t* src_vertex = graph->vertices[src];
    edge_t* edge = src_vertex->edges;
    edge_t* prev = NULL;
    
    while (edge) {
        if (edge->dest == dest) {
            // 간선 제거
            if (prev) {
                prev->next = edge->next;
            } else {
                src_vertex->edges = edge->next;
            }
            
            edge_destroy(edge);
            src_vertex->out_degree--;
            graph->num_edges--;
            
            return SCC_SUCCESS;
        }
        
        prev = edge;
        edge = edge->next;
    }
    
    return SCC_ERROR_INVALID_PARAMETER; // 간선을 찾을 수 없음
}

// 그래프 쿼리 함수들
bool graph_has_edge(const graph_t* graph, int src, int dest) {
    if (!graph || src < 0 || src >= graph->num_vertices || 
        dest < 0 || dest >= graph->num_vertices) {
        return false;
    }
    
    vertex_t* src_vertex = graph->vertices[src];
    edge_t* edge = src_vertex->edges;
    
    while (edge) {
        if (edge->dest == dest) {
            return true;
        }
        edge = edge->next;
    }
    
    return false;
}

int graph_get_out_degree(const graph_t* graph, int vertex) {
    if (!graph || vertex < 0 || vertex >= graph->num_vertices) {
        scc_set_error(SCC_ERROR_INVALID_VERTEX);
        return -1;
    }
    
    return graph->vertices[vertex]->out_degree;
}

int graph_get_vertex_count(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return -1;
    }
    return graph->num_vertices;
}

int graph_get_edge_count(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return -1;
    }
    return graph->num_edges;
}

// 그래프 복사 및 변환
graph_t* graph_copy(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    graph_t* copy = graph_create(graph->capacity);
    if (!copy) return NULL;
    
    // 모든 정점 추가
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph_add_vertex(copy) != i) {
            graph_destroy(copy);
            return NULL;
        }
    }
    
    // 모든 간선 복사
    for (int src = 0; src < graph->num_vertices; src++) {
        vertex_t* vertex = graph->vertices[src];
        edge_t* edge = vertex->edges;
        
        while (edge) {
            if (graph_add_edge(copy, src, edge->dest) != SCC_SUCCESS) {
                graph_destroy(copy);
                return NULL;
            }
            edge = edge->next;
        }
        
        // 사용자 데이터 복사
        copy->vertices[src]->data = vertex->data;
    }
    
    return copy;
}

graph_t* graph_transpose(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    graph_t* transpose = graph_create(graph->capacity);
    if (!transpose) return NULL;
    
    // 모든 정점 추가
    for (int i = 0; i < graph->num_vertices; i++) {
        if (graph_add_vertex(transpose) != i) {
            graph_destroy(transpose);
            return NULL;
        }
    }
    
    // 모든 간선을 반대 방향으로 추가
    for (int src = 0; src < graph->num_vertices; src++) {
        vertex_t* vertex = graph->vertices[src];
        edge_t* edge = vertex->edges;
        
        while (edge) {
            if (graph_add_edge(transpose, edge->dest, src) != SCC_SUCCESS) {
                graph_destroy(transpose);
                return NULL;
            }
            edge = edge->next;
        }
    }
    
    return transpose;
}

// 정점 데이터 관리
int graph_set_vertex_data(graph_t* graph, int vertex, void* data) {
    if (!graph || vertex < 0 || vertex >= graph->num_vertices) {
        scc_set_error(SCC_ERROR_INVALID_VERTEX);
        return SCC_ERROR_INVALID_VERTEX;
    }
    
    graph->vertices[vertex]->data = data;
    return SCC_SUCCESS;
}

void* graph_get_vertex_data(const graph_t* graph, int vertex) {
    if (!graph || vertex < 0 || vertex >= graph->num_vertices) {
        scc_set_error(SCC_ERROR_INVALID_VERTEX);
        return NULL;
    }
    
    return graph->vertices[vertex]->data;
}

// 그래프 검증
bool graph_is_valid(const graph_t* graph) {
    if (!graph || !graph->vertices) return false;
    if (graph->num_vertices < 0 || graph->num_edges < 0) return false;
    if (graph->num_vertices > graph->capacity) return false;
    
    int edge_count = 0;
    for (int i = 0; i < graph->num_vertices; i++) {
        vertex_t* vertex = graph->vertices[i];
        if (!vertex || vertex->id != i) return false;
        
        // 간선 개수 검증
        int out_degree = 0;
        edge_t* edge = vertex->edges;
        while (edge) {
            if (edge->dest < 0 || edge->dest >= graph->num_vertices) return false;
            out_degree++;
            edge_count++;
            edge = edge->next;
        }
        
        if (vertex->out_degree != out_degree) return false;
    }
    
    return edge_count == graph->num_edges;
}

void graph_print_debug(const graph_t* graph) {
    if (!graph) {
        printf("Graph: NULL\n");
        return;
    }
    
    printf("Graph: %d vertices, %d edges, capacity %d\n", 
           graph->num_vertices, graph->num_edges, graph->capacity);
    
    for (int i = 0; i < graph->num_vertices; i++) {
        vertex_t* vertex = graph->vertices[i];
        printf("  Vertex %d (degree %d): ", i, vertex->out_degree);
        
        edge_t* edge = vertex->edges;
        while (edge) {
            printf("%d ", edge->dest);
            edge = edge->next;
        }
        printf("\n");
    }
}

// 내부 헬퍼 함수들 구현
static int graph_ensure_capacity(graph_t* graph, int required_capacity) {
    if (graph->capacity >= required_capacity) {
        return SCC_SUCCESS;
    }
    
    vertex_t** new_vertices = realloc(graph->vertices, 
                                     required_capacity * sizeof(vertex_t*));
    if (!new_vertices) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return SCC_ERROR_MEMORY_ALLOCATION;
    }
    
    // 새로운 공간을 NULL로 초기화
    for (int i = graph->capacity; i < required_capacity; i++) {
        new_vertices[i] = NULL;
    }
    
    graph->vertices = new_vertices;
    graph->capacity = required_capacity;
    
    return SCC_SUCCESS;
}

static edge_t* edge_create(int dest) {
    edge_t* edge = malloc(sizeof(edge_t));
    if (!edge) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    edge->dest = dest;
    edge->next = NULL;
    
    return edge;
}

static void edge_destroy(edge_t* edge) {
    if (edge) {
        free(edge);
    }
}

static vertex_t* vertex_create(int id) {
    vertex_t* vertex = malloc(sizeof(vertex_t));
    if (!vertex) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    vertex->id = id;
    vertex->edges = NULL;
    vertex->out_degree = 0;
    vertex->index = -1;
    vertex->lowlink = -1;
    vertex->on_stack = false;
    vertex->visited = false;
    vertex->data = NULL;
    
    return vertex;
}

static void vertex_destroy(vertex_t* vertex) {
    if (!vertex) return;
    
    // 모든 간선 정리
    edge_t* edge = vertex->edges;
    while (edge) {
        edge_t* next = edge->next;
        edge_destroy(edge);
        edge = next;
    }
    
    free(vertex);
}