#include "test_framework.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // 테스트 초기화
    test_init();
    
    // 개별 모듈 테스트 여부를 결정
    bool run_all = true;
    bool run_specific = false;
    
    // 명령행 인수 처리
    if (argc > 1) {
        run_all = false;
        for (int i = 1; i < argc; i++) {
            const char* arg = argv[i];
            
            if (strcmp(arg, "graph") == 0) {
                run_graph_tests();
                run_specific = true;
            } else if (strcmp(arg, "scc") == 0) {
                run_scc_tests();
                run_specific = true;
            } else if (strcmp(arg, "tarjan") == 0) {
                run_tarjan_tests();
                run_specific = true;
            } else if (strcmp(arg, "kosaraju") == 0) {
                run_kosaraju_tests();
                run_specific = true;
            } else if (strcmp(arg, "memory") == 0) {
                run_memory_tests();
                run_specific = true;
            } else if (strcmp(arg, "utils") == 0) {
                run_utils_tests();
                run_specific = true;
            } else if (strcmp(arg, "io") == 0) {
                run_io_tests();
                run_specific = true;
            } else if (strcmp(arg, "integration") == 0) {
                run_integration_tests();
                run_specific = true;
            } else if (strcmp(arg, "performance") == 0) {
                run_performance_tests();
                run_specific = true;
            } else if (strcmp(arg, "all") == 0) {
                run_all = true;
                break;
            } else if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
                printf("사용법: %s [모듈명...]\n", argv[0]);
                printf("모듈명:\n");
                printf("  graph    - 그래프 자료구조 테스트\n");
                printf("  scc      - SCC 메인 API 테스트\n");
                printf("  tarjan   - Tarjan 알고리즘 테스트\n");
                printf("  kosaraju - Kosaraju 알고리즘 테스트\n");
                printf("  memory   - 메모리 관리 테스트\n");
                printf("  utils       - 유틸리티 함수 테스트\n");
                printf("  io          - 파일 I/O 테스트\n");
                printf("  integration - 통합 테스트\n");
                printf("  performance - 성능 벤치마크 테스트\n");
                printf("  all         - 모든 테스트 실행 (기본값)\n");
                printf("  --help   - 이 도움말 표시\n");
                return 0;
            } else {
                printf("알 수 없는 모듈: %s\n", arg);
                printf("--help 옵션으로 사용법을 확인하세요.\n");
                return 1;
            }
        }
        
        if (!run_specific) {
            printf("유효한 모듈을 지정하지 않았습니다.\n");
            return 1;
        }
    }
    
    // 모든 테스트 실행
    if (run_all) {
        printf("전체 테스트 스위트를 실행합니다...\n\n");
        
        run_graph_tests();
        run_scc_tests();
        run_tarjan_tests();
        run_kosaraju_tests();
        run_memory_tests();
        run_utils_tests();
        run_io_tests();
        run_integration_tests();
        run_performance_tests();
    }
    
    // 결과 요약 출력
    test_print_summary();
    
    // 테스트 결과에 따른 종료 코드 반환
    return test_all_passed() ? 0 : 1;
}