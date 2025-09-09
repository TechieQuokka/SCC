#include "test_framework.h"
#include "../src/scc_algorithms.h"
#include "../src/graph.h"
#include <assert.h>

// Tarjan 알고리즘 기본 테스트
static void test_tarjan_basic() {
    TEST_START("Tarjan algorithm basic functionality");
    
    graph_t* graph = graph_create(4);
    for (int i = 0; i < 4; i++) {
        graph_add_vertex(graph);
    }
    
    // 간단한 사이클: 0->1->2->0, 3은 독립
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    scc_result_t* result = scc_find_tarjan(graph);
    ASSERT_NOT_NULL(result, "Tarjan 알고리즘이 성공해야 함");
    
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

// Tarjan 스택 연산 테스트
static void test_tarjan_stack_operations() {
    TEST_START("Tarjan stack operations");
    
    int num_vertices = 5;
    tarjan_state_t* state = tarjan_state_create(num_vertices);
    ASSERT_NOT_NULL(state, "Tarjan 상태 생성이 성공해야 함");
    
    // 스택이 비어있는지 확인
    ASSERT_TRUE(tarjan_stack_is_empty(state), "초기 스택이 비어있어야 함");
    
    // 요소 추가
    int result = tarjan_stack_push(state, 0);
    ASSERT_EQUAL(result, SCC_SUCCESS, "스택 push가 성공해야 함");
    ASSERT_FALSE(tarjan_stack_is_empty(state), "push 후 스택이 비어있지 않아야 함");
    ASSERT_TRUE(tarjan_stack_contains(state, 0), "스택에 요소 0이 있어야 함");
    
    // 여러 요소 추가
    tarjan_stack_push(state, 1);
    tarjan_stack_push(state, 2);
    ASSERT_TRUE(tarjan_stack_contains(state, 1), "스택에 요소 1이 있어야 함");
    ASSERT_TRUE(tarjan_stack_contains(state, 2), "스택에 요소 2가 있어야 함");
    ASSERT_FALSE(tarjan_stack_contains(state, 3), "스택에 요소 3이 없어야 함");
    
    // 요소 제거 (LIFO)
    int popped = tarjan_stack_pop(state);
    ASSERT_EQUAL(popped, 2, "마지막에 추가한 요소가 먼저 제거되어야 함");
    ASSERT_FALSE(tarjan_stack_contains(state, 2), "제거된 요소가 스택에 없어야 함");
    
    popped = tarjan_stack_pop(state);
    ASSERT_EQUAL(popped, 1, "두 번째로 마지막에 추가한 요소가 제거되어야 함");
    
    popped = tarjan_stack_pop(state);
    ASSERT_EQUAL(popped, 0, "첫 번째로 추가한 요소가 마지막에 제거되어야 함");
    
    ASSERT_TRUE(tarjan_stack_is_empty(state), "모든 요소 제거 후 스택이 비어있어야 함");
    
    tarjan_state_destroy(state);
    TEST_END();
}

// Tarjan 상태 생성/제거 테스트
static void test_tarjan_state_management() {
    TEST_START("Tarjan state management");
    
    // 유효한 상태 생성
    tarjan_state_t* state = tarjan_state_create(10);
    ASSERT_NOT_NULL(state, "유효한 크기로 상태 생성이 성공해야 함");
    ASSERT_NOT_NULL(state->result, "결과 구조가 초기화되어야 함");
    ASSERT_NOT_NULL(state->stack, "스택이 초기화되어야 함");
    ASSERT_NOT_NULL(state->vertices_processed, "정점 처리 상태가 초기화되어야 함");
    
    tarjan_state_destroy(state);
    
    // 잘못된 크기로 상태 생성
    tarjan_state_t* invalid_state = tarjan_state_create(0);
    ASSERT_NULL(invalid_state, "잘못된 크기로 상태 생성이 실패해야 함");
    
    invalid_state = tarjan_state_create(-1);
    ASSERT_NULL(invalid_state, "음수 크기로 상태 생성이 실패해야 함");
    
    TEST_END();
}

// 복잡한 그래프에서 Tarjan 테스트
static void test_tarjan_complex_graph() {
    TEST_START("Tarjan algorithm on complex graph");
    
    graph_t* graph = graph_create(8);
    for (int i = 0; i < 8; i++) {
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
    
    // SCC 3: 5->6->7->5
    graph_add_edge(graph, 5, 6);
    graph_add_edge(graph, 6, 7);
    graph_add_edge(graph, 7, 5);
    
    // SCC 간 연결
    graph_add_edge(graph, 2, 3);  // SCC1 -> SCC2
    graph_add_edge(graph, 4, 5);  // SCC2 -> SCC3
    graph_add_edge(graph, 1, 6);  // SCC1 -> SCC3 (직접 연결)
    
    scc_result_t* result = scc_find_tarjan(graph);
    ASSERT_NOT_NULL(result, "복잡한 그래프에서 Tarjan 알고리즘이 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(result), 3, "3개의 SCC가 있어야 함");
    
    // 각 SCC의 크기 확인
    bool found_size_3 = false, found_size_2 = false;
    for (int i = 0; i < 3; i++) {
        int size = scc_get_component_size(result, i);
        if (size == 3) found_size_3 = true;
        else if (size == 2) found_size_2 = true;
    }
    
    ASSERT_TRUE(found_size_3, "크기 3인 SCC가 2개 있어야 함");
    ASSERT_TRUE(found_size_2, "크기 2인 SCC가 1개 있어야 함");
    
    // 같은 SCC 내의 정점들 확인
    int comp0 = scc_get_vertex_component(result, 0);
    int comp1 = scc_get_vertex_component(result, 1);
    int comp2 = scc_get_vertex_component(result, 2);
    ASSERT_EQUAL(comp0, comp1, "정점 0, 1, 2가 같은 SCC에 속해야 함");
    ASSERT_EQUAL(comp1, comp2, "정점 0, 1, 2가 같은 SCC에 속해야 함");
    
    int comp3 = scc_get_vertex_component(result, 3);
    int comp4 = scc_get_vertex_component(result, 4);
    ASSERT_EQUAL(comp3, comp4, "정점 3, 4가 같은 SCC에 속해야 함");
    
    int comp5 = scc_get_vertex_component(result, 5);
    int comp6 = scc_get_vertex_component(result, 6);
    int comp7 = scc_get_vertex_component(result, 7);
    ASSERT_EQUAL(comp5, comp6, "정점 5, 6, 7이 같은 SCC에 속해야 함");
    ASSERT_EQUAL(comp6, comp7, "정점 5, 6, 7이 같은 SCC에 속해야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// Tarjan 알고리즘 성능 테스트 (간단)
static void test_tarjan_performance() {
    TEST_START("Tarjan algorithm performance");
    
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
    
    BENCHMARK_START("Tarjan on 1000-vertex cycle");
    scc_result_t* result = scc_find_tarjan(graph);
    BENCHMARK_END();
    
    ASSERT_NOT_NULL(result, "큰 그래프에서 Tarjan 알고리즘이 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 1, "하나의 큰 SCC가 있어야 함");
    ASSERT_EQUAL(scc_get_component_size(result, 0), size, "SCC 크기가 전체 정점 수와 같아야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// 엣지 케이스 테스트
static void test_tarjan_edge_cases() {
    TEST_START("Tarjan algorithm edge cases");
    
    // 자기 루프만 있는 정점
    graph_t* self_loop_graph = graph_create(1);
    graph_add_vertex(self_loop_graph);
    graph_add_edge(self_loop_graph, 0, 0);
    
    scc_result_t* result = scc_find_tarjan(self_loop_graph);
    ASSERT_NOT_NULL(result, "자기 루프가 있는 그래프에서 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 1, "하나의 SCC가 있어야 함");
    ASSERT_EQUAL(scc_get_component_size(result, 0), 1, "SCC 크기가 1이어야 함");
    
    scc_result_destroy(result);
    graph_destroy(self_loop_graph);
    
    // 간선이 없는 그래프
    graph_t* no_edge_graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(no_edge_graph);
    }
    
    result = scc_find_tarjan(no_edge_graph);
    ASSERT_NOT_NULL(result, "간선이 없는 그래프에서 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 3, "3개의 개별 SCC가 있어야 함");
    
    for (int i = 0; i < 3; i++) {
        ASSERT_EQUAL(scc_get_component_size(result, i), 1, "각 SCC 크기가 1이어야 함");
    }
    
    scc_result_destroy(result);
    graph_destroy(no_edge_graph);
    
    TEST_END();
}

// 모든 Tarjan 테스트 실행
void run_tarjan_tests() {
    printf("=== Tarjan 알고리즘 테스트 ===\n");
    
    test_tarjan_basic();
    test_tarjan_stack_operations();
    test_tarjan_state_management();
    test_tarjan_complex_graph();
    test_tarjan_performance();
    test_tarjan_edge_cases();
    
    printf("Tarjan 알고리즘 테스트 완료\n\n");
}