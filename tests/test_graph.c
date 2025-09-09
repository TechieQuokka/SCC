#include "test_framework.h"
#include "../src/graph.h"
#include <assert.h>

// 그래프 생성 테스트
static void test_graph_create_destroy() {
    TEST_START("Graph creation and destruction");
    
    // 유효한 용량으로 그래프 생성
    graph_t* graph = graph_create(10);
    ASSERT_NOT_NULL(graph, "Graph creation should succeed");
    ASSERT_EQUAL(graph_get_vertex_count(graph), 0, "Initial vertex count should be 0");
    ASSERT_EQUAL(graph_get_edge_count(graph), 0, "Initial edge count should be 0");
    
    graph_destroy(graph);
    
    // 잘못된 용량으로 그래프 생성
    graph_t* null_graph = graph_create(0);
    ASSERT_NULL(null_graph, "Graph creation with 0 capacity should fail");
    
    TEST_END();
}

// 정점 추가 테스트
static void test_graph_add_vertex() {
    TEST_START("Vertex addition");
    
    graph_t* graph = graph_create(5);
    ASSERT_NOT_NULL(graph, "Graph creation should succeed");
    
    // 정점 추가
    int v1 = graph_add_vertex(graph);
    ASSERT_EQUAL(v1, 0, "First vertex should have ID 0");
    ASSERT_EQUAL(graph_get_vertex_count(graph), 1, "Vertex count should be 1");
    
    int v2 = graph_add_vertex(graph);
    ASSERT_EQUAL(v2, 1, "Second vertex should have ID 1");
    ASSERT_EQUAL(graph_get_vertex_count(graph), 2, "Vertex count should be 2");
    
    // 용량 초과 테스트
    for (int i = 2; i < 5; i++) {
        graph_add_vertex(graph);
    }
    ASSERT_EQUAL(graph_get_vertex_count(graph), 5, "Should have 5 vertices");
    
    int overflow = graph_add_vertex(graph);
    ASSERT_EQUAL(overflow, SCC_ERROR_GRAPH_FULL, "Adding vertex beyond capacity should fail");
    
    graph_destroy(graph);
    TEST_END();
}

// 간선 추가 테스트
static void test_graph_add_edge() {
    TEST_START("Edge addition");
    
    graph_t* graph = graph_create(3);
    
    // 정점 추가
    graph_add_vertex(graph);  // 0
    graph_add_vertex(graph);  // 1
    graph_add_vertex(graph);  // 2
    
    // 유효한 간선 추가
    int result = graph_add_edge(graph, 0, 1);
    ASSERT_EQUAL(result, SCC_SUCCESS, "Adding valid edge should succeed");
    ASSERT_EQUAL(graph_get_edge_count(graph), 1, "Edge count should be 1");
    ASSERT_TRUE(graph_has_edge(graph, 0, 1), "Edge 0->1 should exist");
    
    // 중복 간선 추가
    result = graph_add_edge(graph, 0, 1);
    ASSERT_EQUAL(result, SCC_ERROR_EDGE_EXISTS, "Adding duplicate edge should return edge exists");
    ASSERT_EQUAL(graph_get_edge_count(graph), 1, "Edge count should remain 1");
    
    // 자기 자신으로의 간선
    result = graph_add_edge(graph, 0, 0);
    ASSERT_EQUAL(result, SCC_SUCCESS, "Self-loop should be allowed");
    ASSERT_EQUAL(graph_get_edge_count(graph), 2, "Edge count should be 2");
    
    // 잘못된 정점 번호
    result = graph_add_edge(graph, 0, 5);
    ASSERT_EQUAL(result, SCC_ERROR_INVALID_VERTEX, "Invalid destination vertex should fail");
    
    result = graph_add_edge(graph, 5, 0);
    ASSERT_EQUAL(result, SCC_ERROR_INVALID_VERTEX, "Invalid source vertex should fail");
    
    graph_destroy(graph);
    TEST_END();
}

// 간선 제거 테스트
static void test_graph_remove_edge() {
    TEST_START("Edge removal");
    
    graph_t* graph = graph_create(3);
    graph_add_vertex(graph);  // 0
    graph_add_vertex(graph);  // 1
    graph_add_vertex(graph);  // 2
    
    // 간선 추가
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 0, 2);
    ASSERT_EQUAL(graph_get_edge_count(graph), 3, "Should have 3 edges");
    
    // 간선 제거
    int result = graph_remove_edge(graph, 0, 1);
    ASSERT_EQUAL(result, SCC_SUCCESS, "Edge removal should succeed");
    ASSERT_EQUAL(graph_get_edge_count(graph), 2, "Edge count should be 2");
    ASSERT_FALSE(graph_has_edge(graph, 0, 1), "Edge 0->1 should not exist");
    
    // 존재하지 않는 간선 제거
    result = graph_remove_edge(graph, 0, 1);
    ASSERT_EQUAL(result, SCC_ERROR_EDGE_NOT_FOUND, "Removing non-existent edge should fail");
    ASSERT_EQUAL(graph_get_edge_count(graph), 2, "Edge count should remain 2");
    
    graph_destroy(graph);
    TEST_END();
}

