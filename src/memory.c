#include "graph.h"
#include "scc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// 전역 오류 상태
static thread_local scc_error_t last_error = SCC_SUCCESS;

// 오류 처리 함수들
void scc_set_error(scc_error_t error) {
    last_error = error;
}

int scc_get_last_error(void) {
    return last_error;
}

void scc_clear_error(void) {
    last_error = SCC_SUCCESS;
}

const char* scc_error_string(scc_error_t error) {
    static const char* error_messages[] = {
        "성공",
        "널 포인터 인수",
        "유효하지 않은 정점 ID",
        "메모리 할당 실패",
        "그래프가 비어있음",
        "유효하지 않은 매개변수",
        "정점이 이미 존재함",
        "간선이 이미 존재함"
    };
    
    if (error >= 0 || error < -7) return "알 수 없는 오류";
    return error_messages[-error];
}

// 메모리 풀 구현
memory_pool_t* memory_pool_create(size_t block_size, size_t alignment) {
    if (block_size == 0 || alignment == 0) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    // alignment를 2의 거듭제곱으로 조정
    size_t align = 1;
    while (align < alignment) align <<= 1;
    
    memory_pool_t* pool = malloc(sizeof(memory_pool_t));
    if (!pool) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    pool->blocks = NULL;
    pool->block_size = block_size;
    pool->total_allocated = 0;
    pool->total_used = 0;
    pool->alignment = align;
    
    return pool;
}

void memory_pool_destroy(memory_pool_t* pool) {
    if (!pool) return;
    
    memory_block_t* block = pool->blocks;
    while (block) {
        memory_block_t* next = block->next;
        free(block->data);
        free(block);
        block = next;
    }
    
    free(pool);
}

void* memory_pool_alloc(memory_pool_t* pool, size_t size) {
    if (!pool || size == 0) {
        scc_set_error(SCC_ERROR_INVALID_PARAMETER);
        return NULL;
    }
    
    // 정렬된 크기 계산
    size_t aligned_size = (size + pool->alignment - 1) & ~(pool->alignment - 1);
    
    // 사용 가능한 블록 찾기
    memory_block_t* block = pool->blocks;
    while (block) {
        if (block->is_free && block->size >= aligned_size) {
            block->is_free = false;
            pool->total_used += block->size;
            return block->data;
        }
        block = block->next;
    }
    
    // 새 블록 할당
    size_t alloc_size = (aligned_size > pool->block_size) ? aligned_size : pool->block_size;
    
    memory_block_t* new_block = malloc(sizeof(memory_block_t));
    if (!new_block) {
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    new_block->data = aligned_alloc(pool->alignment, alloc_size);
    if (!new_block->data) {
        free(new_block);
        scc_set_error(SCC_ERROR_MEMORY_ALLOCATION);
        return NULL;
    }
    
    new_block->size = alloc_size;
    new_block->is_free = false;
    new_block->next = pool->blocks;
    pool->blocks = new_block;
    
    pool->total_allocated += alloc_size;
    pool->total_used += alloc_size;
    
    return new_block->data;
}

void memory_pool_free(memory_pool_t* pool, void* ptr) {
    if (!pool || !ptr) return;
    
    memory_block_t* block = pool->blocks;
    while (block) {
        if (block->data == ptr) {
            if (!block->is_free) {
                block->is_free = true;
                pool->total_used -= block->size;
            }
            return;
        }
        block = block->next;
    }
}

void memory_pool_reset(memory_pool_t* pool) {
    if (!pool) return;
    
    memory_block_t* block = pool->blocks;
    while (block) {
        block->is_free = true;
        block = block->next;
    }
    
    pool->total_used = 0;
}