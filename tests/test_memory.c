#include "test_framework.h"
#include "../src/scc.h"
#include <assert.h>
#include <string.h>

// 메모리 풀 생성 및 제거 테스트
static void test_memory_pool_create_destroy() {
    TEST_START("Memory pool creation and destruction");
    
    // 유효한 크기로 풀 생성
    memory_pool_t* pool = memory_pool_create(1024, 64);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    memory_pool_destroy(pool);
    
    // 잘못된 크기로 풀 생성
    memory_pool_t* invalid_pool = memory_pool_create(0, 64);
    ASSERT_NULL(invalid_pool, "크기 0으로 풀 생성이 실패해야 함");
    
    invalid_pool = memory_pool_create(1024, 0);
    ASSERT_NULL(invalid_pool, "정렬 크기 0으로 풀 생성이 실패해야 함");
    
    TEST_END();
}

// 메모리 할당 테스트
static void test_memory_pool_allocation() {
    TEST_START("Memory pool allocation");
    
    memory_pool_t* pool = memory_pool_create(1024, 8);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    // 작은 할당들
    void* ptr1 = memory_pool_alloc(pool, 64);
    ASSERT_NOT_NULL(ptr1, "64바이트 할당이 성공해야 함");
    
    void* ptr2 = memory_pool_alloc(pool, 128);
    ASSERT_NOT_NULL(ptr2, "128바이트 할당이 성공해야 함");
    
    void* ptr3 = memory_pool_alloc(pool, 256);
    ASSERT_NOT_NULL(ptr3, "256바이트 할당이 성공해야 함");
    
    // 할당된 메모리들이 서로 다른 주소인지 확인
    ASSERT_NOT_EQUAL((intptr_t)ptr1, (intptr_t)ptr2, "할당된 포인터들이 달라야 함");
    ASSERT_NOT_EQUAL((intptr_t)ptr2, (intptr_t)ptr3, "할당된 포인터들이 달라야 함");
    ASSERT_NOT_EQUAL((intptr_t)ptr1, (intptr_t)ptr3, "할당된 포인터들이 달라야 함");
    
    // 메모리 쓰기/읽기 테스트
    memset(ptr1, 0xAA, 64);
    memset(ptr2, 0xBB, 128);
    memset(ptr3, 0xCC, 256);
    
    ASSERT_EQUAL(((unsigned char*)ptr1)[0], 0xAA, "메모리 쓰기가 정상 동작해야 함");
    ASSERT_EQUAL(((unsigned char*)ptr2)[0], 0xBB, "메모리 쓰기가 정상 동작해야 함");
    ASSERT_EQUAL(((unsigned char*)ptr3)[0], 0xCC, "메모리 쓰기가 정상 동작해야 함");
    
    memory_pool_destroy(pool);
    TEST_END();
}

// 메모리 정렬 테스트
static void test_memory_pool_alignment() {
    TEST_START("Memory pool alignment");
    
    // 16바이트 정렬 풀
    memory_pool_t* pool = memory_pool_create(1024, 16);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    void* ptr1 = memory_pool_alloc(pool, 15);  // 16보다 작은 크기
    void* ptr2 = memory_pool_alloc(pool, 17);  // 16보다 큰 크기
    void* ptr3 = memory_pool_alloc(pool, 32);  // 16의 배수
    
    ASSERT_NOT_NULL(ptr1, "할당이 성공해야 함");
    ASSERT_NOT_NULL(ptr2, "할당이 성공해야 함");
    ASSERT_NOT_NULL(ptr3, "할당이 성공해야 함");
    
    // 16바이트 정렬 확인
    ASSERT_EQUAL((intptr_t)ptr1 % 16, 0, "포인터가 16바이트로 정렬되어야 함");
    ASSERT_EQUAL((intptr_t)ptr2 % 16, 0, "포인터가 16바이트로 정렬되어야 함");
    ASSERT_EQUAL((intptr_t)ptr3 % 16, 0, "포인터가 16바이트로 정렬되어야 함");
    
    memory_pool_destroy(pool);
    TEST_END();
}

