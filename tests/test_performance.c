#include "test_framework.h"
#include "../src/scc.h"
#include "../src/graph.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// 성능 측정용 구조체
typedef struct {
    const char* name;
    double time_ms;
    size_t memory_bytes;
    int result_components;
} benchmark_result_t;

// 그래프 생성 헬퍼 함수들
static graph_t* create_cycle_graph(int size) {
    graph_t* graph = graph_create(size);
    for (int i = 0; i < size; i++) {
        graph_add_vertex(graph);
    }
    for (int i = 0; i < size; i++) {
        graph_add_edge(graph, i, (i + 1) % size);
    }
    return graph;
}

static graph_t* create_path_graph(int size) {
    graph_t* graph = graph_create(size);
    for (int i = 0; i < size; i++) {
        graph_add_vertex(graph);
    }
    for (int i = 0; i < size - 1; i++) {
        graph_add_edge(graph, i, i + 1);
    }
    return graph;
}

static graph_t* create_complete_graph(int size) {
    graph_t* graph = graph_create(size);
    for (int i = 0; i < size; i++) {
        graph_add_vertex(graph);
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i != j) {
                graph_add_edge(graph, i, j);
            }
        }
    }
    return graph;
}

static graph_t* create_random_graph(int size, double edge_probability) {
    graph_t* graph = graph_create(size);
    for (int i = 0; i < size; i++) {
        graph_add_vertex(graph);
    }
    
    srand((unsigned int)time(NULL));
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i != j && (double)rand() / RAND_MAX < edge_probability) {
                graph_add_edge(graph, i, j);
            }
        }
    }
    return graph;
}

static graph_t* create_multi_component_graph(int components, int component_size) {
    int total_size = components * component_size;
    graph_t* graph = graph_create(total_size);
    
    for (int i = 0; i < total_size; i++) {
        graph_add_vertex(graph);
    }
    
    // 각 컴포넌트 내부를 사이클로 연결
    for (int c = 0; c < components; c++) {
        int start = c * component_size;
        for (int i = 0; i < component_size; i++) {
            int from = start + i;
            int to = start + (i + 1) % component_size;
            graph_add_edge(graph, from, to);
        }
        
        // 컴포넌트 간 연결 (마지막 컴포넌트 제외)
        if (c < components - 1) {
            int from = start + component_size - 1;
            int to = start + component_size;
            graph_add_edge(graph, from, to);
        }
    }
    
    return graph;
}

// 단일 알고리즘 성능 측정
static benchmark_result_t benchmark_algorithm(
    const char* name,
    scc_result_t* (*algorithm)(const graph_t*),
    const graph_t* graph
) {
    benchmark_result_t result = {0};
    result.name = name;
    
    clock_t start = clock();
    scc_result_t* scc_result = algorithm(graph);
    clock_t end = clock();
    
    result.time_ms = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
    
    if (scc_result) {
        result.result_components = scc_get_component_count(scc_result);
        
        // 메모리 사용량 추정 (실제 측정은 복잡하므로 근사치)
        int vertices = graph_get_vertex_count(graph);
        result.memory_bytes = sizeof(scc_result_t) + 
                             result.result_components * sizeof(scc_component_t) +
                             vertices * sizeof(int); // vertex_to_component
        
        scc_result_destroy(scc_result);
    }
    
    return result;
}

