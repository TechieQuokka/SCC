#include "test_framework.h"
#include "../src/scc_algorithms.h"
#include "../src/graph.h"
#include <assert.h>

// Kosaraju 알고리즘 기본 테스트
static void test_kosaraju_basic() {
    TEST_START("Kosaraju algorithm basic functionality");
    
    graph_t* graph = graph_create(4);
    for (int i = 0; i < 4; i++) {
        graph_add_vertex(graph);
    }
    
    // 간단한 사이클: 0->1->2->0, 3은 독립
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    scc_result_t* result = scc_find_kosaraju(graph);
    ASSERT_NOT_NULL(result, "Kosaraju 알고리즘이 성공해야 함");
    
    // 2개의 SCC가 있어야 함: {0, 1, 2}, {3}
    ASSERT_EQUAL(scc_get_component_count(result), 2, "2개의 SCC가 있어야 함");
    
    // 첫 번째 SCC 확인
    int comp0 = scc_get_vertex_component(result, 0);
    int comp1 = scc_get_vertex_component(result, 1);
    int comp2 = scc_get_vertex_component(result, 2);
    int comp3 = scc_get_vertex_component(result, 3);
    
    ASSERT_EQUAL(comp0, comp1, "정점 0과 1이 같은 컴포넌트에 속해야 함");
    ASSERT_EQUAL(comp1, comp2, "정점 1과 2가 같은 컴포넌트에 속해야 함");
    ASSERT_NOT_EQUAL(comp0, comp3, "정점 0과 3이 다른 컴포넌트에 속해야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// Kosaraju 상태 관리 테스트
static void test_kosaraju_state_management() {
    TEST_START("Kosaraju state management");
    
    // 유효한 상태 생성
    kosaraju_state_t* state = kosaraju_state_create(10);
    ASSERT_NOT_NULL(state, "유효한 크기로 상태 생성이 성공해야 함");
    ASSERT_NOT_NULL(state->result, "결과 구조가 초기화되어야 함");
    ASSERT_NOT_NULL(state->finish_order, "완료 순서 배열이 초기화되어야 함");
    ASSERT_NOT_NULL(state->visited_first_pass, "첫 번째 방문 배열이 초기화되어야 함");
    ASSERT_NOT_NULL(state->visited_second_pass, "두 번째 방문 배열이 초기화되어야 함");
    ASSERT_EQUAL(state->finish_index, 0, "완료 인덱스가 0으로 초기화되어야 함");
    ASSERT_EQUAL(state->current_component, 0, "현재 컴포넌트가 0으로 초기화되어야 함");
    
    kosaraju_state_destroy(state);
    
    // 잘못된 크기로 상태 생성
    kosaraju_state_t* invalid_state = kosaraju_state_create(0);
    ASSERT_NULL(invalid_state, "잘못된 크기로 상태 생성이 실패해야 함");
    
    invalid_state = kosaraju_state_create(-1);
    ASSERT_NULL(invalid_state, "음수 크기로 상태 생성이 실패해야 함");
    
    TEST_END();
}

// Kosaraju와 Tarjan 결과 비교 테스트
static void test_kosaraju_vs_tarjan() {
    TEST_START("Kosaraju vs Tarjan results comparison");
    
    graph_t* graph = graph_create(6);
    for (int i = 0; i < 6; i++) {
        graph_add_vertex(graph);
    }
    
    // 복잡한 그래프 구성
    // SCC 1: 0->1->0
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 0);
    
    // SCC 2: 2->3->4->2
    graph_add_edge(graph, 2, 3);
    graph_add_edge(graph, 3, 4);
    graph_add_edge(graph, 4, 2);
    
    // SCC 3: 5 (독립)
    
    // 컴포넌트 간 연결
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 4, 5);
    
    scc_result_t* kosaraju_result = scc_find_kosaraju(graph);
    scc_result_t* tarjan_result = scc_find_tarjan(graph);
    
    ASSERT_NOT_NULL(kosaraju_result, "Kosaraju 결과가 NULL이 아니어야 함");
    ASSERT_NOT_NULL(tarjan_result, "Tarjan 결과가 NULL이 아니어야 함");
    
    // 컴포넌트 개수가 같아야 함
    ASSERT_EQUAL(scc_get_component_count(kosaraju_result), 
                 scc_get_component_count(tarjan_result),
                 "두 알고리즘의 컴포넌트 개수가 같아야 함");
    
    // 각 정점이 같은 정점들과 같은 컴포넌트에 속하는지 확인
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 6; j++) {
            int kosaraju_same = (scc_get_vertex_component(kosaraju_result, i) == 
                                scc_get_vertex_component(kosaraju_result, j));
            int tarjan_same = (scc_get_vertex_component(tarjan_result, i) == 
                              scc_get_vertex_component(tarjan_result, j));
            ASSERT_EQUAL(kosaraju_same, tarjan_same, 
                        "두 알고리즘에서 정점 쌍의 컴포넌트 관계가 같아야 함");
        }
    }
    
    scc_result_destroy(kosaraju_result);
    scc_result_destroy(tarjan_result);
    graph_destroy(graph);
    TEST_END();
}

