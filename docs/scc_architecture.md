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
    
    // Tarjan-specific fields
    int index;
    int lowlink;
    bool on_stack;
    
    // General purpose
    bool visited;
    void* data;  // User data
} vertex_t;

typedef struct graph {
    vertex_t** vertices;
    int num_vertices;
    int num_edges;
    int capacity;
    
    // Memory management
    struct memory_pool* vertex_pool;
    struct memory_pool* edge_pool;
} graph_t;
```

#### 4.1.2 SCC Result Structure
```c
typedef struct scc_component {
    int* vertices;
    int size;
    int capacity;
} scc_component_t;

typedef struct scc_result {
    scc_component_t* components;
    int num_components;
    int* vertex_to_component;  // Mapping array
    
    // Statistics
    int largest_component_size;
    int smallest_component_size;
    double average_component_size;
} scc_result_t;
```

### 4.2 Algorithm-Specific Structures

#### 4.2.1 Tarjan's Algorithm State
```c
typedef struct tarjan_state {
    int* stack;
    int stack_top;
    int current_index;
    
    scc_result_t* result;
    int current_component;
} tarjan_state_t;
```

#### 4.2.2 Kosaraju's Algorithm State
```c
typedef struct kosaraju_state {
    int* finish_order;
    int finish_index;
    graph_t* transpose_graph;
    
    scc_result_t* result;
    int current_component;
} kosaraju_state_t;
```

## 5. API Design

### 5.1 Core API Functions

#### 5.1.1 Graph Management
```c
// Graph creation and destruction
graph_t* graph_create(int initial_capacity);
void graph_destroy(graph_t* graph);

// Graph modification
int graph_add_vertex(graph_t* graph);
int graph_add_edge(graph_t* graph, int src, int dest);
int graph_remove_edge(graph_t* graph, int src, int dest);

// Graph queries
bool graph_has_edge(const graph_t* graph, int src, int dest);
int graph_get_out_degree(const graph_t* graph, int vertex);
```

#### 5.1.2 SCC Computation
```c
// Main SCC functions
scc_result_t* scc_find_tarjan(const graph_t* graph);
scc_result_t* scc_find_kosaraju(const graph_t* graph);
scc_result_t* scc_find(const graph_t* graph);  // Default algorithm

// Result management
void scc_result_destroy(scc_result_t* result);
```

#### 5.1.3 Utility Functions
```c
// Result analysis
int scc_get_component_count(const scc_result_t* result);
int scc_get_component_size(const scc_result_t* result, int component_id);
int scc_get_vertex_component(const scc_result_t* result, int vertex);

// Graph properties
bool scc_is_strongly_connected(const graph_t* graph);
graph_t* scc_build_condensation_graph(const graph_t* graph, const scc_result_t* scc);
```

### 5.2 Advanced API

#### 5.2.1 Streaming/Online Processing
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

## 6. Memory Management Strategy

### 6.1 Memory Pool Design
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

### 6.2 Allocation Strategy
- **Small allocations**: Use memory pools for vertices and edges
- **Large allocations**: Direct malloc/free for result structures
- **Stack allocations**: Use for temporary algorithm state when possible

## 7. Error Handling

### 7.1 Error Codes
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

### 7.2 Error Handling Pattern
```c
scc_result_t* result = scc_find_tarjan(graph);
if (!result) {
    int error = scc_get_last_error();
    fprintf(stderr, "SCC computation failed: %s\n", scc_error_string(error));
    return -1;
}
```

## 8. Performance Considerations

### 8.1 Time Complexity Analysis
- **Graph Creation**: O(V) for vertex allocation, O(1) amortized for edge addition
- **Tarjan's Algorithm**: O(V + E) with low constant factors
- **Memory Access Patterns**: Optimized for cache locality

### 8.2 Space Complexity
- **Graph Storage**: O(V + E) for adjacency lists
- **Algorithm Overhead**: O(V) for Tarjan's state
- **Result Storage**: O(V) for component mapping

### 8.3 Optimization Strategies
1. **Memory Layout**: Structure packing and alignment
2. **Cache Optimization**: Breadth-first edge traversal where possible  
3. **Branch Prediction**: Minimize conditional branches in hot paths
4. **SIMD**: Potential for vectorized operations in dense graphs

## 9. Testing Strategy

### 9.1 Test Categories
1. **Unit Tests**: Individual function correctness
2. **Integration Tests**: Algorithm end-to-end testing
3. **Performance Tests**: Scalability and timing benchmarks
4. **Stress Tests**: Large graph handling and memory limits

### 9.2 Test Cases
- Empty graphs
- Single vertex graphs
- Trivial SCCs (single vertices)
- Complex SCCs with cycles
- Large randomly generated graphs
- Real-world graph datasets

## 10. Build System and Dependencies

### 10.1 Build Configuration
- **Compiler**: GCC 9.0+ or Clang 10.0+
- **Standards**: C99 minimum, C11 recommended
- **Build System**: CMake 3.15+
- **Dependencies**: None (self-contained implementation)

### 10.2 Compilation Flags
```cmake
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -fsanitize=address -fsanitize=undefined")
```

## 11. Usage Examples and Integration

### 11.1 Basic Usage
```c
#include "scc.h"

int main() {
    graph_t* graph = graph_create(100);
    
    // Add vertices and edges
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);
    
    // Find SCCs
    scc_result_t* result = scc_find(graph);
    
    printf("Found %d strongly connected components\n", 
           scc_get_component_count(result));
    
    // Cleanup
    scc_result_destroy(result);
    graph_destroy(graph);
    
    return 0;
}
```

### 11.2 Advanced Usage
```c
// Process large graph with custom memory management
graph_t* graph = graph_create_with_pools(1000000, vertex_pool, edge_pool);
scc_result_t* result = scc_find_tarjan(graph);

// Analyze results
for (int i = 0; i < result->num_components; i++) {
    if (result->components[i].size > 100) {
        printf("Large component %d has %d vertices\n", 
               i, result->components[i].size);
    }
}

// Build condensation graph
graph_t* condensed = scc_build_condensation_graph(graph, result);
```

## 12. Future Extensions

### 12.1 Planned Features
1. **Parallel SCC**: Multi-threaded implementations
2. **External Memory**: Support for graphs larger than RAM
3. **Dynamic Updates**: Efficient incremental SCC maintenance
4. **GPU Acceleration**: CUDA/OpenCL implementations

### 12.2 Research Directions
1. **Approximate SCCs**: Trade accuracy for speed in massive graphs
2. **Distributed SCCs**: Cross-machine graph processing
3. **Persistent Data Structures**: Functional graph updates

## 13. Conclusion

This architecture provides a solid foundation for high-performance SCC computation in C. The modular design allows for easy extension and optimization while maintaining code clarity and correctness. The dual algorithm approach (Tarjan + Kosaraju) provides both performance and educational value.

The implementation prioritizes:
- **Correctness**: Comprehensive testing and clear algorithms
- **Performance**: Optimized data structures and memory management  
- **Usability**: Clean APIs and good documentation
- **Maintainability**: Modular design and consistent coding standards

---

*Document Version: 1.0*  
*Last Updated: 2025-09-09*  
*Author: Claude Code Assistant*