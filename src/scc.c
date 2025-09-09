#include "scc.h"
#include "scc_algorithms.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

// SCC 결과 관리
void scc_result_destroy(scc_result_t* result) {
    if (!result) return;
    
    if (result->components) {
        for (int i = 0; i < result->num_components; i++) {
            free(result->components[i].vertices);
        }
        free(result->components);
    }
    
    free(result->vertex_to_component);
    free(result);
}

scc_result_t* scc_result_copy(const scc_result_t* result) {
    if (!result) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    scc_result_t* copy = malloc(sizeof(scc_result_t));
    if (!copy) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    copy->num_components = result->num_components;
    copy->largest_component_size = result->largest_component_size;
    copy->smallest_component_size = result->smallest_component_size;
    copy->average_component_size = result->average_component_size;
    
    // vertex_to_component 배열 복사
    int total_vertices = 0;
    for (int i = 0; i < result->num_components; i++) {
        total_vertices += result->components[i].size;
    }
    
    copy->vertex_to_component = malloc(total_vertices * sizeof(int));
    if (!copy->vertex_to_component) {
        free(copy);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    memcpy(copy->vertex_to_component, result->vertex_to_component, 
           total_vertices * sizeof(int));
    
    // 컴포넌트들 복사
    copy->components = malloc(result->num_components * sizeof(scc_component_t));
    if (!copy->components) {
        free(copy->vertex_to_component);
        free(copy);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    for (int i = 0; i < result->num_components; i++) {
        scc_component_t* src_comp = &result->components[i];
        scc_component_t* dst_comp = &copy->components[i];
        
        dst_comp->size = src_comp->size;
        dst_comp->capacity = src_comp->size; // 필요한 크기만 할당
        
        dst_comp->vertices = malloc(dst_comp->capacity * sizeof(int));
        if (!dst_comp->vertices) {
            // 이전 컴포넌트들 정리
            for (int j = 0; j < i; j++) {
                free(copy->components[j].vertices);
            }
            free(copy->components);
            free(copy->vertex_to_component);
            free(copy);
            scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
            return NULL;
        }
        
        memcpy(dst_comp->vertices, src_comp->vertices, src_comp->size * sizeof(int));
    }
    
    return copy;
}

// 결과 분석 함수들
int scc_get_component_count(const scc_result_t* result) {
    if (!result) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return -1;
    }
    return result->num_components;
}

int scc_get_component_size(const scc_result_t* result, int component_id) {
    if (!result || component_id < 0 || component_id >= result->num_components) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return -1;
    }
    return result->components[component_id].size;
}

int scc_get_vertex_component(const scc_result_t* result, int vertex) {
    if (!result || !result->vertex_to_component) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return -1;
    }
    
    // vertex 범위 검사 (총 정점 수를 알 수 없으므로 간접적으로 확인)
    if (vertex < 0) {
        scc_set_error(SCC_ERROR_INVALID_VERTEX);
        return -1;
    }
    
    return result->vertex_to_component[vertex];
}

const int* scc_get_component_vertices(const scc_result_t* result, int component_id) {
    if (!result || component_id < 0 || component_id >= result->num_components) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    return result->components[component_id].vertices;
}

// 그래프 속성 함수들
bool scc_is_strongly_connected(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return false;
    }
    
    scc_result_t* result = scc_find(graph);
    if (!result) {
        return false;
    }
    
    bool is_connected = (result->num_components == 1);
    scc_result_destroy(result);
    
    return is_connected;
}

