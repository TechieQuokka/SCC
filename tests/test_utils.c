#include "test_framework.h"
#include "../src/scc.h"
#include "../src/graph.h"
#include <assert.h>

// DFS 방문 기록용 구조체
typedef struct {
    int* visited_order;
    int count;
    int capacity;
} visit_record_t;

// DFS/BFS 방문 콜백 함수
static void record_visit(int vertex, void* user_data) {
    visit_record_t* record = (visit_record_t*)user_data;
    if (record->count < record->capacity) {
        record->visited_order[record->count++] = vertex;
    }
}

// DFS 순회 테스트
static void test_graph_dfs() {
    TEST_START("Graph DFS traversal");
    
    graph_t* graph = graph_create(5);
    for (int i = 0; i < 5; i++) {
        graph_add_vertex(graph);
    }
    
    // 트리 구조 생성: 0->1, 0->2, 1->3, 1->4
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 0, 2);
    graph_add_edge(graph, 1, 3);
    graph_add_edge(graph, 1, 4);
    
    visit_record_t record;
    record.visited_order = malloc(5 * sizeof(int));
    record.count = 0;
    record.capacity = 5;
    
    // 정점 0부터 DFS 시작
    graph_dfs(graph, 0, record_visit, &record);
    
    // 모든 정점이 방문되었는지 확인
    ASSERT_EQUAL(record.count, 5, "DFS에서 모든 정점이 방문되어야 함");
    
    // 첫 번째 방문은 시작 정점이어야 함
    ASSERT_EQUAL(record.visited_order[0], 0, "DFS 첫 방문은 시작 정점이어야 함");
    
    // 방문된 모든 정점이 유효한 범위 내에 있는지 확인
    for (int i = 0; i < record.count; i++) {
        ASSERT_TRUE(record.visited_order[i] >= 0 && record.visited_order[i] < 5,
                    "방문된 정점이 유효한 범위에 있어야 함");
    }
    
    free(record.visited_order);
    graph_destroy(graph);
    TEST_END();
}

// BFS 순회 테스트
static void test_graph_bfs() {
    TEST_START("Graph BFS traversal");
    
    graph_t* graph = graph_create(5);
    for (int i = 0; i < 5; i++) {
        graph_add_vertex(graph);
    }
    
    // 트리 구조 생성: 0->1, 0->2, 1->3, 1->4
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 0, 2);
    graph_add_edge(graph, 1, 3);
    graph_add_edge(graph, 1, 4);
    
    visit_record_t record;
    record.visited_order = malloc(5 * sizeof(int));
    record.count = 0;
    record.capacity = 5;
    
    // 정점 0부터 BFS 시작
    graph_bfs(graph, 0, record_visit, &record);
    
    // 모든 정점이 방문되었는지 확인
    ASSERT_EQUAL(record.count, 5, "BFS에서 모든 정점이 방문되어야 함");
    
    // 첫 번째 방문은 시작 정점이어야 함
    ASSERT_EQUAL(record.visited_order[0], 0, "BFS 첫 방문은 시작 정점이어야 함");
    
    // BFS의 특성: 거리 1인 정점들(1, 2)이 거리 2인 정점들(3, 4)보다 먼저 방문
    bool found_1_or_2 = false;
    bool found_3_or_4 = false;
    int pos_1_or_2 = -1, pos_3_or_4 = -1;
    
    for (int i = 1; i < record.count; i++) {  // 0 제외하고 확인
        int vertex = record.visited_order[i];
        if (vertex == 1 || vertex == 2) {
            if (!found_1_or_2) {
                found_1_or_2 = true;
                pos_1_or_2 = i;
            }
        } else if (vertex == 3 || vertex == 4) {
            if (!found_3_or_4) {
                found_3_or_4 = true;
                pos_3_or_4 = i;
            }
        }
    }
    
    if (found_1_or_2 && found_3_or_4) {
        ASSERT_TRUE(pos_1_or_2 < pos_3_or_4, "BFS에서 거리가 가까운 정점이 먼저 방문되어야 함");
    }
    
    free(record.visited_order);
    graph_destroy(graph);
    TEST_END();
}