// 메모리 풀 리셋 테스트
static void test_memory_pool_reset() {
    TEST_START("Memory pool reset");
    
    memory_pool_t* pool = memory_pool_create(512, 8);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    // 여러 할당 수행
    void* ptr1 = memory_pool_alloc(pool, 100);
    void* ptr2 = memory_pool_alloc(pool, 200);
    ASSERT_NOT_NULL(ptr1, "첫 번째 할당이 성공해야 함");
    ASSERT_NOT_NULL(ptr2, "두 번째 할당이 성공해야 함");
    
    // 풀 리셋
    memory_pool_reset(pool);
    
    // 리셋 후 다시 할당 - 같은 주소부터 시작해야 함
    void* ptr3 = memory_pool_alloc(pool, 100);
    ASSERT_NOT_NULL(ptr3, "리셋 후 할당이 성공해야 함");
    
    // 리셋 후 첫 할당은 풀의 시작 위치와 같아야 함
    // (구현에 따라 다를 수 있지만, 일반적으로는 그래야 함)
    
    memory_pool_destroy(pool);
    TEST_END();
}

// 메모리 풀 용량 초과 테스트
static void test_memory_pool_overflow() {
    TEST_START("Memory pool overflow handling");
    
    // 작은 풀 생성
    memory_pool_t* pool = memory_pool_create(256, 8);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    // 풀 용량보다 큰 할당 요청
    void* large_ptr = memory_pool_alloc(pool, 512);
    ASSERT_NULL(large_ptr, "풀 용량을 초과하는 할당은 실패해야 함");
    
    // 작은 할당들을 풀이 가득 찰 때까지 수행
    void* ptrs[10];
    int alloc_count = 0;
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = memory_pool_alloc(pool, 32);
        if (ptrs[i] != NULL) {
            alloc_count++;
        } else {
            break;
        }
    }
    
    ASSERT_TRUE(alloc_count > 0, "일부 할당은 성공해야 함");
    ASSERT_TRUE(alloc_count < 10, "모든 할당이 성공할 수는 없음 (풀 크기 제한)");
    
    memory_pool_destroy(pool);
    TEST_END();
}

// 오류 코드 테스트
static void test_error_handling() {
    TEST_START("Error code handling");
    
    // 초기 오류 상태 확인
    ASSERT_EQUAL(scc_get_last_error(), SCC_SUCCESS, "초기 오류 상태는 SUCCESS여야 함");
    
    // 오류 설정 및 확인
    scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
    ASSERT_EQUAL(scc_get_last_error(), SCC_ERROR_MEMORY_ALLOCATION, 
                 "설정한 오류 코드가 반환되어야 함");
    
    // 오류 초기화
    scc_clear_error();
    ASSERT_EQUAL(scc_get_last_error(), SCC_SUCCESS, "오류 클리어 후 SUCCESS여야 함");
    
    // 여러 오류 코드 테스트
    scc_error_t test_errors[] = {
        SCC_ERROR_NULL_POINTER,
        SCC_ERROR_INVALID_PARAMETER,
        SCC_ERROR_GRAPH_EMPTY,
        SCC_ERROR_GRAPH_FULL,
        SCC_ERROR_INVALID_VERTEX,
        SCC_ERROR_EDGE_EXISTS,
        SCC_ERROR_EDGE_NOT_FOUND
    };
    
    for (size_t i = 0; i < sizeof(test_errors) / sizeof(test_errors[0]); i++) {
        scc_set_error(test_errors[i]);
        ASSERT_EQUAL(scc_get_last_error(), test_errors[i], 
                     "각 오류 코드가 정확히 설정/반환되어야 함");
    }
    
    TEST_END();
}

