#include "test_framework.h"
#include "../src/scc.h"
#include "../src/graph.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 전체 워크플로우 테스트 (그래프 생성 -> SCC 찾기 -> 결과 분석 -> 파일 I/O)
static void test_complete_workflow() {
    TEST_START("Complete SCC workflow");
    
    // 1. 복잡한 그래프 생성
    graph_t* graph = graph_create(8);
    for (int i = 0; i < 8; i++) {
        graph_add_vertex(graph);
    }
    
    // 여러 SCC를 가진 그래프 구성
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
    
    // 컴포넌트 간 연결
    graph_add_edge(graph, 2, 3);  // SCC1 -> SCC2
    graph_add_edge(graph, 4, 5);  // SCC2 -> SCC3
    graph_add_edge(graph, 1, 6);  // SCC1 -> SCC3 (교차 간선)
    
    // 2. SCC 찾기 (자동 알고리즘 선택)
    scc_result_t* result = scc_find(graph);
    ASSERT_NOT_NULL(result, "SCC 찾기가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 3, "3개의 SCC가 있어야 함");
    
    // 3. 결과 분석
    ASSERT_EQUAL(result->largest_component_size, 3, "가장 큰 SCC 크기가 3이어야 함");
    ASSERT_EQUAL(result->smallest_component_size, 2, "가장 작은 SCC 크기가 2여야 함");
    
    double expected_avg = 8.0 / 3.0;  // 8개 정점, 3개 컴포넌트
    ASSERT_DOUBLE_EQUAL(result->average_component_size, expected_avg, 0.01, 
                        "평균 컴포넌트 크기가 올바라야 함");
    
    // 4. 축약 그래프 생성
    graph_t* condensed = scc_build_condensation_graph(graph, result);
    ASSERT_NOT_NULL(condensed, "축약 그래프 생성이 성공해야 함");
    ASSERT_EQUAL(graph_get_vertex_count(condensed), 3, "축약 그래프가 3개 정점을 가져야 함");
    
    // 축약 그래프는 DAG여야 함 (사이클 없음)
    scc_result_t* condensed_scc = scc_find(condensed);
    ASSERT_NOT_NULL(condensed_scc, "축약 그래프 SCC 찾기가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(condensed_scc), 3, 
                 "축약 그래프의 각 정점이 별도 SCC여야 함 (DAG)");
    
    // 5. 파일 I/O 테스트
    char filename[] = "integration_test_graph.txt";
    int save_result = graph_save_to_file(graph, filename, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(save_result, SCC_SUCCESS, "그래프 저장이 성공해야 함");
    
    graph_t* loaded_graph = NULL;
    int load_result = graph_load_from_file(&loaded_graph, filename, GRAPH_FORMAT_EDGE_LIST);
    ASSERT_EQUAL(load_result, SCC_SUCCESS, "그래프 로드가 성공해야 함");
    ASSERT_NOT_NULL(loaded_graph, "로드된 그래프가 NULL이 아니어야 함");
    
    // 6. 로드된 그래프에서 동일한 SCC 결과 확인
    scc_result_t* loaded_result = scc_find(loaded_graph);
    ASSERT_NOT_NULL(loaded_result, "로드된 그래프에서 SCC 찾기가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(loaded_result), 
                 scc_get_component_count(result),
                 "원본과 로드된 그래프의 SCC 개수가 같아야 함");
    
    // 정리
    remove(filename);
    scc_result_destroy(loaded_result);
    graph_destroy(loaded_graph);
    scc_result_destroy(condensed_scc);
    graph_destroy(condensed);
    scc_result_destroy(result);
    graph_destroy(graph);
    
    TEST_END();
}

// 알고리즘 일관성 테스트
static void test_algorithm_consistency() {
    TEST_START("Algorithm consistency across different graphs");
    
    // 여러 다른 그래프 패턴에 대해 Tarjan과 Kosaraju 결과 비교
    
    struct {
        const char* name;
        int vertices;
        int edges[][2];
        int edge_count;
    } test_graphs[] = {
        {
            "단일 사이클",
            4,
            {{0, 1}, {1, 2}, {2, 3}, {3, 0}},
            4
        },
        {
            "두 개의 분리된 사이클",
            6,
            {{0, 1}, {1, 2}, {2, 0}, {3, 4}, {4, 5}, {5, 3}},
            6
        },
        {
            "선형 체인",
            5,
            {{0, 1}, {1, 2}, {2, 3}, {3, 4}},
            5
        },
        {
            "완전 그래프 (작은)",
            4,
            {{0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 2}, {1, 3}, 
             {2, 0}, {2, 1}, {2, 3}, {3, 0}, {3, 1}, {3, 2}},
            12
        },
        {
            "별 모양",
            5,
            {{0, 1}, {0, 2}, {0, 3}, {0, 4}},
            4
        }
    };
    
    for (size_t t = 0; t < sizeof(test_graphs) / sizeof(test_graphs[0]); t++) {
        // 그래프 생성
        graph_t* graph = graph_create(test_graphs[t].vertices);
        for (int i = 0; i < test_graphs[t].vertices; i++) {
            graph_add_vertex(graph);
        }
        
        for (int i = 0; i < test_graphs[t].edge_count; i++) {
            graph_add_edge(graph, test_graphs[t].edges[i][0], test_graphs[t].edges[i][1]);
        }
        
        // 두 알고리즘으로 SCC 찾기
        scc_result_t* tarjan_result = scc_find_tarjan(graph);
        scc_result_t* kosaraju_result = scc_find_kosaraju(graph);
        
        ASSERT_NOT_NULL(tarjan_result, "Tarjan 결과가 NULL이 아니어야 함");
        ASSERT_NOT_NULL(kosaraju_result, "Kosaraju 결과가 NULL이 아니어야 함");
        
        // 결과 비교
        ASSERT_EQUAL(scc_get_component_count(tarjan_result),
                     scc_get_component_count(kosaraju_result),
                     "두 알고리즘의 컴포넌트 개수가 같아야 함");
        
        // 각 정점 쌍이 같은 컴포넌트에 속하는지 확인
        for (int i = 0; i < test_graphs[t].vertices; i++) {
            for (int j = i + 1; j < test_graphs[t].vertices; j++) {
                bool tarjan_same = (scc_get_vertex_component(tarjan_result, i) == 
                                   scc_get_vertex_component(tarjan_result, j));
                bool kosaraju_same = (scc_get_vertex_component(kosaraju_result, i) == 
                                     scc_get_vertex_component(kosaraju_result, j));
                ASSERT_EQUAL(tarjan_same, kosaraju_same, 
                            "정점 쌍의 컴포넌트 관계가 두 알고리즘에서 같아야 함");
            }
        }
        
        // 정리
        scc_result_destroy(kosaraju_result);
        scc_result_destroy(tarjan_result);
        graph_destroy(graph);
    }
    
    TEST_END();
}

// 메모리 관리 통합 테스트
static void test_memory_management_integration() {
    TEST_START("Memory management integration");
    
    // 메모리 풀을 사용한 그래프 연산
    memory_pool_t* pool = memory_pool_create(4096, 8);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    // 여러 그래프와 SCC 결과를 동시에 관리
    const int num_graphs = 5;
    graph_t* graphs[num_graphs];
    scc_result_t* results[num_graphs];
    
    // 각기 다른 크기의 그래프 생성
    for (int g = 0; g < num_graphs; g++) {
        int size = (g + 1) * 3;  // 3, 6, 9, 12, 15 정점
        graphs[g] = graph_create(size);
        
        for (int i = 0; i < size; i++) {
            graph_add_vertex(graphs[g]);
        }
        
        // 간단한 사이클 생성
        for (int i = 0; i < size; i++) {
            graph_add_edge(graphs[g], i, (i + 1) % size);
        }
        
        results[g] = scc_find(graphs[g]);
        ASSERT_NOT_NULL(results[g], "SCC 찾기가 성공해야 함");
        ASSERT_EQUAL(scc_get_component_count(results[g]), 1, 
                     "각 그래프는 하나의 큰 SCC를 가져야 함");
    }
    
    // 메모리 사용량 확인
    size_t used_before_cleanup = memory_pool_get_used_size(pool);
    
    // 모든 결과 정리
    for (int g = 0; g < num_graphs; g++) {
        scc_result_destroy(results[g]);
        graph_destroy(graphs[g]);
    }
    
    // 메모리 풀 정리
    memory_pool_destroy(pool);
    
    // 오류 상태 확인
    ASSERT_EQUAL(scc_get_last_error(), SCC_SUCCESS, 
                 "모든 작업 완료 후 오류가 없어야 함");
    
    TEST_END();
}

// 대용량 데이터 처리 테스트
static void test_large_scale_processing() {
    TEST_START("Large scale data processing");
    
    // 대용량 그래프 생성 (1000개 정점)
    int size = 1000;
    graph_t* large_graph = graph_create(size);
    
    for (int i = 0; i < size; i++) {
        graph_add_vertex(large_graph);
    }
    
    // 여러 SCC를 가진 구조 생성
    // - 10개의 100-정점 컴포넌트
    // - 각 컴포넌트 내에서는 강하게 연결됨
    // - 컴포넌트 간에는 단방향 연결
    
    for (int comp = 0; comp < 10; comp++) {
        int start = comp * 100;
        int end = start + 100;
        
        // 컴포넌트 내부를 강하게 연결
        for (int i = start; i < end; i++) {
            graph_add_edge(large_graph, i, start + (i + 1 - start) % 100);  // 사이클
            if (i + 50 < end) {
                graph_add_edge(large_graph, i, i + 50);  // 추가 연결
            }
        }
        
        // 다음 컴포넌트로의 연결
        if (comp < 9) {
            graph_add_edge(large_graph, end - 1, end);  // 마지막 -> 다음 첫 번째
        }
    }
    
    BENCHMARK_START("Large graph SCC computation (1000 vertices)");
    scc_result_t* result = scc_find(large_graph);
    BENCHMARK_END();
    
    ASSERT_NOT_NULL(result, "대용량 그래프 SCC 찾기가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(result), 10, "10개의 SCC가 있어야 함");
    
    // 각 컴포넌트 크기 확인
    for (int i = 0; i < 10; i++) {
        ASSERT_EQUAL(scc_get_component_size(result, i), 100, 
                     "각 SCC의 크기가 100이어야 함");
    }
    
    // 파일 I/O 성능 테스트
    char filename[] = "large_graph_test.txt";
    
    BENCHMARK_START("Large graph save to file");
    int save_result = graph_save_to_file(large_graph, filename, GRAPH_FORMAT_EDGE_LIST);
    BENCHMARK_END();
    
    ASSERT_EQUAL(save_result, SCC_SUCCESS, "대용량 그래프 저장이 성공해야 함");
    
    BENCHMARK_START("Large graph load from file");
    graph_t* loaded = NULL;
    int load_result = graph_load_from_file(&loaded, filename, GRAPH_FORMAT_EDGE_LIST);
    BENCHMARK_END();
    
    ASSERT_EQUAL(load_result, SCC_SUCCESS, "대용량 그래프 로드가 성공해야 함");
    ASSERT_NOT_NULL(loaded, "로드된 그래프가 NULL이 아니어야 함");
    ASSERT_EQUAL(graph_get_vertex_count(loaded), size, 
                 "로드된 그래프의 정점 수가 같아야 함");
    
    // 정리
    remove(filename);
    graph_destroy(loaded);
    scc_result_destroy(result);
    graph_destroy(large_graph);
    
    TEST_END();
}

// 경계 조건 통합 테스트
static void test_edge_cases_integration() {
    TEST_START("Edge cases integration");
    
    // 1. 빈 그래프들
    graph_t* empty_graph = graph_create(10);
    scc_result_t* empty_result = scc_find(empty_graph);
    
    if (empty_result) {
        ASSERT_EQUAL(scc_get_component_count(empty_result), 0, 
                     "빈 그래프는 0개 컴포넌트를 가져야 함");
        scc_result_destroy(empty_result);
    }
    graph_destroy(empty_graph);
    
    // 2. 단일 정점 그래프
    graph_t* single_vertex = graph_create(1);
    graph_add_vertex(single_vertex);
    
    scc_result_t* single_result = scc_find(single_vertex);
    ASSERT_NOT_NULL(single_result, "단일 정점 그래프 처리가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(single_result), 1, 
                 "단일 정점은 1개 컴포넌트를 가져야 함");
    
    scc_result_destroy(single_result);
    graph_destroy(single_vertex);
    
    // 3. 자기 루프만 있는 그래프
    graph_t* self_loop_graph = graph_create(3);
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(self_loop_graph);
        graph_add_edge(self_loop_graph, i, i);
    }
    
    scc_result_t* self_loop_result = scc_find(self_loop_graph);
    ASSERT_NOT_NULL(self_loop_result, "자기 루프 그래프 처리가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(self_loop_result), 3, 
                 "자기 루프만 있는 그래프는 각각 별도 컴포넌트여야 함");
    
    scc_result_destroy(self_loop_result);
    graph_destroy(self_loop_graph);
    
    // 4. 완전 연결 그래프
    graph_t* complete_graph = graph_create(4);
    for (int i = 0; i < 4; i++) {
        graph_add_vertex(complete_graph);
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i != j) {
                graph_add_edge(complete_graph, i, j);
            }
        }
    }
    
    scc_result_t* complete_result = scc_find(complete_graph);
    ASSERT_NOT_NULL(complete_result, "완전 그래프 처리가 성공해야 함");
    ASSERT_EQUAL(scc_get_component_count(complete_result), 1, 
                 "완전 그래프는 1개의 큰 컴포넌트여야 함");
    ASSERT_EQUAL(scc_get_component_size(complete_result, 0), 4, 
                 "완전 그래프 컴포넌트 크기가 4여야 함");
    
    scc_result_destroy(complete_result);
    graph_destroy(complete_graph);
    
    TEST_END();
}

