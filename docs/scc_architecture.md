# 강한 연결 요소(SCC) 구현 아키텍처

## 초록

이 문서는 C언어로 강한 연결 요소(SCC) 알고리즘을 구현하기 위한 포괄적인 아키텍처 설계를 제시합니다. 이 설계는 Kosaraju와 Tarjan 알고리즘을 포함한 여러 알고리즘 구현을 제공하면서 효율성, 모듈성, 확장성에 중점을 둡니다.

## 1. 서론

### 1.1 문제 정의
방향 그래프에서 강한 연결 요소는 컴포넌트 내의 각 정점에서 다른 모든 정점으로의 경로가 존재하는 최대 정점 집합입니다. 모든 SCC를 찾는 것은 컴파일러 최적화, 소셜 네트워크 분석, 회로 설계 등에 응용되는 그래프 분석의 기본입니다.

### 1.2 설계 목표
- **성능**: 최소한의 메모리 오버헤드로 대형 그래프에 최적화
- **모듈성**: 그래프 표현을 알고리즘 구현으로부터 분리
- **확장성**: 여러 SCC 알고리즘과 그래프 형식 지원
- **사용성**: 대형 시스템 통합을 위한 명확한 API 제공

## 2. 알고리즘 선택 및 비교

### 2.1 Kosaraju 알고리즘
**시간 복잡도**: O(V + E)
**공간 복잡도**: O(V)
**장점**:
- 구현과 이해가 간단
- 좋은 캐시 지역성
- 디버깅이 용이

**단점**:
- 두 번의 DFS 패스가 필요
- 전치 그래프 구성 필요

### 2.2 Tarjan 알고리즘
**시간 복잡도**: O(V + E)
**공간 복잡도**: O(V)
**장점**:
- 단일 DFS 패스
- 그래프 전치 불필요
- 더 효율적인 메모리 사용

**단점**:
- 더 복잡한 구현
- 명시적 스택 관리 사용

### 2.3 권장 접근법
**주요**: 일반적인 사용을 위한 Tarjan 알고리즘
**보조**: 교육 목적 및 디버깅을 위한 Kosaraju 알고리즘
**근거**: Tarjan의 단일 패스 접근법이 실제로 더 메모리 효율적이고 빠릅니다.

## 3. 시스템 아키텍처

### 3.1 고수준 아키텍처

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   User API      │    │  Algorithm      │    │    Graph        │
│   Interface     │◄──►│  Implementations│◄──►│  Representation │
└─────────────────┘    └─────────────────┘    └─────────────────┘
         │                       │                       │
         ▼                       ▼                       ▼
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Result        │    │   Utilities     │    │    Memory       │
│   Processing    │    │   & Helpers     │    │   Management    │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

### 3.2 컴포넌트 분석

#### 3.2.1 핵심 컴포넌트
1. **그래프 데이터 구조** (`graph.h`, `graph.c`)
2. **SCC 알고리즘** (`scc_tarjan.h`, `scc_kosaraju.h`)
3. **API 인터페이스** (`scc.h`, `scc.c`)
4. **메모리 관리** (`memory.h`, `memory.c`)
5. **유틸리티** (`utils.h`, `utils.c`)

#### 3.2.2 선택적 컴포넌트
1. **그래프 I/O** (`graph_io.h`, `graph_io.c`)
2. **시각화** (`visualize.h`, `visualize.c`)
3. **벤치마킹** (`benchmark.h`, `benchmark.c`)

## 4. 데이터 구조 설계

### 4.1 그래프 표현

#### 4.1.1 인접 리스트 구조
```c
typedef struct edge {
    int dest;
    struct edge* next;
} edge_t;

typedef struct vertex {
    int id;
    edge_t* edges;
    int out_degree;
    
    // Tarjan 전용 필드
    int index;
    int lowlink;
    bool on_stack;
    
    // 일반 목적
    bool visited;
    void* data;  // 사용자 데이터
} vertex_t;

typedef struct graph {
    vertex_t** vertices;
    int num_vertices;
    int num_edges;
    int capacity;
    
    // 메모리 관리
    struct memory_pool* vertex_pool;
    struct memory_pool* edge_pool;
} graph_t;
```