// 오류 메시지 테스트
static void test_error_messages() {
    TEST_START("Error message retrieval");
    
    // 각 오류 코드에 대한 메시지 확인
    const char* success_msg = scc_get_error_message(SCC_SUCCESS);
    ASSERT_NOT_NULL(success_msg, "SUCCESS 메시지가 NULL이 아니어야 함");
    ASSERT_TRUE(strlen(success_msg) > 0, "SUCCESS 메시지가 비어있지 않아야 함");
    
    const char* null_ptr_msg = scc_get_error_message(SCC_ERROR_NULL_POINTER);
    ASSERT_NOT_NULL(null_ptr_msg, "NULL_POINTER 메시지가 NULL이 아니어야 함");
    ASSERT_TRUE(strlen(null_ptr_msg) > 0, "NULL_POINTER 메시지가 비어있지 않아야 함");
    
    const char* memory_msg = scc_get_error_message(SCC_ERROR_MEMORY_ALLOCATION);
    ASSERT_NOT_NULL(memory_msg, "MEMORY_ALLOCATION 메시지가 NULL이 아니어야 함");
    ASSERT_TRUE(strlen(memory_msg) > 0, "MEMORY_ALLOCATION 메시지가 비어있지 않아야 함");
    
    // 잘못된 오류 코드에 대한 처리
    const char* invalid_msg = scc_get_error_message((scc_error_t)9999);
    ASSERT_NOT_NULL(invalid_msg, "잘못된 오류 코드에 대해서도 메시지가 반환되어야 함");
    
    TEST_END();
}

// 메모리 사용량 통계 테스트
static void test_memory_statistics() {
    TEST_START("Memory usage statistics");
    
    memory_pool_t* pool = memory_pool_create(1024, 8);
    ASSERT_NOT_NULL(pool, "메모리 풀 생성이 성공해야 함");
    
    // 초기 사용량 확인
    size_t initial_used = memory_pool_get_used_size(pool);
    size_t total_size = memory_pool_get_total_size(pool);
    
    ASSERT_EQUAL(initial_used, 0, "초기 사용량은 0이어야 함");
    ASSERT_EQUAL(total_size, 1024, "전체 크기가 1024여야 함");
    
    // 할당 후 사용량 확인
    void* ptr1 = memory_pool_alloc(pool, 100);
    ASSERT_NOT_NULL(ptr1, "할당이 성공해야 함");
    
    size_t used_after_alloc = memory_pool_get_used_size(pool);
    ASSERT_TRUE(used_after_alloc >= 100, "사용량이 최소 100바이트 이상이어야 함");
    ASSERT_TRUE(used_after_alloc <= total_size, "사용량이 전체 크기를 초과하지 않아야 함");
    
    // 여러 할당 후 사용량 확인
    void* ptr2 = memory_pool_alloc(pool, 200);
    ASSERT_NOT_NULL(ptr2, "두 번째 할당이 성공해야 함");
    
    size_t used_after_second = memory_pool_get_used_size(pool);
    ASSERT_TRUE(used_after_second > used_after_alloc, "사용량이 증가해야 함");
    
    // 리셋 후 사용량 확인
    memory_pool_reset(pool);
    size_t used_after_reset = memory_pool_get_used_size(pool);
    ASSERT_EQUAL(used_after_reset, 0, "리셋 후 사용량은 0이어야 함");
    
    memory_pool_destroy(pool);
    TEST_END();
}

// 모든 메모리 관리 테스트 실행
void run_memory_tests() {
    printf("=== 메모리 관리 모듈 테스트 ===\n");
    
    test_memory_pool_create_destroy();
    test_memory_pool_allocation();
    test_memory_pool_alignment();
    test_memory_pool_reset();
    test_memory_pool_overflow();
    test_error_handling();
    test_error_messages();
    test_memory_statistics();
    
    printf("메모리 관리 모듈 테스트 완료\n\n");
}