// 그래프 전치 테스트
static void test_graph_transpose() {
    TEST_START("Graph transpose");
    
    graph_t* graph = graph_create(3);
    graph_add_vertex(graph);  // 0
    graph_add_vertex(graph);  // 1
    graph_add_vertex(graph);  // 2
    
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 0, 2);
    
    // 전치 그래프 생성
    graph_t* transpose = graph_transpose(graph);
    ASSERT_NOT_NULL(transpose, "Transpose should succeed");
    ASSERT_EQUAL(graph_get_vertex_count(transpose), 3, "Transpose should have same vertex count");
    ASSERT_EQUAL(graph_get_edge_count(transpose), 3, "Transpose should have same edge count");
    
    // 전치된 간선 확인
    ASSERT_TRUE(graph_has_edge(transpose, 1, 0), "Transpose should have edge 1->0");
    ASSERT_TRUE(graph_has_edge(transpose, 2, 1), "Transpose should have edge 2->1");
    ASSERT_TRUE(graph_has_edge(transpose, 2, 0), "Transpose should have edge 2->0");
    
    // 원본 간선은 없어야 함
    ASSERT_FALSE(graph_has_edge(transpose, 0, 1), "Transpose should not have edge 0->1");
    ASSERT_FALSE(graph_has_edge(transpose, 1, 2), "Transpose should not have edge 1->2");
    ASSERT_FALSE(graph_has_edge(transpose, 0, 2), "Transpose should not have edge 0->2");
    
    graph_destroy(transpose);
    graph_destroy(graph);
    TEST_END();
}

// 그래프 유효성 검사 테스트
static void test_graph_validation() {
    TEST_START("Graph validation");
    
    graph_t* graph = graph_create(3);
    graph_add_vertex(graph);
    graph_add_vertex(graph);
    graph_add_edge(graph, 0, 1);
    
    ASSERT_TRUE(graph_is_valid(graph), "Valid graph should pass validation");
    
    // NULL 그래프
    ASSERT_FALSE(graph_is_valid(NULL), "NULL graph should fail validation");
    
    graph_destroy(graph);
    TEST_END();
}

// 그래프 복사 테스트
static void test_graph_copy() {
    TEST_START("Graph copy");
    
    graph_t* original = graph_create(3);
    graph_add_vertex(original);  // 0
    graph_add_vertex(original);  // 1
    graph_add_vertex(original);  // 2
    
    graph_add_edge(original, 0, 1);
    graph_add_edge(original, 1, 2);
    graph_add_edge(original, 2, 0);
    
    // 그래프 복사
    graph_t* copy = graph_copy(original);
    ASSERT_NOT_NULL(copy, "Graph copy should succeed");
    ASSERT_EQUAL(graph_get_vertex_count(copy), graph_get_vertex_count(original), "Copy should have same vertex count");
    ASSERT_EQUAL(graph_get_edge_count(copy), graph_get_edge_count(original), "Copy should have same edge count");
    
    // 모든 간선이 복사되었는지 확인
    ASSERT_TRUE(graph_has_edge(copy, 0, 1), "Copy should have edge 0->1");
    ASSERT_TRUE(graph_has_edge(copy, 1, 2), "Copy should have edge 1->2");
    ASSERT_TRUE(graph_has_edge(copy, 2, 0), "Copy should have edge 2->0");
    
    // 원본에 간선 추가해도 복사본에는 영향 없음
    graph_add_edge(original, 0, 2);
    ASSERT_FALSE(graph_has_edge(copy, 0, 2), "Copy should not be affected by original modification");
    
    graph_destroy(copy);
    graph_destroy(original);
    TEST_END();
}

// 모든 그래프 테스트 실행
void run_graph_tests() {
    printf("=== 그래프 모듈 테스트 ===\n");
    
    test_graph_create_destroy();
    test_graph_add_vertex();
    test_graph_add_edge();
    test_graph_remove_edge();
    test_graph_transpose();
    test_graph_validation();
    test_graph_copy();
    
    printf("그래프 모듈 테스트 완료\n\n");
}