#### 4.1.2 SCC 결과 구조
```c
typedef struct scc_component {
    int* vertices;
    int size;
    int capacity;
} scc_component_t;

typedef struct scc_result {
    scc_component_t* components;
    int num_components;
    int* vertex_to_component;  // 매핑 배열
    
    // 통계
    int largest_component_size;
    int smallest_component_size;
    double average_component_size;
} scc_result_t;
```

### 4.2 알고리즘별 구조

#### 4.2.1 Tarjan 알고리즘 상태
```c
typedef struct tarjan_state {
    int* stack;
    int stack_top;
    int current_index;
    
    scc_result_t* result;
    int current_component;
} tarjan_state_t;
```

#### 4.2.2 Kosaraju 알고리즘 상태
```c
typedef struct kosaraju_state {
    int* finish_order;
    int finish_index;
    graph_t* transpose_graph;
    
    scc_result_t* result;
    int current_component;
} kosaraju_state_t;
```

## 5. API 설계

### 5.1 핵심 API 함수

#### 5.1.1 그래프 관리
```c
// 그래프 생성 및 소멸
graph_t* graph_create(int initial_capacity);
void graph_destroy(graph_t* graph);

// 그래프 수정
int graph_add_vertex(graph_t* graph);
int graph_add_edge(graph_t* graph, int src, int dest);
int graph_remove_edge(graph_t* graph, int src, int dest);

// 그래프 쿼리
bool graph_has_edge(const graph_t* graph, int src, int dest);
int graph_get_out_degree(const graph_t* graph, int vertex);
```

#### 5.1.2 SCC 계산
```c
// 주요 SCC 함수
scc_result_t* scc_find_tarjan(const graph_t* graph);
scc_result_t* scc_find_kosaraju(const graph_t* graph);
scc_result_t* scc_find(const graph_t* graph);  // 기본 알고리즘

// 결과 관리
void scc_result_destroy(scc_result_t* result);
```

#### 5.1.3 유틸리티 함수
```c
// 결과 분석
int scc_get_component_count(const scc_result_t* result);
int scc_get_component_size(const scc_result_t* result, int component_id);
int scc_get_vertex_component(const scc_result_t* result, int vertex);

// 그래프 속성
bool scc_is_strongly_connected(const graph_t* graph);
graph_t* scc_build_condensation_graph(const graph_t* graph, const scc_result_t* scc);
```

### 5.2 고급 API

#### 5.2.1 스트리밍/온라인 처리
```c
typedef struct scc_incremental {
    graph_t* graph;
    scc_result_t* current_result;
    bool needs_recomputation;
} scc_incremental_t;

scc_incremental_t* scc_incremental_create(int initial_capacity);
int scc_incremental_add_edge(scc_incremental_t* scc_inc, int src, int dest);
const scc_result_t* scc_incremental_get_result(scc_incremental_t* scc_inc);
```

## 6. 메모리 관리 전략

### 6.1 메모리 풀 설계
```c
typedef struct memory_block {
    void* data;
    size_t size;
    bool is_free;
    struct memory_block* next;
} memory_block_t;

typedef struct memory_pool {
    memory_block_t* blocks;
    size_t block_size;
    size_t total_allocated;
    size_t total_used;
} memory_pool_t;
```

### 6.2 할당 전략
- **작은 할당**: 정점과 간선에 메모리 풀 사용
- **큰 할당**: 결과 구조에 직접 malloc/free 사용
- **스택 할당**: 가능한 경우 임시 알고리즘 상태에 사용

## 7. 오류 처리

### 7.1 오류 코드
```c
typedef enum {
    SCC_SUCCESS = 0,
    SCC_ERROR_NULL_POINTER = -1,
    SCC_ERROR_INVALID_VERTEX = -2,
    SCC_ERROR_MEMORY_ALLOCATION = -3,
    SCC_ERROR_GRAPH_EMPTY = -4,
    SCC_ERROR_INVALID_PARAMETER = -5
} scc_error_t;
```

### 7.2 오류 처리 패턴
```c
scc_result_t* result = scc_find_tarjan(graph);
if (!result) {
    int error = scc_get_last_error();
    fprintf(stderr, "SCC 계산 실패: %s\n", scc_error_string(error));
    return -1;
}
```

## 8. 성능 고려사항

