#include "graph.h"
#include "scc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// 내부 헬퍼 함수들
static int load_edge_list_format(graph_t** graph, FILE* file);
static int load_adjacency_list_format(graph_t** graph, FILE* file);
static int save_edge_list_format(const graph_t* graph, FILE* file);
static int save_adjacency_list_format(const graph_t* graph, FILE* file);
static int save_dot_format(const graph_t* graph, FILE* file);
static char* trim_whitespace(char* str);
static int parse_integers(const char* line, int* numbers, int max_numbers);

// 그래프 파일 로드
int graph_load_from_file(graph_t** graph, const char* filename, graph_format_t format) {
    if (!graph || !filename) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return SCC_ERROR_NULL_POINTER;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return SCC_ERROR_INVALID_PARAMETER;
    }
    
    int result;
    switch (format) {
        case GRAPH_FORMAT_EDGE_LIST:
            result = load_edge_list_format(graph, file);
            break;
        case GRAPH_FORMAT_ADJACENCY_LIST:
            result = load_adjacency_list_format(graph, file);
            break;
        default:
            scc_set_error(SCC_ERROR_INVALID_PARAMETER);
            result = SCC_ERROR_INVALID_PARAMETER;
            break;
    }
    
    fclose(file);
    return result;
}

// 그래프 파일 저장
int graph_save_to_file(const graph_t* graph, const char* filename, graph_format_t format) {
    if (!graph || !filename) {
        scc_set_error(SCC_ERROR_NULL_POINTER);
        return SCC_ERROR_NULL_POINTER;
    }
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return SCC_ERROR_INVALID_PARAMETER;
    }
    
    int result;
    switch (format) {
        case GRAPH_FORMAT_EDGE_LIST:
            result = save_edge_list_format(graph, file);
            break;
        case GRAPH_FORMAT_ADJACENCY_LIST:
            result = save_adjacency_list_format(graph, file);
            break;
        case GRAPH_FORMAT_DOT:
            result = save_dot_format(graph, file);
            break;
        default:
            scc_set_error(SCC_ERROR_INVALID_PARAMETER);
            result = SCC_ERROR_INVALID_PARAMETER;
            break;
    }
    
    fclose(file);
    return result;
}

// 간선 리스트 형식 로드
static int load_edge_list_format(graph_t** graph, FILE* file) {
    char line[1024];
    int max_vertex = -1;
    
    // 첫 번째 패스: 최대 정점 번호 찾기
    long file_pos = ftell(file);
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#') {
            continue; // 빈 줄이나 주석 건너뛰기
        }
        
        int src, dest;
        if (sscanf(trimmed, "%d %d", &src, &dest) == 2) {
            if (src > max_vertex) max_vertex = src;
            if (dest > max_vertex) max_vertex = dest;
        }
    }
    
    if (max_vertex < 0) {
        scc_set_error(SCC_ERROR_GRAPH_EMPTY);
        return SCC_ERROR_GRAPH_EMPTY;
    }
    
    // 그래프 생성
    *graph = graph_create(max_vertex + 1);
    if (!*graph) {
        return SCC_ERROR_MEMORY_ALLOCATION;
    }
    
    // 모든 정점 추가
    for (int i = 0; i <= max_vertex; i++) {
        if (graph_add_vertex(*graph) != i) {
            graph_destroy(*graph);
            *graph = NULL;
            return SCC_ERROR_MEMORY_ALLOCATION;
        }
    }
    
    // 두 번째 패스: 간선 추가
    fseek(file, file_pos, SEEK_SET);
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#') {
            continue;
        }
        
        int src, dest;
        if (sscanf(trimmed, "%d %d", &src, &dest) == 2) {
            int result = graph_add_edge(*graph, src, dest);
            if (result != SCC_SUCCESS && result != SCC_ERROR_EDGE_EXISTS) {
                graph_destroy(*graph);
                *graph = NULL;
                return result;
            }
        }
    }
    
    return SCC_SUCCESS;
}