// 그래프 무결성 검증 테스트
static void test_graph_verify_integrity() {
    TEST_START("Graph integrity verification");
    
    // 유효한 그래프
    graph_t* valid_graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(valid_graph);
    }
    graph_add_edge(valid_graph, 0, 1);
    graph_add_edge(valid_graph, 1, 2);
    
    int result = graph_verify_integrity(valid_graph);
    ASSERT_EQUAL(result, SCC_SUCCESS, "유효한 그래프의 무결성 검증이 성공해야 함");
    
    graph_destroy(valid_graph);
    
    // NULL 그래프
    result = graph_verify_integrity(NULL);
    ASSERT_EQUAL(result, SCC_ERROR_NULL_POINTER, "NULL 그래프는 오류를 반환해야 함");
    
    TEST_END();
}

// 간선 반복자 테스트
static void test_graph_edge_iterator() {
    TEST_START("Graph edge iterator");
    
    graph_t* graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(graph);
    }
    
    // 간선 추가
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    // 반복자 생성
    graph_edge_iterator_t* iter = graph_edge_iterator_create(graph);
    ASSERT_NOT_NULL(iter, "간선 반복자 생성이 성공해야 함");
    
    // 모든 간선 순회
    int edge_count = 0;
    int src, dest;
    bool edges_found[3][3] = {false};
    
    while (graph_edge_iterator_next(iter, &src, &dest)) {
        ASSERT_TRUE(src >= 0 && src < 3, "소스 정점이 유효해야 함");
        ASSERT_TRUE(dest >= 0 && dest < 3, "목적지 정점이 유효해야 함");
        
        edges_found[src][dest] = true;
        edge_count++;
    }
    
    ASSERT_EQUAL(edge_count, 3, "3개의 간선이 모두 순회되어야 함");
    ASSERT_TRUE(edges_found[0][1], "간선 0->1이 발견되어야 함");
    ASSERT_TRUE(edges_found[1][2], "간선 1->2가 발견되어야 함");
    ASSERT_TRUE(edges_found[2][0], "간선 2->0이 발견되어야 함");
    
    // 반복자 리셋 후 다시 순회
    graph_edge_iterator_reset(iter);
    int reset_count = 0;
    while (graph_edge_iterator_next(iter, &src, &dest)) {
        reset_count++;
    }
    ASSERT_EQUAL(reset_count, edge_count, "리셋 후 같은 수의 간선이 순회되어야 함");
    
    graph_edge_iterator_destroy(iter);
    graph_destroy(graph);
    TEST_END();
}

// 그래프 리사이징 테스트
static void test_graph_resize() {
    TEST_START("Graph resizing");
    
    graph_t* graph = graph_create(5);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(graph);
    }
    
    // 용량 확장
    int result = graph_resize(graph, 10);
    ASSERT_EQUAL(result, SCC_SUCCESS, "그래프 확장이 성공해야 함");
    
    // 확장 후에도 기존 정점들이 유효해야 함
    ASSERT_EQUAL(graph_get_vertex_count(graph), 3, "기존 정점 수가 유지되어야 함");
    
    // 새로운 정점 추가 가능
    int new_vertex = graph_add_vertex(graph);
    ASSERT_EQUAL(new_vertex, 3, "새 정점이 추가되어야 함");
    
    // 현재 정점 수보다 작게 리사이징은 실패해야 함
    result = graph_resize(graph, 2);
    ASSERT_EQUAL(result, SCC_ERROR_INVALID_PARAMETER, "현재 정점 수보다 작은 크기로 리사이징은 실패해야 함");
    
    // 같은 크기로 리사이징은 성공해야 함
    result = graph_resize(graph, 10);
    ASSERT_EQUAL(result, SCC_SUCCESS, "같은 크기로 리사이징은 성공해야 함");
    
    graph_destroy(graph);
    TEST_END();
}