### 8.1 시간 복잡도 분석
- **그래프 생성**: 정점 할당에 O(V), 간선 추가에 분할상환 O(1)
- **Tarjan 알고리즘**: 낮은 상수 인수를 가진 O(V + E)
- **메모리 접근 패턴**: 캐시 지역성에 최적화

### 8.2 공간 복잡도
- **그래프 저장**: 인접 리스트에 O(V + E)
- **알고리즘 오버헤드**: Tarjan 상태에 O(V)
- **결과 저장**: 컴포넌트 매핑에 O(V)

### 8.3 최적화 전략
1. **메모리 레이아웃**: 구조체 패킹 및 정렬
2. **캐시 최적화**: 가능한 경우 너비 우선 간선 순회
3. **분기 예측**: 핫 패스에서 조건부 분기 최소화
4. **SIMD**: 밀집 그래프에서 벡터화 연산 가능성

## 9. 테스팅 전략

### 9.1 테스트 범주
1. **단위 테스트**: 개별 함수 정확성
2. **통합 테스트**: 알고리즘 end-to-end 테스팅
3. **성능 테스트**: 확장성 및 타이밍 벤치마크
4. **스트레스 테스트**: 대형 그래프 처리 및 메모리 한계

### 9.2 테스트 케이스
- 빈 그래프
- 단일 정점 그래프
- 단순한 SCC (단일 정점)
- 사이클이 있는 복잡한 SCC
- 대규모 무작위 생성 그래프
- 실제 그래프 데이터셋

## 10. 빌드 시스템 및 종속성

### 10.1 빌드 구성
- **컴파일러**: GCC 9.0+ 또는 Clang 10.0+
- **표준**: C99 최소, C11 권장
- **빌드 시스템**: CMake 3.15+
- **종속성**: 없음 (독립적인 구현)

### 10.2 컴파일 플래그
```cmake
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fsanitize=address -fsanitize=undefined")
```

## 11. 사용 예제 및 통합

### 11.1 기본 사용법
```c
#include "scc.h"

int main() {
    graph_t* graph = graph_create(100);
    
    // 정점과 간선 추가
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    // SCC 찾기
    scc_result_t* result = scc_find(graph);
    
    printf("%d개의 강한 연결 요소를 찾았습니다\n", 
           scc_get_component_count(result));
    
    // 정리
    scc_result_destroy(result);
    graph_destroy(graph);
    
    return 0;
}
```

### 11.2 고급 사용법
```c
// 사용자 정의 메모리 관리로 대형 그래프 처리
graph_t* graph = graph_create_with_pools(1000000, vertex_pool, edge_pool);
scc_result_t* result = scc_find_tarjan(graph);

// 결과 분석
for (int i = 0; i < result->num_components; i++) {
    if (result->components[i].size > 100) {
        printf("큰 컴포넌트 %d는 %d개의 정점을 가집니다\n", 
               i, result->components[i].size);
    }
}

// 축약 그래프 구축
graph_t* condensed = scc_build_condensation_graph(graph, result);
```

## 12. 향후 확장

### 12.1 계획된 기능
1. **병렬 SCC**: 멀티스레드 구현
2. **외부 메모리**: RAM보다 큰 그래프 지원
3. **동적 업데이트**: 효율적인 증분 SCC 유지
4. **GPU 가속**: CUDA/OpenCL 구현

### 12.2 연구 방향
1. **근사 SCC**: 대규모 그래프에서 속도를 위한 정확성 교환
2. **분산 SCC**: 다중 머신 그래프 처리
3. **영구 데이터 구조**: 함수형 그래프 업데이트

## 13. 결론

이 아키텍처는 C에서 고성능 SCC 계산을 위한 견고한 기반을 제공합니다. 모듈형 설계는 코드의 명확성과 정확성을 유지하면서 쉬운 확장과 최적화를 가능하게 합니다. 이중 알고리즘 접근법(Tarjan + Kosaraju)은 성능과 교육적 가치를 모두 제공합니다.

구현에서 우선시하는 사항:
- **정확성**: 포괄적인 테스팅과 명확한 알고리즘
- **성능**: 최적화된 데이터 구조와 메모리 관리
- **사용성**: 깔끔한 API와 좋은 문서화
- **유지보수성**: 모듈형 설계와 일관된 코딩 표준

---

*문서 버전: 1.0*  
*최종 업데이트: 2025-09-09*  
*작성자: Claude Code Assistant*