// 오류 복구 및 견고성 테스트
static void test_error_recovery_robustness() {
    TEST_START("Error recovery and robustness");
    
    // 초기 오류 상태 설정
    scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
    ASSERT_EQUAL(scc_get_last_error(), SCC_ERROR_MEMORY_ALLOCATION, 
                 "오류가 설정되어야 함");
    
    // 정상 작업 수행 - 오류가 클리어되어야 함
    graph_t* graph = graph_create(3);
    ASSERT_NOT_NULL(graph, "정상 작업이 성공해야 함");
    
    for (int i = 0; i < 3; i++) {
        graph_add_vertex(graph);
    }
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    scc_result_t* result = scc_find(graph);
    ASSERT_NOT_NULL(result, "SCC 찾기가 성공해야 함");
    
    // 여러 잘못된 인수로 함수 호출
    ASSERT_NULL(scc_result_copy(NULL), "NULL 결과 복사는 실패해야 함");
    ASSERT_EQUAL(scc_get_last_error(), SCC_ERROR_NULL_POINTER, 
                 "적절한 오류 코드가 설정되어야 함");
    
    scc_clear_error();
    
    ASSERT_EQUAL(graph_add_edge(graph, 10, 20), SCC_ERROR_INVALID_VERTEX, 
                 "잘못된 정점 번호는 오류를 반환해야 함");
    ASSERT_EQUAL(scc_get_last_error(), SCC_ERROR_INVALID_VERTEX, 
                 "오류 상태가 업데이트되어야 함");
    
    // 정상 정리
    scc_result_destroy(result);
    graph_destroy(graph);
    
    // 최종 정리 후 오류 상태 확인
    scc_clear_error();
    ASSERT_EQUAL(scc_get_last_error(), SCC_SUCCESS, 
                 "정리 후 오류 상태가 클리어되어야 함");
    
    TEST_END();
}

// 모든 통합 테스트 실행
void run_integration_tests() {
    printf("=== 통합 테스트 ===\n");
    
    test_complete_workflow();
    test_algorithm_consistency();
    test_memory_management_integration();
    test_large_scale_processing();
    test_edge_cases_integration();
    test_error_recovery_robustness();
    
    printf("통합 테스트 완료\n\n");
}