// 전치 그래프 테스트
static void test_kosaraju_transpose() {
    TEST_START("Kosaraju transpose graph functionality");
    
    graph_t* original = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(original);
    }
    
    graph_add_edge(original, 0, 1);
    graph_add_edge(original, 1, 2);
    graph_add_edge(original, 2, 0);
    
    // 내부적으로 전치 그래프가 올바르게 생성되는지 확인하기 위해
    // kosaraju 알고리즘을 실행하고 결과를 검증
    scc_result_t* result = scc_find_kosaraju(original);
    ASSERT_NOT_NULL(result, "Kosaraju 알고리즘이 성공해야 함");
    
    // 모든 정점이 하나의 SCC에 속해야 함 (사이클이므로)
    ASSERT_EQUAL(scc_get_component_count(result), 1, "하나의 SCC가 있어야 함");
    ASSERT_EQUAL(scc_get_component_size(result, 0), 3, "SCC 크기가 3이어야 함");
    
    scc_result_destroy(result);
    graph_destroy(original);
    TEST_END();
}

// Kosaraju DFS 함수 테스트
static void test_kosaraju_dfs_functions() {
    TEST_START("Kosaraju DFS functions");
    
    graph_t* graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(graph);
    }
    
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    
    kosaraju_state_t* state = kosaraju_state_create(3);
    ASSERT_NOT_NULL(state, "상태 생성이 성공해야 함");
    
    // 첫 번째 DFS 테스트
    kosaraju_dfs_first(graph, 0, state);
    
    // 정점 0이 방문되었는지 확인
    ASSERT_TRUE(state->visited_first_pass[0], "정점 0이 방문되어야 함");
    
    // 완료 순서에 정점들이 추가되었는지 확인
    ASSERT_TRUE(state->finish_index > 0, "완료 순서에 정점들이 추가되어야 함");
    
    kosaraju_state_destroy(state);
    graph_destroy(graph);
    TEST_END();
}

