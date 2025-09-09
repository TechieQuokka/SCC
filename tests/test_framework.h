#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// 테스트 통계
typedef struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    int assertions_run;
    int assertions_passed;
    int assertions_failed;
} test_stats_t;

extern test_stats_t g_test_stats;

// 색상 정의 (지원되는 터미널에서)
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// 테스트 매크로들
#define TEST_START(name) \
    do { \
        printf(COLOR_CYAN "  [테스트] %s" COLOR_RESET "\n", (name)); \
        g_test_stats.tests_run++; \
    } while(0)

#define TEST_END() \
    do { \
        g_test_stats.tests_passed++; \
        printf(COLOR_GREEN "    ✓ 통과" COLOR_RESET "\n"); \
    } while(0)

#define TEST_FAIL(message) \
    do { \
        g_test_stats.tests_failed++; \
        printf(COLOR_RED "    ✗ 실패: %s" COLOR_RESET "\n", (message)); \
        return; \
    } while(0)

// 어설션 매크로들
#define ASSERT_TRUE(condition, message) \
    do { \
        g_test_stats.assertions_run++; \
        if (condition) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_TRUE 실패: %s (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (message), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_TRUE failed"); \
        } \
    } while(0)

#define ASSERT_FALSE(condition, message) \
    do { \
        g_test_stats.assertions_run++; \
        if (!(condition)) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_FALSE 실패: %s (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (message), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_FALSE failed"); \
        } \
    } while(0)

#define ASSERT_EQUAL(actual, expected, message) \
    do { \
        g_test_stats.assertions_run++; \
        if ((actual) == (expected)) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_EQUAL 실패: %s" COLOR_RESET "\n", (message)); \
            printf(COLOR_RED "      기대값: %d, 실제값: %d (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (int)(expected), (int)(actual), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_EQUAL failed"); \
        } \
    } while(0)

#define ASSERT_NOT_EQUAL(actual, expected, message) \
    do { \
        g_test_stats.assertions_run++; \
        if ((actual) != (expected)) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_NOT_EQUAL 실패: %s (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (message), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_NOT_EQUAL failed"); \
        } \
    } while(0)

#define ASSERT_NULL(ptr, message) \
    do { \
        g_test_stats.assertions_run++; \
        if ((ptr) == NULL) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_NULL 실패: %s (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (message), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_NULL failed"); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr, message) \
    do { \
        g_test_stats.assertions_run++; \
        if ((ptr) != NULL) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_NOT_NULL 실패: %s (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (message), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_NOT_NULL failed"); \
        } \
    } while(0)

#define ASSERT_STRING_EQUAL(actual, expected, message) \
    do { \
        g_test_stats.assertions_run++; \
        if (strcmp((actual), (expected)) == 0) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_STRING_EQUAL 실패: %s" COLOR_RESET "\n", (message)); \
            printf(COLOR_RED "      기대값: \"%s\", 실제값: \"%s\" (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (expected), (actual), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_STRING_EQUAL failed"); \
        } \
    } while(0)

#define ASSERT_DOUBLE_EQUAL(actual, expected, tolerance, message) \
    do { \
        g_test_stats.assertions_run++; \
        double _diff = (actual) - (expected); \
        if (_diff < 0) _diff = -_diff; \
        if (_diff <= (tolerance)) { \
            g_test_stats.assertions_passed++; \
        } else { \
            g_test_stats.assertions_failed++; \
            printf(COLOR_RED "    ✗ ASSERT_DOUBLE_EQUAL 실패: %s" COLOR_RESET "\n", (message)); \
            printf(COLOR_RED "      기대값: %.6f, 실제값: %.6f, 허용오차: %.6f (파일: %s, 줄: %d)" COLOR_RESET "\n", \
                   (expected), (actual), (tolerance), __FILE__, __LINE__); \
            TEST_FAIL("ASSERT_DOUBLE_EQUAL failed"); \
        } \
    } while(0)

// 성능 측정 매크로
#define BENCHMARK_START(name) \
    do { \
        printf(COLOR_YELLOW "  [벤치마크] %s 시작..." COLOR_RESET "\n", (name)); \
        clock_t _start_time = clock(); \
        
#define BENCHMARK_END() \
        clock_t _end_time = clock(); \
        double _elapsed = ((double)(_end_time - _start_time)) / CLOCKS_PER_SEC * 1000.0; \
        printf(COLOR_YELLOW "    실행 시간: %.3f ms" COLOR_RESET "\n", _elapsed); \
    } while(0)

// 유틸리티 함수들
void test_init();
void test_print_summary();
bool test_all_passed();

// 테스트 스위트 선언
void run_graph_tests();
void run_scc_tests();
void run_tarjan_tests();
void run_kosaraju_tests();
void run_memory_tests();
void run_utils_tests();
void run_io_tests();
void run_integration_tests();
void run_performance_tests();

#endif // TEST_FRAMEWORK_H