// 순회 함수 엣지 케이스 테스트
static void test_traversal_edge_cases() {
    TEST_START("Traversal edge cases");
    
    graph_t* graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(graph);
    }
    
    visit_record_t record;
    record.visited_order = malloc(3 * sizeof(int));
    record.count = 0;
    record.capacity = 3;
    
    // NULL 그래프로 DFS/BFS 호출
    graph_dfs(NULL, 0, record_visit, &record);
    ASSERT_EQUAL(record.count, 0, "NULL 그래프에서는 방문이 없어야 함");
    
    // 잘못된 시작 정점으로 DFS/BFS 호출
    graph_dfs(graph, 5, record_visit, &record);
    ASSERT_EQUAL(record.count, 0, "잘못된 시작 정점에서는 방문이 없어야 함");
    
    // NULL 콜백으로 DFS/BFS 호출 (오류 발생해야 함)
    graph_dfs(graph, 0, NULL, &record);
    ASSERT_EQUAL(scc_get_last_error(), SCC_ERROR_INVALID_PARAMETER, "NULL 콜백은 오류를 발생시켜야 함");
    
    scc_clear_error();
    
    // 연결되지 않은 정점에서 시작
    record.count = 0;
    graph_dfs(graph, 2, record_visit, &record);  // 정점 2는 고립됨
    ASSERT_EQUAL(record.count, 1, "고립된 정점에서는 자기 자신만 방문해야 함");
    ASSERT_EQUAL(record.visited_order[0], 2, "방문된 정점은 시작 정점이어야 함");
    
    free(record.visited_order);
    graph_destroy(graph);
    TEST_END();
}

// 벤치마크 기능 테스트
static void test_benchmark_functionality() {
    TEST_START("Benchmark functionality");
    
    // 간단한 그래프 생성
    graph_t* graph = graph_create(10);
    for (int i = 0; i < 10; i++) {
        graph_add_vertex(graph);
    }
    
    // 사이클 생성
    for (int i = 0; i < 10; i++) {
        graph_add_edge(graph, i, (i + 1) % 10);
    }
    
    // 벤치마크 실행
    scc_benchmark_result_t* benchmark = scc_benchmark_algorithms(graph);
    ASSERT_NOT_NULL(benchmark, "벤치마크가 성공해야 함");
    
    // 결과 검증
    ASSERT_TRUE(benchmark->tarjan_time_ms >= 0, "Tarjan 실행 시간이 유효해야 함");
    ASSERT_TRUE(benchmark->kosaraju_time_ms >= 0, "Kosaraju 실행 시간이 유효해야 함");
    ASSERT_TRUE(benchmark->tarjan_memory_peak_bytes > 0, "Tarjan 메모리 사용량이 양수여야 함");
    ASSERT_TRUE(benchmark->kosaraju_memory_peak_bytes > 0, "Kosaraju 메모리 사용량이 양수여야 함");
    ASSERT_TRUE(benchmark->results_match, "두 알고리즘의 결과가 일치해야 함");
    
    scc_benchmark_result_destroy(benchmark);
    graph_destroy(graph);
    TEST_END();
}

// NULL 포인터 안전성 테스트
static void test_null_pointer_safety() {
    TEST_START("NULL pointer safety");
    
    // NULL 그래프로 각 함수 호출
    ASSERT_NULL(graph_edge_iterator_create(NULL), "NULL 그래프로 반복자 생성은 실패해야 함");
    
    graph_edge_iterator_destroy(NULL);  // 크래시 없이 처리되어야 함
    
    ASSERT_EQUAL(graph_resize(NULL, 10), SCC_ERROR_NULL_POINTER, "NULL 그래프 리사이징은 오류를 반환해야 함");
    
    ASSERT_NULL(scc_benchmark_algorithms(NULL), "NULL 그래프 벤치마크는 실패해야 함");
    
    scc_benchmark_result_destroy(NULL);  // 크래시 없이 처리되어야 함
    
    TEST_END();
}

// 모든 유틸리티 테스트 실행
void run_utils_tests() {
    printf("=== 유틸리티 모듈 테스트 ===\n");
    
    test_graph_dfs();
    test_graph_bfs();
    test_graph_verify_integrity();
    test_graph_edge_iterator();
    test_graph_resize();
    test_traversal_edge_cases();
    test_benchmark_functionality();
    test_null_pointer_safety();
    
    printf("유틸리티 모듈 테스트 완료\n\n");
}