graph_t* scc_build_condensation_graph(const graph_t* graph, const scc_result_t* scc) {
    if (!graph || !scc) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    graph_t* condensed = graph_create(scc->num_components);
    if (!condensed) {
        return NULL;
    }
    
    // 모든 컴포넌트에 대해 정점 추가
    for (int i = 0; i < scc->num_components; i++) {
        if (graph_add_vertex(condensed) != i) {
            graph_destroy(condensed);
            return NULL;
        }
    }
    
    // 컴포넌트 간 간선 추가
    int num_vertices = graph_get_vertex_count(graph);
    for (int v = 0; v < num_vertices; v++) {
        vertex_t* vertex = graph->vertices[v];
        int src_comp = scc->vertex_to_component[v];
        
        edge_t* edge = vertex->edges;
        while (edge) {
            int dest_comp = scc->vertex_to_component[edge->dest];
            
            // 다른 컴포넌트로의 간선만 추가 (자기 자신 제외)
            if (src_comp != dest_comp && !graph_has_edge(condensed, src_comp, dest_comp)) {
                if (graph_add_edge(condensed, src_comp, dest_comp) != SCC_SUCCESS) {
                    graph_destroy(condensed);
                    return NULL;
                }
            }
            
            edge = edge->next;
        }
    }
    
    return condensed;
}

// 기본 SCC 찾기 함수 (자동 알고리즘 선택)
scc_result_t* scc_find(const graph_t* graph) {
    if (!graph) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return NULL;
    }
    
    // 알고리즘 자동 선택
    scc_algorithm_choice_t algorithm = scc_recommend_algorithm(graph);
    
    switch (algorithm) {
        case SCC_ALGORITHM_TARJAN:
            return scc_find_tarjan(graph);
        case SCC_ALGORITHM_KOSARAJU:
            return scc_find_kosaraju(graph);
        default:
            return scc_find_tarjan(graph); // 기본값
    }
}

// 통계 출력 함수들
void scc_print_statistics(const scc_result_t* result) {
    if (!result) {
        printf("SCC 결과: NULL\n");
        return;
    }
    
    printf("강한 연결 요소 통계:\n");
    printf("  전체 컴포넌트 수: %d\n", result->num_components);
    printf("  가장 큰 컴포넌트 크기: %d\n", result->largest_component_size);
    printf("  가장 작은 컴포넌트 크기: %d\n", result->smallest_component_size);
    printf("  평균 컴포넌트 크기: %.2f\n", result->average_component_size);
}

void scc_print_components(const scc_result_t* result) {
    if (!result) {
        printf("SCC 결과: NULL\n");
        return;
    }
    
    printf("강한 연결 요소들:\n");
    for (int i = 0; i < result->num_components; i++) {
        scc_component_t* comp = &result->components[i];
        printf("  컴포넌트 %d (%d개 정점): ", i, comp->size);
        
        for (int j = 0; j < comp->size; j++) {
            printf("%d ", comp->vertices[j]);
            if (j > 10 && comp->size > 15) { // 너무 길면 생략
                printf("... (총 %d개)", comp->size);
                break;
            }
        }
        printf("\n");
    }
}

// 알고리즘 선택 휴리스틱
scc_algorithm_choice_t scc_recommend_algorithm(const graph_t* graph) {
    if (!graph) {
        return SCC_ALGORITHM_TARJAN; // 기본값
    }
    
    int num_vertices = graph_get_vertex_count(graph);
    int num_edges = graph_get_edge_count(graph);
    
    if (num_vertices == 0) {
        return SCC_ALGORITHM_TARJAN;
    }
    
    double density = (double)num_edges / ((double)num_vertices * num_vertices);
    
    // 작은 그래프는 오버헤드가 중요하지 않음
    if (num_vertices < 1000) {
        return SCC_ALGORITHM_TARJAN;
    }
    
    // 밀집 그래프는 더 단순한 접근 방식을 선호
    if (density > 0.1) {
        return SCC_ALGORITHM_KOSARAJU;
    }
    
    // 기본값: 더 나은 캐시 성능
    return SCC_ALGORITHM_TARJAN;
}

const char* scc_algorithm_name(scc_algorithm_choice_t algorithm) {
    switch (algorithm) {
        case SCC_ALGORITHM_AUTO:
            return "자동 선택";
        case SCC_ALGORITHM_TARJAN:
            return "Tarjan";
        case SCC_ALGORITHM_KOSARAJU:
            return "Kosaraju";
        default:
            return "알 수 없음";
    }
}