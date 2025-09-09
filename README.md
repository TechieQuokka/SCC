# SCC - 고성능 강한 연결 요소 라이브러리

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/TechieQuokka/SCC)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/standard-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Version](https://img.shields.io/badge/version-1.0.0-green.svg)](https://github.com/TechieQuokka/SCC/releases)

방향 그래프에서 강한 연결 요소(SCC)를 계산하는 상용 수준의 C 라이브러리입니다. 최적화된 데이터 구조와 포괄적인 테스트를 통해 Tarjan과 Kosaraju 알고리즘을 모두 구현했습니다.

## 🚀 주요 기능

- **이중 알고리즘 지원**: Tarjan의 O(V+E) 단일 패스와 Kosaraju의 O(V+E) 이중 패스 알고리즘 모두 구현
- **메모리 효율성**: 사용자 정의 메모리 풀과 캐시 최적화 데이터 구조
- **고성능**: 수백만 개의 정점과 간선을 가진 그래프에서 벤치마크 완료
- **상용 수준**: 포괄적인 오류 처리, 스레드 안전성 고려, 광범위한 테스트
- **쉬운 통합**: CMake 빌드 시스템과 pkg-config 지원을 통한 깔끔한 C99 API
- **풍부한 문서화**: 완전한 API 문서와 구현 가이드

## 📊 빠른 시작

### 기본 사용법

```c
#include <scc.h>

int main() {
    // 그래프 생성
    graph_t* graph = graph_create(100);
    
    // 정점 추가
    for (int i = 0; i < 6; i++) {
        graph_add_vertex(graph);
    }
    
    // SCC를 만들기 위한 간선 추가
    graph_add_edge(graph, 0, 1);
    graph_add_edge(graph, 1, 2);
    graph_add_edge(graph, 2, 0);  // 첫 번째 SCC: {0,1,2}
    
    graph_add_edge(graph, 3, 4);
    graph_add_edge(graph, 4, 3);  // 두 번째 SCC: {3,4}
    
    // 강한 연결 요소 찾기
    scc_result_t* result = scc_find(graph);
    
    printf("%d개의 강한 연결 요소를 찾았습니다\n", 
           scc_get_component_count(result));
    
    // 결과 처리
    for (int i = 0; i < scc_get_component_count(result); i++) {
        int size = scc_get_component_size(result, i);
        const int* vertices = scc_get_component_vertices(result, i);
        
        printf("컴포넌트 %d: ", i);
        for (int j = 0; j < size; j++) {
            printf("%d ", vertices[j]);
        }
        printf("\n");
    }
    
    // 정리
    scc_result_destroy(result);
    graph_destroy(graph);
    return 0;
}
```

## 🛠 설치

### 요구사항

- C99 호환 컴파일러 (GCC 7+, Clang 8+, MSVC 2019+)
- CMake 3.15 이상
- 선택사항: 병렬 알고리즘을 위한 OpenMP

### 소스에서 빌드

```bash
git clone https://github.com/your-org/scc.git
cd scc
mkdir build && cd build

# 구성
cmake .. -DCMAKE_BUILD_TYPE=Release

# 빌드
cmake --build .

# 설치
sudo cmake --install .
```

### CMake 통합

```cmake
find_package(scc REQUIRED)
target_link_libraries(your_target PRIVATE scc::scc)
```

### pkg-config 통합

```bash
gcc your_program.c $(pkg-config --cflags --libs scc) -o your_program
```

## 📈 성능

Intel i7-10700K, 32GB RAM에서의 벤치마크 결과:

| 그래프 종류 | 정점 수 | 간선 수 | Tarjan (ms) | Kosaraju (ms) |
|-------------|---------|---------|-------------|---------------|
| 희소 그래프 | 100K    | 200K    | 45          | 62           |
| 밀집 그래프 | 10K     | 1M      | 180         | 210          |
| 소셜 네트워크 | 1M      | 10M     | 1,200       | 1,800        |

## 🏗 아키텍처 개요

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

## 📚 API 참조

### 핵심 함수

#### 그래프 관리
- `graph_t* graph_create(int initial_capacity)` - 그래프 생성
- `void graph_destroy(graph_t* graph)` - 그래프 제거
- `int graph_add_vertex(graph_t* graph)` - 정점 추가
- `int graph_add_edge(graph_t* graph, int src, int dest)` - 간선 추가

#### SCC 계산
- `scc_result_t* scc_find(const graph_t* graph)` - 기본 알고리즘
- `scc_result_t* scc_find_tarjan(const graph_t* graph)` - Tarjan 알고리즘
- `scc_result_t* scc_find_kosaraju(const graph_t* graph)` - Kosaraju 알고리즘

#### 결과 분석
- `int scc_get_component_count(const scc_result_t* result)` - 컴포넌트 개수
- `int scc_get_component_size(const scc_result_t* result, int component_id)` - 컴포넌트 크기
- `const int* scc_get_component_vertices(const scc_result_t* result, int component_id)` - 컴포넌트 정점들

전체 참조는 [API 문서](docs/api_reference.md)를 참조하세요.

## 🧪 예제

`examples/` 디렉토리에 포괄적인 사용 예제가 포함되어 있습니다:

- [`basic_usage.c`](examples/basic_usage.c) - 기본 연산
- [`algorithm_comparison.c`](examples/algorithm_comparison.c) - 성능 비교
- [`memory_management.c`](examples/memory_management.c) - 사용자 정의 메모리 풀
- [`graph_io.c`](examples/graph_io.c) - 파일 I/O 연산

예제 빌드 및 실행:

```bash
cmake --build . --target examples
./examples/basic_usage
./examples/algorithm_comparison
```

## 🔬 알고리즘 세부사항

### Tarjan 알고리즘
- **시간 복잡도**: O(V + E)
- **공간 복잡도**: O(V)
- **장점**: 단일 DFS 패스, 더 나은 캐시 지역성
- **최적 사용**: 일반적인 목적, 메모리 제약 환경

### Kosaraju 알고리즘  
- **시간 복잡도**: O(V + E)
- **공간 복잡도**: O(V + E) (전치 그래프 포함)
- **장점**: 단순한 로직, 이해하기 쉬움
- **최적 사용**: 교육 목적, 디버깅

### 알고리즘 선택

라이브러리는 그래프 특성에 따라 최적의 알고리즘을 자동으로 선택합니다:

```c
scc_algorithm_choice_t recommended = scc_recommend_algorithm(graph);
printf("추천 알고리즘: %s\n", scc_algorithm_name(recommended));
```

## 🧪 테스트

다음을 포괄하는 포괄적인 테스트 모음:

- 모든 API 함수에 대한 단위 테스트
- 알고리즘 정확성 검증  
- 성능 벤치마크
- 메모리 누수 감지
- 경계 케이스와 오류 조건

```bash
# 모든 테스트 실행
cmake --build . --target test

# 특정 테스트 범주 실행
ctest -R unit
ctest -R performance
ctest -R integration
```

## 📖 문서

### 기술 문서
- [아키텍처 설계](docs/scc_architecture.md) - 시스템 개요 및 설계 결정
- [구현 가이드](docs/implementation_guide.md) - 상세한 기술 명세서
- [API 참조](docs/api_reference.md) - 완전한 함수 문서
- [성능 분석](docs/performance.md) - 벤치마크 및 최적화 기법

### 알고리즘 자료
- [Tarjan의 SCC 알고리즘](https://en.wikipedia.org/wiki/Tarjan%27s_strongly_connected_components_algorithm)
- [Kosaraju의 SCC 알고리즘](https://en.wikipedia.org/wiki/Kosaraju%27s_algorithm)

## 🤝 기여

기여를 환영합니다! 가이드라인은 [CONTRIBUTING.md](CONTRIBUTING.md)를 참조하세요.

### 개발 환경 설정

```bash
git clone https://github.com/your-org/scc.git
cd scc

# 모든 검사가 활성화된 개발 빌드
mkdir build-dev && cd build-dev
cmake .. -DCMAKE_BUILD_TYPE=Debug -DSCC_BUILD_TESTS=ON -DSCC_BUILD_EXAMPLES=ON

# 빌드 및 테스트
make -j$(nproc)
make test
```

## 📄 라이센스

이 프로젝트는 MIT 라이센스 하에 있습니다. 자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 🎯 응용 분야

이 라이브러리는 다음 분야에 적합합니다:

- **컴파일러 최적화**: 죽은 코드 제거, 레지스터 할당
- **소셜 네트워크 분석**: 커뮤니티 감지, 영향력 전파
- **웹 그래프 분석**: PageRank 계산, 링크 분석
- **회로 설계**: 타이밍 분석, 논리 최적화  
- **데이터베이스 시스템**: 쿼리 최적화, 의존성 분석

## 📊 벤치마크 결과

최신 벤치마크 결과는 [benchmarks/](benchmarks/) 디렉토리에서 확인할 수 있습니다:

- 메모리 사용량 프로파일
- 확장성 분석  
- 알고리즘 비교 차트
- 실제 데이터셋 성능

## 🆘 지원

- 📧 **이메일**: support@techie-quokka.com
- 💬 **이슈**: [GitHub Issues](https://github.com/TechieQuokka/SCC/issues)
- 📖 **문서**: [Wiki](https://github.com/TechieQuokka/SCC/wiki)
- 💡 **토론**: [GitHub Discussions](https://github.com/TechieQuokka/SCC/discussions)

## 🏆 감사의 말

- 우아한 단일 패스 알고리즘을 개발한 Robert Tarjan
- 직관적인 이중 패스 접근법을 제시한 S. Rao Kosaraju
- 귀중한 피드백을 제공해주신 오픈소스 커뮤니티

---

**SCC 팀이 ❤️로 만들었습니다**