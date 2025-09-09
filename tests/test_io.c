#include "test_framework.h"
#include "../src/graph.h"
#include "../src/scc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

// 임시 파일명 생성을 위한 헬퍼 함수
static char* get_temp_filename(const char* suffix) {
    static char filename[256];
    static int counter = 0;
    snprintf(filename, sizeof(filename), "temp_test_%d_%s", counter++, suffix);
    return filename;
}

// 파일 내용 확인 헬퍼 함수
static bool file_contains_string(const char* filename, const char* search_string) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;
    
    char line[1024];
    bool found = false;
    
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, search_string)) {
            found = true;
            break;
        }
    }
    
    fclose(file);
    return found;
}

// 간선 리스트 형식 저장/로드 테스트
static void test_edge_list_format() {
    TEST_START("Edge list format I/O");
    
    // 원본 그래프 생성
    graph_t* original = graph_create(4);
    for (int i = 0; i < 4; i++) {
        graph_add_vertex(original);
    }
    
    graph_add_edge(original, 0, 1);
    graph_add_edge(original, 1, 2);
    graph_add_edge(original, 2, 3);
    graph_add_edge(original, 3, 0);
    graph_add_edge(original, 0, 2);  // 추가 간선
    
    // 파일에 저장
    char* filename = get_temp_filename("edges.txt");
    int result = graph_save_to_file(original, filename, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_SUCCESS, "간선 리스트 저장이 성공해야 함");
    
    // 파일 내용 확인
    ASSERT_TRUE(file_contains_string(filename, "0 1"), "파일에 간선 0->1이 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "1 2"), "파일에 간선 1->2가 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "2 3"), "파일에 간선 2->3이 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "3 0"), "파일에 간선 3->0이 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "0 2"), "파일에 간선 0->2가 있어야 함");
    
    // 파일에서 로드
    graph_t* loaded = NULL;
    result = graph_load_from_file(&loaded, filename, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_SUCCESS, "간선 리스트 로드가 성공해야 함");
    ASSERT_NOT_NULL(loaded, "로드된 그래프가 NULL이 아니어야 함");
    
    // 원본과 로드된 그래프 비교
    ASSERT_EQUAL(graph_get_vertex_count(loaded), graph_get_vertex_count(original), 
                 "정점 수가 같아야 함");
    ASSERT_EQUAL(graph_get_edge_count(loaded), graph_get_edge_count(original), 
                 "간선 수가 같아야 함");
    
    // 모든 간선이 올바르게 로드되었는지 확인
    ASSERT_TRUE(graph_has_edge(loaded, 0, 1), "로드된 그래프에 간선 0->1이 있어야 함");
    ASSERT_TRUE(graph_has_edge(loaded, 1, 2), "로드된 그래프에 간선 1->2가 있어야 함");
    ASSERT_TRUE(graph_has_edge(loaded, 2, 3), "로드된 그래프에 간선 2->3이 있어야 함");
    ASSERT_TRUE(graph_has_edge(loaded, 3, 0), "로드된 그래프에 간선 3->0이 있어야 함");
    ASSERT_TRUE(graph_has_edge(loaded, 0, 2), "로드된 그래프에 간선 0->2가 있어야 함");
    
    // 정리
    remove(filename);
    graph_destroy(loaded);
    graph_destroy(original);
    TEST_END();
}

// 인접 리스트 형식 저장/로드 테스트
static void test_adjacency_list_format() {
    TEST_START("Adjacency list format I/O");
    
    // 원본 그래프 생성
    graph_t* original = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(original);
    }
    
    graph_add_edge(original, 0, 1);
    graph_add_edge(original, 0, 2);
    graph_add_edge(original, 1, 2);
    // 정점 2는 나가는 간선이 없음
    
    // 파일에 저장
    char* filename = get_temp_filename("adj.txt");
    int result = graph_save_to_file(original, filename, GRAPH_FORMAT_ADJACENCY_LIST);
    ASSERT_EQUAL(result, SCC_SUCCESS, "인접 리스트 저장이 성공해야 함");
    
    // 파일 내용 확인 (정점 0의 인접 리스트)
    ASSERT_TRUE(file_contains_string(filename, "0 1 2") || 
                file_contains_string(filename, "0 2 1"), 
                "파일에 정점 0의 인접 리스트가 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "1 2"), "파일에 정점 1의 인접 리스트가 있어야 함");
    
    // 파일에서 로드
    graph_t* loaded = NULL;
    result = graph_load_from_file(&loaded, filename, GRAPH_FORMAT_ADJACENCY_LIST);
    ASSERT_EQUAL(result, SCC_SUCCESS, "인접 리스트 로드가 성공해야 함");
    ASSERT_NOT_NULL(loaded, "로드된 그래프가 NULL이 아니어야 함");
    
    // 원본과 로드된 그래프 비교
    ASSERT_EQUAL(graph_get_vertex_count(loaded), graph_get_vertex_count(original), 
                 "정점 수가 같아야 함");
    ASSERT_EQUAL(graph_get_edge_count(loaded), graph_get_edge_count(original), 
                 "간선 수가 같아야 함");
    
    // 모든 간선이 올바르게 로드되었는지 확인
    ASSERT_TRUE(graph_has_edge(loaded, 0, 1), "로드된 그래프에 간선 0->1이 있어야 함");
    ASSERT_TRUE(graph_has_edge(loaded, 0, 2), "로드된 그래프에 간선 0->2가 있어야 함");
    ASSERT_TRUE(graph_has_edge(loaded, 1, 2), "로드된 그래프에 간선 1->2가 있어야 함");
    
    // 정리
    remove(filename);
    graph_destroy(loaded);
    graph_destroy(original);
    TEST_END();
}