// 큰 그래프에서 Kosaraju 성능 테스트
static void test_kosaraju_performance() {
    TEST_START("Kosaraju algorithm performance");
    
    // 큰 선형 그래프 생성
    int size = 1000;
    graph_t* graph = graph_create(size);
    
    for (int i = 0; i < size; i++) {
        graph_add_vertex(graph);
    }
    
    // 선형 체인: 0->1->2->...->999->0
    for (int i = 0; i < size - 1; i++) {
        graph_add_edge(graph, i, i + 1);
    }
    graph_add_edge(graph, size - 1, 0);  // 사이클 완성
    
    BENCHMARK_START("Kosaraju on 1000-vertex cycle");
    scc_result_t* result = scc_find_kosaraju(graph);
    BENCHMARK_END();
    
    ASSERT_NOT_NULL(result, "큰 그래프에서 Kosaraju 알고리즘이 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 1, "하나의 큰 SCC가 있어야 함");
    ASSERT_EQUAL(scc_get_component_size(result, 0), size, "SCC 크기가 전체 정점 수와 같아야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// 복잡한 그래프에서 Kosaraju 테스트
static void test_kosaraju_complex_graph() {
    TEST_START("Kosaraju algorithm on complex graph");
    
    graph_t* graph = graph_create(7);
    for (int i = 0; i < 7; i++) {
        graph_add_vertex(graph);
    }
    
    // 여러 개의 SCC를 가진 복잡한 그래프
    // SCC 1: 0->1->2->0
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    // SCC 2: 3->4->3
    graph_add_edge(graph, 3, 4);
    graph_add_edge(graph, 4, 3);
    
    // SCC 3: 5 (독립)
    // SCC 4: 6 (독립)
    
    // SCC 간 연결
    graph_add_edge(graph, 2, 3);  // SCC1 -> SCC2
    graph_add_edge(graph, 4, 5);  // SCC2 -> SCC3
    graph_add_edge(graph, 1, 6);  // SCC1 -> SCC4
    
    scc_result_t* result = scc_find_kosaraju(graph);
    ASSERT_NOT_NULL(result, "복잡한 그래프에서 Kosaraju 알고리즘이 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(result), 4, "4개의 SCC가 있어야 함");
    
    // 각 SCC의 크기 확인
    int size_counts[4] = {0}; // 크기별 개수
    for (int i = 0; i < 4; i++) {
        int size = scc_get_component_size(result, i);
        ASSERT_TRUE(size >= 1 && size <= 3, "SCC 크기가 1~3 사이여야 함");
        if (size == 1) size_counts[0]++;
        else if (size == 2) size_counts[1]++;
        else if (size == 3) size_counts[2]++;
    }
    
    ASSERT_EQUAL(size_counts[2], 1, "크기 3인 SCC가 1개 있어야 함");  // {0,1,2}
    ASSERT_EQUAL(size_counts[1], 1, "크기 2인 SCC가 1개 있어야 함");  // {3,4}
    ASSERT_EQUAL(size_counts[0], 2, "크기 1인 SCC가 2개 있어야 함");  // {5}, {6}
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// Kosaraju 엣지 케이스 테스트
static void test_kosaraju_edge_cases() {
    TEST_START("Kosaraju algorithm edge cases");
    
    // 자기 루프만 있는 정점
    graph_t* self_loop_graph = graph_create(2);
    graph_add_vertex(self_loop_graph);
    graph_add_vertex(self_loop_graph);
    graph_add_edge(self_loop_graph, 0, 0);
    graph_add_edge(self_loop_graph, 1, 1);
    
    scc_result_t* result = scc_find_kosaraju(self_loop_graph);
    ASSERT_NOT_NULL(result, "자기 루프가 있는 그래프에서 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 2, "2개의 SCC가 있어야 함");
    
    for (int i = 0; i < 2; i++) {
        ASSERT_EQUAL(scc_get_component_size(result, i), 1, "각 SCC 크기가 1이어야 함");
    }
    
    scc_result_destroy(result);
    graph_destroy(self_loop_graph);
    
    // 간선이 없는 그래프
    graph_t* no_edge_graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(no_edge_graph);
    }
    
    result = scc_find_kosaraju(no_edge_graph);
    ASSERT_NOT_NULL(result, "간선이 없는 그래프에서 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 3, "3개의 개별 SCC가 있어야 함");
    
    for (int i = 0; i < 3; i++) {
        ASSERT_EQUAL(scc_get_component_size(result, i), 1, "각 SCC 크기가 1이어야 함");
    }
    
    scc_result_destroy(result);
    graph_destroy(no_edge_graph);
    
    TEST_END();
}

// 모든 Kosaraju 테스트 실행
void run_kosaraju_tests() {
    printf("=== Kosaraju 알고리즘 테스트 ===\n");
    
    test_kosaraju_basic();
    test_kosaraju_state_management();
    test_kosaraju_vs_tarjan();
    test_kosaraju_transpose();
    test_kosaraju_dfs_functions();
    test_kosaraju_performance();
    test_kosaraju_complex_graph();
    test_kosaraju_edge_cases();
    
    printf("Kosaraju 알고리즘 테스트 완료\n\n");
}