// 확장성 테스트 (다양한 크기의 그래프)
static void test_scalability_cycle_graphs() {
    TEST_START("Scalability test on cycle graphs");
    
    int sizes[] = {100, 500, 1000, 2000, 5000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    printf("    크기별 성능 (사이클 그래프):\n");
    printf("    크기     | Tarjan (ms) | Kosaraju (ms) | 비율\n");
    printf("    ---------|-------------|---------------|------\n");
    
    for (int i = 0; i < num_sizes; i++) {
        graph_t* graph = create_cycle_graph(sizes[i]);
        
        benchmark_result_t tarjan = benchmark_algorithm("Tarjan", scc_find_tarjan, graph);
        benchmark_result_t kosaraju = benchmark_algorithm("Kosaraju", scc_find_kosaraju, graph);
        
        double ratio = (kosaraju.time_ms > 0) ? tarjan.time_ms / kosaraju.time_ms : 0.0;
        
        printf("    %8d | %11.3f | %13.3f | %4.2f\n", 
               sizes[i], tarjan.time_ms, kosaraju.time_ms, ratio);
        
        // 결과 검증
        ASSERT_EQUAL(tarjan.result_components, 1, "사이클 그래프는 1개 컴포넌트를 가져야 함");
        ASSERT_EQUAL(kosaraju.result_components, 1, "사이클 그래프는 1개 컴포넌트를 가져야 함");
        
        graph_destroy(graph);
    }
    
    TEST_END();
}

// 밀도별 성능 테스트
static void test_performance_by_density() {
    TEST_START("Performance by graph density");
    
    int size = 1000;
    double densities[] = {0.001, 0.01, 0.05, 0.1, 0.2};
    int num_densities = sizeof(densities) / sizeof(densities[0]);
    
    printf("    밀도별 성능 (1000개 정점):\n");
    printf("    밀도   | 간선 수 | Tarjan (ms) | Kosaraju (ms)\n");
    printf("    -------|---------|-------------|---------------\n");
    
    for (int i = 0; i < num_densities; i++) {
        graph_t* graph = create_random_graph(size, densities[i]);
        int edge_count = graph_get_edge_count(graph);
        
        benchmark_result_t tarjan = benchmark_algorithm("Tarjan", scc_find_tarjan, graph);
        benchmark_result_t kosaraju = benchmark_algorithm("Kosaraju", scc_find_kosaraju, graph);
        
        printf("    %6.3f | %7d | %11.3f | %13.3f\n", 
               densities[i], edge_count, tarjan.time_ms, kosaraju.time_ms);
        
        // 결과 일치 확인
        ASSERT_EQUAL(tarjan.result_components, kosaraju.result_components, 
                     "두 알고리즘의 결과가 일치해야 함");
        
        graph_destroy(graph);
    }
    
    TEST_END();
}

// 그래프 형태별 성능 비교
static void test_performance_by_graph_type() {
    TEST_START("Performance by graph type");
    
    int size = 2000;
    
    struct {
        const char* name;
        graph_t* (*creator)(int);
        int expected_components;
    } graph_types[] = {
        {"사이클", create_cycle_graph, 1},
        {"경로", create_path_graph, 2000},
        {"완전그래프 (100개)", NULL, 1}, // 별도 처리
        {"다중 컴포넌트 (20x100)", NULL, 20} // 별도 처리
    };
    
    printf("    그래프 형태별 성능:\n");
    printf("    형태            | 컴포넌트 | Tarjan (ms) | Kosaraju (ms)\n");
    printf("    ----------------|----------|-------------|---------------\n");
    
    // 사이클과 경로 그래프
    for (int i = 0; i < 2; i++) {
        graph_t* graph = graph_types[i].creator(size);
        
        benchmark_result_t tarjan = benchmark_algorithm("Tarjan", scc_find_tarjan, graph);
        benchmark_result_t kosaraju = benchmark_algorithm("Kosaraju", scc_find_kosaraju, graph);
        
        printf("    %-15s | %8d | %11.3f | %13.3f\n", 
               graph_types[i].name, tarjan.result_components, 
               tarjan.time_ms, kosaraju.time_ms);
        
        ASSERT_EQUAL(tarjan.result_components, graph_types[i].expected_components,
                     "예상 컴포넌트 수와 일치해야 함");
        
        graph_destroy(graph);
    }
    
    // 완전 그래프 (크기 줄임)
    graph_t* complete = create_complete_graph(100);
    benchmark_result_t tarjan_complete = benchmark_algorithm("Tarjan", scc_find_tarjan, complete);
    benchmark_result_t kosaraju_complete = benchmark_algorithm("Kosaraju", scc_find_kosaraju, complete);
    
    printf("    %-15s | %8d | %11.3f | %13.3f\n", 
           "완전그래프", tarjan_complete.result_components, 
           tarjan_complete.time_ms, kosaraju_complete.time_ms);
    
    graph_destroy(complete);
    
    // 다중 컴포넌트 그래프
    graph_t* multi = create_multi_component_graph(20, 100);
    benchmark_result_t tarjan_multi = benchmark_algorithm("Tarjan", scc_find_tarjan, multi);
    benchmark_result_t kosaraju_multi = benchmark_algorithm("Kosaraju", scc_find_kosaraju, multi);
    
    printf("    %-15s | %8d | %11.3f | %13.3f\n", 
           "다중컴포넌트", tarjan_multi.result_components, 
           tarjan_multi.time_ms, kosaraju_multi.time_ms);
    
    ASSERT_EQUAL(tarjan_multi.result_components, 20, "20개 컴포넌트를 가져야 함");
    
    graph_destroy(multi);
    
    TEST_END();
}

// I/O 성능 테스트
static void test_io_performance() {
    TEST_START("I/O performance");
    
    int sizes[] = {1000, 5000, 10000};
    graph_format_t formats[] = {GRAPH_FORMAT_EDGE_LIST, GRAPH_FORMAT_ADJACENCY_LIST, GRAPH_FORMAT_DOT};
    const char* format_names[] = {"간선리스트", "인접리스트", "DOT"};
    
    printf("    I/O 성능 (사이클 그래프):\n");
    printf("    크기  | 형식       | 저장 (ms) | 로드 (ms) | 파일 크기 (추정)\n");
    printf("    ------|------------|-----------|-----------|------------------\n");
    
    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); s++) {
        graph_t* graph = create_cycle_graph(sizes[s]);
        
        for (size_t f = 0; f < sizeof(formats) / sizeof(formats[0]); f++) {
            if (formats[f] == GRAPH_FORMAT_DOT && sizes[s] > 1000) {
                continue; // DOT은 큰 그래프에서 생략
            }
            
            char filename[256];
            snprintf(filename, sizeof(filename), "perf_test_%d_%d.txt", sizes[s], (int)f);
            
            // 저장 성능 측정
            clock_t start = clock();
            int save_result = graph_save_to_file(graph, filename, formats[f]);
            clock_t end = clock();
            double save_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            
            ASSERT_EQUAL(save_result, SCC_SUCCESS, "그래프 저장이 성공해야 함");
            
            // 파일 크기 확인
            FILE* file = fopen(filename, "r");
            long file_size = 0;
            if (file) {
                fseek(file, 0, SEEK_END);
                file_size = ftell(file);
                fclose(file);
            }
            
            // 로드 성능 측정
            start = clock();
            graph_t* loaded = NULL;
            int load_result = graph_load_from_file(&loaded, filename, formats[f]);
            end = clock();
            double load_time = ((double)(end - start) / CLOCKS_PER_SEC) * 1000.0;
            
            ASSERT_EQUAL(load_result, SCC_SUCCESS, "그래프 로드가 성공해야 함");
            ASSERT_NOT_NULL(loaded, "로드된 그래프가 NULL이 아니어야 함");
            
            printf("    %5d | %-10s | %9.3f | %9.3f | %8ld bytes\n", 
                   sizes[s], format_names[f], save_time, load_time, file_size);
            
            // 정리
            remove(filename);
            graph_destroy(loaded);
        }
        
        graph_destroy(graph);
    }
    
    TEST_END();
}