// DOT 형식 저장 테스트
static void test_dot_format() {
    TEST_START("DOT format export");
    
    // 그래프 생성
    graph_t* graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(graph);
    }
    
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    // DOT 형식으로 저장
    char* filename = get_temp_filename("graph.dot");
    int result = graph_save_to_file(graph, filename, GRAPH_FORMAT_DOT);
    ASSERT_EQUAL(result, SCC_SUCCESS, "DOT 형식 저장이 성공해야 함");
    
    // DOT 파일 구조 확인
    ASSERT_TRUE(file_contains_string(filename, "digraph G"), "DOT 파일에 digraph 선언이 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "0 -> 1"), "DOT 파일에 간선 0->1이 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "1 -> 2"), "DOT 파일에 간선 1->2가 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "2 -> 0"), "DOT 파일에 간선 2->0이 있어야 함");
    ASSERT_TRUE(file_contains_string(filename, "}"), "DOT 파일에 닫는 괄호가 있어야 함");
    
    // 정리
    remove(filename);
    graph_destroy(graph);
    TEST_END();
}

// 주석과 빈 줄 처리 테스트
static void test_comments_and_empty_lines() {
    TEST_START("Comments and empty lines handling");
    
    char* filename = get_temp_filename("with_comments.txt");
    
    // 주석과 빈 줄이 있는 파일 작성
    FILE* file = fopen(filename, "w");
    ASSERT_NOT_NULL(file, "테스트 파일 생성이 성공해야 함");
    
    fprintf(file, "# 이것은 주석입니다\n");
    fprintf(file, "\n");  // 빈 줄
    fprintf(file, "0 1\n");
    fprintf(file, "# 또 다른 주석\n");
    fprintf(file, "1 2\n");
    fprintf(file, "\n");  // 또 다른 빈 줄
    fprintf(file, "2 0\n");
    fclose(file);
    
    // 파일 로드
    graph_t* graph = NULL;
    int result = graph_load_from_file(&graph, filename, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_SUCCESS, "주석이 있는 파일 로드가 성공해야 함");
    ASSERT_NOT_NULL(graph, "로드된 그래프가 NULL이 아니어야 함");
    
    // 그래프 내용 확인
    ASSERT_EQUAL(graph_get_vertex_count(graph), 3, "정점 수가 3개여야 함");
    ASSERT_EQUAL(graph_get_edge_count(graph), 3, "간선 수가 3개여야 함");
    ASSERT_TRUE(graph_has_edge(graph, 0, 1), "간선 0->1이 있어야 함");
    ASSERT_TRUE(graph_has_edge(graph, 1, 2), "간선 1->2가 있어야 함");
    ASSERT_TRUE(graph_has_edge(graph, 2, 0), "간선 2->0이 있어야 함");
    
    // 정리
    remove(filename);
    graph_destroy(graph);
    TEST_END();
}

// 빈 그래프 I/O 테스트
static void test_empty_graph_io() {
    TEST_START("Empty graph I/O");
    
    // 빈 그래프 생성 (정점 없음)
    graph_t* empty_graph = graph_create(5);
    
    char* filename = get_temp_filename("empty.txt");
    int result = graph_save_to_file(empty_graph, filename, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_SUCCESS, "빈 그래프 저장이 성공해야 함");
    
    // 빈 파일에서 로드 (주석만 있는 파일)
    graph_t* loaded = NULL;
    result = graph_load_from_file(&loaded, filename, GRAPH_FORMAT_EDGE_LIST);
    
    // 빈 그래프의 경우 구현에 따라 오류를 반환하거나 빈 그래프를 생성할 수 있음
    if (result == SCC_SUCCESS) {
        ASSERT_NOT_NULL(loaded, "로드가 성공했다면 그래프가 NULL이 아니어야 함");
        ASSERT_EQUAL(graph_get_vertex_count(loaded), 0, "빈 그래프의 정점 수는 0이어야 함");
        graph_destroy(loaded);
    } else {
        ASSERT_EQUAL(result, SCC_ERROR_GRAPH_EMPTY, "빈 그래프 로드는 적절한 오류를 반환해야 함");
    }
    
    remove(filename);
    graph_destroy(empty_graph);
    TEST_END();
}