// 인접 리스트 형식 로드
static int load_adjacency_list_format(graph_t** graph, FILE* file) {
    char line[2048];
    int max_vertex = -1;
    
    // 첫 번째 패스: 최대 정점 번호 찾기
    long file_pos = ftell(file);
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#') {
            continue;
        }
        
        int numbers[1024]; // 충분히 큰 배열
        int count = parse_integers(trimmed, numbers, 1024);
        
        for (int i = 0; i < count; i++) {
            if (numbers[i] > max_vertex) {
                max_vertex = numbers[i];
            }
        }
    }
    
    if (max_vertex < 0) {
        scc_set_error(SCC_ERROR_GRAPH_EMPTY);
        return SCC_ERROR_GRAPH_EMPTY;
    }
    
    // 그래프 생성
    *graph = graph_create(max_vertex + 1);
    if (!*graph) {
        return SCC_ERROR_MEMORY_ALLOCATION;
    }
    
    // 모든 정점 추가
    for (int i = 0; i <= max_vertex; i++) {
        if (graph_add_vertex(*graph) != i) {
            graph_destroy(*graph);
            *graph = NULL;
            return SCC_ERROR_MEMORY_ALLOCATION;
        }
    }
    
    // 두 번째 패스: 간선 추가
    fseek(file, file_pos, SEEK_SET);
    while (fgets(line, sizeof(line), file)) {
        char* trimmed = trim_whitespace(line);
        if (strlen(trimmed) == 0 || trimmed[0] == '#') {
            continue;
        }
        
        int numbers[1024];
        int count = parse_integers(trimmed, numbers, 1024);
        
        if (count >= 2) {
            int src = numbers[0];
            // 첫 번째 숫자는 소스 정점, 나머지는 목적지 정점들
            for (int i = 1; i < count; i++) {
                int dest = numbers[i];
                int result = graph_add_edge(*graph, src, dest);
                if (result != SCC_SUCCESS && result != SCC_ERROR_EDGE_EXISTS) {
                    graph_destroy(*graph);
                    *graph = NULL;
                    return result;
                }
            }
        }
    }
    
    return SCC_SUCCESS;
}

// 간선 리스트 형식 저장
static int save_edge_list_format(const graph_t* graph, FILE* file) {
    fprintf(file, "# 간선 리스트 형식\n");
    fprintf(file, "# 형식: 소스_정점 목적지_정점\n");
    fprintf(file, "# 정점 수: %d, 간선 수: %d\n", 
            graph_get_vertex_count(graph), graph_get_edge_count(graph));
    fprintf(file, "\n");
    
    for (int src = 0; src < graph_get_vertex_count(graph); src++) {
        vertex_t* vertex = graph->vertices[src];
        edge_t* edge = vertex->edges;
        
        while (edge) {
            fprintf(file, "%d %d\n", src, edge->dest);
            edge = edge->next;
        }
    }
    
    return SCC_SUCCESS;
}

// 인접 리스트 형식 저장
static int save_adjacency_list_format(const graph_t* graph, FILE* file) {
    fprintf(file, "# 인접 리스트 형식\n");
    fprintf(file, "# 형식: 소스_정점 목적지1 목적지2 ...\n");
    fprintf(file, "# 정점 수: %d, 간선 수: %d\n", 
            graph_get_vertex_count(graph), graph_get_edge_count(graph));
    fprintf(file, "\n");
    
    for (int src = 0; src < graph_get_vertex_count(graph); src++) {
        vertex_t* vertex = graph->vertices[src];
        
        if (vertex->out_degree > 0) {
            fprintf(file, "%d", src);
            
            edge_t* edge = vertex->edges;
            while (edge) {
                fprintf(file, " %d", edge->dest);
                edge = edge->next;
            }
            
            fprintf(file, "\n");
        }
    }
    
    return SCC_SUCCESS;
}

// DOT 형식 저장 (Graphviz용)
static int save_dot_format(const graph_t* graph, FILE* file) {
    fprintf(file, "digraph G {\n");
    fprintf(file, "  // SCC 그래프 - 정점 수: %d, 간선 수: %d\n", 
            graph_get_vertex_count(graph), graph_get_edge_count(graph));
    fprintf(file, "  \n");
    
    // 정점 정의 (선택사항)
    for (int i = 0; i < graph_get_vertex_count(graph); i++) {
        fprintf(file, "  %d [label=\"%d\"];\n", i, i);
    }
    
    fprintf(file, "  \n");
    
    // 간선 정의
    for (int src = 0; src < graph_get_vertex_count(graph); src++) {
        vertex_t* vertex = graph->vertices[src];
        edge_t* edge = vertex->edges;
        
        while (edge) {
            fprintf(file, "  %d -> %d;\n", src, edge->dest);
            edge = edge->next;
        }
    }
    
    fprintf(file, "}\n");
    
    return SCC_SUCCESS;
}

// 헬퍼 함수들
static char* trim_whitespace(char* str) {
    // 앞쪽 공백 제거
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return str; // 모든 문자가 공백
    
    // 뒤쪽 공백 제거
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    *(end + 1) = 0;
    
    return str;
}

static int parse_integers(const char* line, int* numbers, int max_numbers) {
    int count = 0;
    char* str_copy = malloc(strlen(line) + 1);
    if (!str_copy) return 0;
    
    strcpy(str_copy, line);
    
    char* token = strtok(str_copy, " \t\n\r");
    while (token && count < max_numbers) {
        char* endptr;
        int num = strtol(token, &endptr, 10);
        
        if (*endptr == '\0') { // 전체 토큰이 숫자
            numbers[count++] = num;
        }
        
        token = strtok(NULL, " \t\n\r");
    }
    
    free(str_copy);
    return count;
}