// 메모리 사용량 프로파일링
static void test_memory_usage_profiling() {
    TEST_START("Memory usage profiling");
    
    // 메모리 풀을 사용한 할당 패턴 분석
    int sizes[] = {100, 500, 1000, 2000};
    
    printf("    메모리 사용량 프로파일:\n");
    printf("    크기  | 풀 크기 | 사용량  | 효율성 (%%) | 피크 사용량\n");
    printf("    ------|---------|---------|-------------|-------------\n");
    
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
        int size = sizes[i];
        
        // 적절한 풀 크기 계산 (경험적)
        size_t pool_size = size * sizeof(vertex_t) + size * 10 * sizeof(edge_t) + 4096;
        memory_pool_t* pool = memory_pool_create(pool_size, 8);
        
        if (pool) {
            // 그래프 생성 및 SCC 계산
            graph_t* graph = create_random_graph(size, 0.01);
            
            size_t used_before = memory_pool_get_used_size(pool);
            scc_result_t* result = scc_find(graph);
            size_t used_after = memory_pool_get_used_size(pool);
            
            size_t actual_used = used_after - used_before;
            double efficiency = (double)actual_used / pool_size * 100.0;
            
            printf("    %5d | %7zu | %7zu | %11.1f | %11zu\n", 
                   size, pool_size, actual_used, efficiency, used_after);
            
            // 정리
            if (result) scc_result_destroy(result);
            graph_destroy(graph);
            memory_pool_destroy(pool);
        }
    }
    
    TEST_END();
}