// 파일 오류 처리 테스트
static void test_file_error_handling() {
    TEST_START("File error handling");
    
    graph_t* graph = graph_create(2);
    graph_add_vertex(graph);
    graph_add_vertex(graph);
    graph_add_edge(graph, 0, 1);
    
    // 존재하지 않는 디렉토리에 저장 시도
    int result = graph_save_to_file(graph, "/nonexistent/path/file.txt", GRAPH_FORMAT_EDGE_LIST);
    ASSERT_NOT_EQUAL(result, SCC_SUCCESS, "잘못된 경로로 저장은 실패해야 함");
    
    // 존재하지 않는 파일에서 로드 시도
    graph_t* loaded = NULL;
    result = graph_load_from_file(&loaded, "nonexistent_file.txt", GRAPH_FORMAT_EDGE_LIST);
    ASSERT_NOT_EQUAL(result, SCC_SUCCESS, "존재하지 않는 파일 로드는 실패해야 함");
    ASSERT_NULL(loaded, "실패 시 그래프는 NULL이어야 함");
    
    // NULL 포인터로 호출
    result = graph_save_to_file(NULL, "test.txt", GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_ERROR_NULL_POINTER, "NULL 그래프 저장은 NULL 포인터 오류를 반환해야 함");
    
    result = graph_load_from_file(NULL, "test.txt", GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_ERROR_NULL_POINTER, "NULL 포인터로 로드는 NULL 포인터 오류를 반환해야 함");
    
    result = graph_save_to_file(graph, NULL, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_ERROR_NULL_POINTER, "NULL 파일명으로 저장은 NULL 포인터 오류를 반환해야 함");
    
    result = graph_load_from_file(&loaded, NULL, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(result, SCC_ERROR_NULL_POINTER, "NULL 파일명으로 로드는 NULL 포인터 오류를 반환해야 함");
    
    graph_destroy(graph);
    TEST_END();
}

// 잘못된 형식 테스트
static void test_invalid_format_handling() {
    TEST_START("Invalid format handling");
    
    graph_t* graph = graph_create(2);
    graph_add_vertex(graph);
    graph_add_vertex(graph);
    graph_add_edge(graph, 0, 1);
    
    // 잘못된 형식으로 저장
    int result = graph_save_to_file(graph, "test.txt", (graph_format_t)999);
    ASSERT_EQUAL(result, SCC_ERROR_INVALID_PARAMETER, "잘못된 형식으로 저장은 실패해야 함");
    
    // 잘못된 형식으로 로드
    graph_t* loaded = NULL;
    result = graph_load_from_file(&loaded, "test.txt", (graph_format_t)999);
    ASSERT_EQUAL(result, SCC_ERROR_INVALID_PARAMETER, "잘못된 형식으로 로드는 실패해야 함");
    
    graph_destroy(graph);
    TEST_END();
}

// 큰 그래프 I/O 성능 테스트
static void test_large_graph_io() {
    TEST_START("Large graph I/O performance");
    
    // 큰 그래프 생성 (성능 테스트)
    int size = 1000;
    graph_t* large_graph = graph_create(size);
    
    for (int i = 0; i < size; i++) {
        graph_add_vertex(large_graph);
    }
    
    // 연결된 그래프 만들기 (각 정점에서 다음 몇 개 정점으로 간선)
    for (int i = 0; i < size; i++) {
        for (int j = 1; j <= 3 && (i + j) < size; j++) {
            graph_add_edge(large_graph, i, i + j);
        }
    }
    
    char* filename = get_temp_filename("large.txt");
    
    BENCHMARK_START("Large graph save (1000 vertices)");
    int result = graph_save_to_file(large_graph, filename, GRAPH_FORMAT_EDGE_LIST);
    BENCHMARK_END();
    
    ASSERT_EQUAL(result, SCC_SUCCESS, "큰 그래프 저장이 성공해야 함");
    
    BENCHMARK_START("Large graph load (1000 vertices)");
    graph_t* loaded = NULL;
    result = graph_load_from_file(&loaded, filename, GRAPH_FORMAT_EDGE_LIST);
    BENCHMARK_END();
    
    ASSERT_EQUAL(result, SCC_SUCCESS, "큰 그래프 로드가 성공해야 함");
    ASSERT_NOT_NULL(loaded, "로드된 큰 그래프가 NULL이 아니어야 함");
    ASSERT_EQUAL(graph_get_vertex_count(loaded), size, "로드된 그래프의 정점 수가 같아야 함");
    
    // 정리
    remove(filename);
    graph_destroy(loaded);
    graph_destroy(large_graph);
    TEST_END();
}

// 모든 I/O 테스트 실행
void run_io_tests() {
    printf("=== I/O 모듈 테스트 ===\n");
    
    test_edge_list_format();
    test_adjacency_list_format();
    test_dot_format();
    test_comments_and_empty_lines();
    test_empty_graph_io();
    test_file_error_handling();
    test_invalid_format_handling();
    test_large_graph_io();
    
    printf("I/O 모듈 테스트 완료\n\n");
}