#include "test_framework.h"
#include "../src/scc.h"
#include "../src/graph.h"
#include <assert.h>

// 간단한 SCC 테스트 (단일 컴포넌트)
static void test_single_component() {
    TEST_START("Single strongly connected component");
    
    graph_t* graph = graph_create(3);
    graph_add_vertex(graph);  // 0
    graph_add_vertex(graph);  // 1
    graph_add_vertex(graph);  // 2
    
    // 강한 연결 구조: 0->1->2->0
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    scc_result_t* result = scc_find(graph);
    ASSERT_NOT_NULL(result, "SCC 찾기가 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(result), 1, "하나의 SCC가 있어야 함");
    ASSERT_EQUAL(scc_get_component_size(result, 0), 3, "SCC 크기가 3이어야 함");
    
    // 모든 정점이 같은 컴포넌트에 속해야 함
    int comp0 = scc_get_vertex_component(result, 0);
    int comp1 = scc_get_vertex_component(result, 1);
    int comp2 = scc_get_vertex_component(result, 2);
    
    ASSERT_EQUAL(comp0, comp1, "정점 0과 1이 같은 컴포넌트에 속해야 함");
    ASSERT_EQUAL(comp1, comp2, "정점 1과 2가 같은 컴포넌트에 속해야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// 여러 SCC 테스트
static void test_multiple_components() {
    TEST_START("Multiple strongly connected components");
    
    graph_t* graph = graph_create(6);
    for (int i = 0; i < 6; i++) {
        graph_add_vertex(graph);
    }
    
    // 첫 번째 SCC: 0->1->0
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 0);
    
    // 두 번째 SCC: 2->3->4->2
    graph_add_edge(graph, 2, 3);
    graph_add_edge(graph, 3, 4);
    graph_add_edge(graph, 4, 2);
    
    // 세 번째 SCC: 5 (자기 자신만)
    
    // 컴포넌트 간 연결
    graph_add_edge(graph, 1, 2);  // 첫 번째 -> 두 번째
    graph_add_edge(graph, 4, 5);  // 두 번째 -> 세 번째
    
    scc_result_t* result = scc_find(graph);
    ASSERT_NOT_NULL(result, "SCC 찾기가 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(result), 3, "3개의 SCC가 있어야 함");
    
    // 첫 번째 컴포넌트 (0, 1)
    int comp0 = scc_get_vertex_component(result, 0);
    int comp1 = scc_get_vertex_component(result, 1);
    ASSERT_EQUAL(comp0, comp1, "정점 0과 1이 같은 컴포넌트에 속해야 함");
    
    // 두 번째 컴포넌트 (2, 3, 4)
    int comp2 = scc_get_vertex_component(result, 2);
    int comp3 = scc_get_vertex_component(result, 3);
    int comp4 = scc_get_vertex_component(result, 4);
    ASSERT_EQUAL(comp2, comp3, "정점 2와 3이 같은 컴포넌트에 속해야 함");
    ASSERT_EQUAL(comp3, comp4, "정점 3과 4가 같은 컴포넌트에 속해야 함");
    
    // 세 번째 컴포넌트 (5)
    int comp5 = scc_get_vertex_component(result, 5);
    
    // 모든 컴포넌트가 다른지 확인
    ASSERT_NOT_EQUAL(comp0, comp2, "첫 번째와 두 번째 컴포넌트는 달라야 함");
    ASSERT_NOT_EQUAL(comp2, comp5, "두 번째와 세 번째 컴포넌트는 달라야 함");
    ASSERT_NOT_EQUAL(comp0, comp5, "첫 번째와 세 번째 컴포넌트는 달라야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// 단일 정점 테스트
static void test_single_vertex() {
    TEST_START("Single vertex graph");
    
    graph_t* graph = graph_create(1);
    graph_add_vertex(graph);  // 0
    
    scc_result_t* result = scc_find(graph);
    ASSERT_NOT_NULL(result, "SCC 찾기가 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(result), 1, "하나의 SCC가 있어야 함");
    ASSERT_EQUAL(scc_get_component_size(result, 0), 1, "SCC 크기가 1이어야 함");
    ASSERT_EQUAL(scc_get_vertex_component(result, 0), 0, "정점 0이 컴포넌트 0에 속해야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// 자기 루프가 있는 경우 테스트
static void test_self_loops() {
    TEST_START("Graph with self loops");
    
    graph_t* graph = graph_create(3);
    graph_add_vertex(graph);  // 0
    graph_add_vertex(graph);  // 1
    graph_add_vertex(graph);  // 2
    
    // 자기 루프들
    graph_add_edge(graph, 0, 0);
    graph_add_edge(graph, 1, 1);
    graph_add_edge(graph, 2, 2);
    
    // 다른 정점으로의 간선들
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    
    scc_result_t* result = scc_find(graph);
    ASSERT_NOT_NULL(result, "SCC 찾기가 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(result), 3, "3개의 SCC가 있어야 함");
    
    // 각 정점이 별도의 컴포넌트에 속해야 함 (자기 루프만 있으므로)
    int comp0 = scc_get_vertex_component(result, 0);
    int comp1 = scc_get_vertex_component(result, 1);
    int comp2 = scc_get_vertex_component(result, 2);
    
    ASSERT_NOT_EQUAL(comp0, comp1, "정점 0과 1이 다른 컴포넌트에 속해야 함");
    ASSERT_NOT_EQUAL(comp1, comp2, "정점 1과 2가 다른 컴포넌트에 속해야 함");
    ASSERT_NOT_EQUAL(comp0, comp2, "정점 0과 2가 다른 컴포넌트에 속해야 함");
    
    scc_result_destroy(result);
    graph_destroy(graph);
    TEST_END();
}

// 빈 그래프 테스트
static void test_empty_graph() {
    TEST_START("Empty graph");
    
    graph_t* graph = graph_create(5);
    // 정점을 추가하지 않음
    
    scc_result_t* result = scc_find(graph);
    // 빈 그래프에 대해서는 적절한 처리가 필요
    // 구현에 따라 NULL을 반환하거나 빈 결과를 반환할 수 있음
    
    if (result) {
        ASSERT_EQUAL(scc_get_component_count(result), 0, "빈 그래프는 0개의 컴포넌트를 가져야 함");
        scc_result_destroy(result);
    }
    
    graph_destroy(graph);
    TEST_END();
}

// SCC 결과 복사 테스트
static void test_scc_result_copy() {
    TEST_START("SCC result copy");
    
    graph_t* graph = graph_create(4);
    for (int i = 0; i < 4; i++) {
        graph_add_vertex(graph);
    }
    
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 0);
    graph_add_edge(graph, 2, 3);
    graph_add_edge(graph, 3, 2);
    
    scc_result_t* original = scc_find(graph);
    ASSERT_NOT_NULL(original, "SCC 찾기가 성공해야 함");
    
    scc_result_t* copy = scc_result_copy(original);
    ASSERT_NOT_NULL(copy, "SCC 결과 복사가 성공해야 함");
    
    ASSERT_EQUAL(scc_get_component_count(copy), scc_get_component_count(original), 
                 "복사본의 컴포넌트 개수가 같아야 함");
    
    // 각 정점의 컴포넌트가 같은지 확인
    for (int i = 0; i < 4; i++) {
        ASSERT_EQUAL(scc_get_vertex_component(copy, i), scc_get_vertex_component(original, i),
                     "복사본의 정점 컴포넌트가 같아야 함");
    }
    
    scc_result_destroy(copy);
    scc_result_destroy(original);
    graph_destroy(graph);
    TEST_END();
}

// 강한 연결성 확인 테스트
static void test_is_strongly_connected() {
    TEST_START("Strong connectivity check");
    
    // 강한 연결 그래프
    graph_t* connected_graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(connected_graph);
    }
    
    graph_add_edge(connected_graph, 0, 1);
    graph_add_edge(connected_graph, 1, 2);
    graph_add_edge(connected_graph, 2, 0);
    
    ASSERT_TRUE(scc_is_strongly_connected(connected_graph), 
                "완전히 연결된 그래프는 강한 연결이어야 함");
    
    graph_destroy(connected_graph);
    
    // 강한 연결이 아닌 그래프
    graph_t* disconnected_graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(disconnected_graph);
    }
    
    graph_add_edge(disconnected_graph, 0, 1);
    graph_add_edge(disconnected_graph, 1, 2);
    // 2에서 0으로의 간선이 없음
    
    ASSERT_FALSE(scc_is_strongly_connected(disconnected_graph), 
                 "비연결된 그래프는 강한 연결이 아니어야 함");
    
    graph_destroy(disconnected_graph);
    TEST_END();
}

// 축약 그래프 생성 테스트
static void test_condensation_graph() {
    TEST_START("Condensation graph creation");
    
    graph_t* graph = graph_create(6);
    for (int i = 0; i < 6; i++) {
        graph_add_vertex(graph);
    }
    
    // SCC 1: 0->1->0
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 0);
    
    // SCC 2: 2->3->2
    graph_add_edge(graph, 2, 3);
    graph_add_edge(graph, 3, 2);
    
    // SCC 3: 4, 5 (개별)
    
    // 컴포넌트 간 간선
    graph_add_edge(graph, 1, 2);  // SCC1 -> SCC2
    graph_add_edge(graph, 3, 4);  // SCC2 -> SCC3
    graph_add_edge(graph, 4, 5);  // SCC3 -> SCC4
    
    scc_result_t* scc_result = scc_find(graph);
    ASSERT_NOT_NULL(scc_result, "SCC 찾기가 성공해야 함");
    
    graph_t* condensed = scc_build_condensation_graph(graph, scc_result);
    ASSERT_NOT_NULL(condensed, "축약 그래프 생성이 성공해야 함");
    
    ASSERT_EQUAL(graph_get_vertex_count(condensed), scc_get_component_count(scc_result),
                 "축약 그래프의 정점 수가 SCC 개수와 같아야 함");
    
    // 축약 그래프는 DAG여야 함 (사이클이 없어야 함)
    // 이는 별도의 함수로 검증할 수 있음
    
    graph_destroy(condensed);
    scc_result_destroy(scc_result);
    graph_destroy(graph);
    TEST_END();
}

// 모든 SCC 테스트 실행
void run_scc_tests() {
    printf("=== SCC 모듈 테스트 ===\n");
    
    test_single_component();
    test_multiple_components();
    test_single_vertex();
    test_self_loops();
    test_empty_graph();
    test_scc_result_copy();
    test_is_strongly_connected();
    test_condensation_graph();
    
    printf("SCC 모듈 테스트 완료\n\n");
}