// 알고리즘 선택 휴리스틱 검증
static void test_algorithm_selection_heuristic() {
    TEST_START("Algorithm selection heuristic validation");
    
    struct {
        const char* name;
        graph_t* (*creator)(int);
        int size;
        scc_algorithm_choice_t expected;
    } test_cases[] = {
        {"작은 사이클", create_cycle_graph, 100, SCC_ALGORITHM_TARJAN},
        {"중간 사이클", create_cycle_graph, 2000, SCC_ALGORITHM_TARJAN},
        {"작은 완전그래프", create_complete_graph, 50, SCC_ALGORITHM_KOSARAJU},
        {"중간 완전그래프", create_complete_graph, 100, SCC_ALGORITHM_KOSARAJU},
    };
    
    printf("    알고리즘 선택 휴리스틱 검증:\n");
    printf("    그래프 형태      | 크기 | 예상   | 선택   | 올바름\n");
    printf("    ----------------|------|--------|--------|---------\n");
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        graph_t* graph = test_cases[i].creator(test_cases[i].size);
        
        scc_algorithm_choice_t recommended = scc_recommend_algorithm(graph);
        bool correct = (recommended == test_cases[i].expected);
        
        printf("    %-15s | %4d | %-6s | %-6s | %s\n", 
               test_cases[i].name, test_cases[i].size,
               scc_algorithm_name(test_cases[i].expected),
               scc_algorithm_name(recommended),
               correct ? "예" : "아니오");
        
        // 성능도 측정해서 선택이 합리적인지 확인
        benchmark_result_t tarjan = benchmark_algorithm("Tarjan", scc_find_tarjan, graph);
        benchmark_result_t kosaraju = benchmark_algorithm("Kosaraju", scc_find_kosaraju, graph);
        
        // 선택된 알고리즘이 더 빠른지 확인 (대략적)
        if (recommended == SCC_ALGORITHM_TARJAN && kosaraju.time_ms > 0) {
            double ratio = tarjan.time_ms / kosaraju.time_ms;
            ASSERT_TRUE(ratio <= 1.5, "Tarjan이 선택된 경우 상대적으로 빨라야 함");
        } else if (recommended == SCC_ALGORITHM_KOSARAJU && tarjan.time_ms > 0) {
            double ratio = kosaraju.time_ms / tarjan.time_ms;
            ASSERT_TRUE(ratio <= 1.5, "Kosaraju가 선택된 경우 상대적으로 빨라야 함");
        }
        
        graph_destroy(graph);
    }
    
    TEST_END();
}

// 종합 벤치마크 실행
static void test_comprehensive_benchmark() {
    TEST_START("Comprehensive benchmark suite");
    
    graph_t* test_graph = create_multi_component_graph(10, 100);
    
    printf("    종합 성능 벤치마크 (1000개 정점, 10개 컴포넌트):\n");
    
    // 내장 벤치마크 함수 사용
    scc_benchmark_result_t* benchmark = scc_benchmark_algorithms(test_graph);
    ASSERT_NOT_NULL(benchmark, "벤치마크가 성공해야 함");
    
    printf("    Tarjan 실행 시간: %.3f ms\n", benchmark->tarjan_time_ms);
    printf("    Kosaraju 실행 시간: %.3f ms\n", benchmark->kosaraju_time_ms);
    printf("    Tarjan 메모리 사용량: %zu bytes\n", benchmark->tarjan_memory_peak_bytes);
    printf("    Kosaraju 메모리 사용량: %zu bytes\n", benchmark->kosaraju_memory_peak_bytes);
    printf("    결과 일치: %s\n", benchmark->results_match ? "예" : "아니오");
    
    ASSERT_TRUE(benchmark->results_match, "두 알고리즘의 결과가 일치해야 함");
    ASSERT_TRUE(benchmark->tarjan_time_ms >= 0, "Tarjan 실행 시간이 유효해야 함");
    ASSERT_TRUE(benchmark->kosaraju_time_ms >= 0, "Kosaraju 실행 시간이 유효해야 함");
    
    scc_benchmark_result_destroy(benchmark);
    graph_destroy(test_graph);
    
    TEST_END();
}

// 모든 성능 테스트 실행
void run_performance_tests() {
    printf("=== 성능 벤치마크 테스트 ===\n");
    
    test_scalability_cycle_graphs();
    test_performance_by_density();
    test_performance_by_graph_type();
    test_io_performance();
    test_memory_usage_profiling();
    test_algorithm_selection_heuristic();
    test_comprehensive_benchmark();
    
    printf("성능 벤치마크 테스트 완료\n\n");
}