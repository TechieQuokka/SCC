#include "test_framework.h"
#include <time.h>

// 글로벌 테스트 통계
test_stats_t g_test_stats = {0};

void test_init() {
    memset(&g_test_stats, 0, sizeof(test_stats_t));
    printf(COLOR_BLUE "=== SCC 라이브러리 테스트 스위트 ===" COLOR_RESET "\n\n");
}

void test_print_summary() {
    printf("\n" COLOR_BLUE "=== 테스트 결과 요약 ===" COLOR_RESET "\n");
    printf("총 테스트: %d개\n", g_test_stats.tests_run);
    printf("성공: " COLOR_GREEN "%d개" COLOR_RESET "\n", g_test_stats.tests_passed);
    
    if (g_test_stats.tests_failed > 0) {
        printf("실패: " COLOR_RED "%d개" COLOR_RESET "\n", g_test_stats.tests_failed);
    } else {
        printf("실패: 0개\n");
    }
    
    printf("\n총 어설션: %d개\n", g_test_stats.assertions_run);
    printf("성공: " COLOR_GREEN "%d개" COLOR_RESET "\n", g_test_stats.assertions_passed);
    
    if (g_test_stats.assertions_failed > 0) {
        printf("실패: " COLOR_RED "%d개" COLOR_RESET "\n", g_test_stats.assertions_failed);
    } else {
        printf("실패: 0개\n");
    }
    
    // 성공률 계산
    double test_success_rate = (g_test_stats.tests_run > 0) ? 
        (double)g_test_stats.tests_passed / g_test_stats.tests_run * 100.0 : 0.0;
    double assertion_success_rate = (g_test_stats.assertions_run > 0) ? 
        (double)g_test_stats.assertions_passed / g_test_stats.assertions_run * 100.0 : 0.0;
    
    printf("\n테스트 성공률: %.1f%%\n", test_success_rate);
    printf("어설션 성공률: %.1f%%\n", assertion_success_rate);
    
    if (test_all_passed()) {
        printf("\n" COLOR_GREEN "🎉 모든 테스트가 통과했습니다!" COLOR_RESET "\n");
    } else {
        printf("\n" COLOR_RED "❌ 일부 테스트가 실패했습니다." COLOR_RESET "\n");
    }
}

bool test_all_passed() {
    return g_test_stats.tests_failed == 0 && g_test_stats.assertions_failed == 0;
}