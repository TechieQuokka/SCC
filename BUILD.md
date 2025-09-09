# SCC 라이브러리 빌드 가이드

이 문서는 SCC (Strongly Connected Components) 라이브러리를 빌드하고 설치하는 방법을 설명합니다.

## 요구사항

### 필수 요구사항

- **C 컴파일러**: GCC 4.9+ 또는 Clang 3.5+ 또는 MSVC 2015+
- **CMake**: 3.15 이상
- **Make**: Unix 시스템 (Linux, macOS)

### 선택적 요구사항

- **OpenMP**: 병렬 알고리즘 지원 (`-DSCC_ENABLE_PARALLEL=ON`)
- **Cairo**: 그래프 시각화 지원 (`-DSCC_ENABLE_VISUALIZATION=ON`)
- **Graphviz**: DOT 파일 렌더링 지원
- **Doxygen**: API 문서 생성

## 빌드 옵션

### CMake 빌드 옵션

| 옵션 | 기본값 | 설명 |
|------|--------|------|
| `SCC_BUILD_SHARED` | ON | 공유 라이브러리 빌드 |
| `SCC_BUILD_STATIC` | ON | 정적 라이브러리 빌드 |
| `SCC_BUILD_TESTS` | ON | 테스트 스위트 빌드 |
| `SCC_BUILD_EXAMPLES` | ON | 예제 프로그램 빌드 |
| `SCC_BUILD_BENCHMARKS` | OFF | 벤치마크 스위트 빌드 |
| `SCC_ENABLE_PARALLEL` | OFF | 병렬 알고리즘 활성화 |
| `SCC_ENABLE_VISUALIZATION` | OFF | 그래프 시각화 활성화 |
| `SCC_ENABLE_PROFILING` | OFF | 프로파일링 지원 활성화 |

## 빌드 방법

### 1. 간편한 빌드 스크립트 사용

#### Linux/macOS

```bash
# 기본 빌드
./scripts/build.sh

# 정리 후 빌드하고 테스트 실행
./scripts/build.sh --clean --test

# 디버그 빌드
./scripts/build.sh --debug

# 병렬 알고리즘 활성화하여 빌드
./scripts/build.sh --enable-parallel

# 설치까지 수행
./scripts/build.sh --clean --test --install --prefix /usr/local
```

#### Windows

```cmd
REM 기본 빌드
scripts\build.bat

REM 정리 후 빌드하고 테스트 실행
scripts\build.bat --clean --test

REM 디버그 빌드
scripts\build.bat --debug

REM 설치까지 수행
scripts\build.bat --clean --test --install
```

### 2. 수동 CMake 빌드

#### Linux/macOS

```bash
# 빌드 디렉토리 생성 및 이동
mkdir build && cd build

# CMake 설정 (Release 빌드)
cmake -DCMAKE_BUILD_TYPE=Release ..

# 빌드 실행
make -j$(nproc)

# 테스트 실행 (선택사항)
ctest

# 설치 (선택사항)
sudo make install
```

#### Windows (Visual Studio)

```cmd
REM 빌드 디렉토리 생성 및 이동
mkdir build
cd build

REM CMake 설정
cmake ..

REM 빌드 실행 (Release 모드)
cmake --build . --config Release

REM 테스트 실행 (선택사항)
ctest -C Release

REM 설치 (선택사항)
cmake --install . --config Release
```

### 3. 고급 빌드 설정

#### 모든 기능 활성화

```bash
mkdir build && cd build

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DSCC_BUILD_TESTS=ON \
    -DSCC_BUILD_EXAMPLES=ON \
    -DSCC_BUILD_BENCHMARKS=ON \
    -DSCC_ENABLE_PARALLEL=ON \
    -DSCC_ENABLE_VISUALIZATION=ON \
    -DSCC_ENABLE_PROFILING=ON \
    ..

make -j$(nproc)
```

#### 최소 빌드 (라이브러리만)

```bash
mkdir build && cd build

cmake \
    -DCMAKE_BUILD_TYPE=MinSizeRel \
    -DSCC_BUILD_TESTS=OFF \
    -DSCC_BUILD_EXAMPLES=OFF \
    -DSCC_BUILD_BENCHMARKS=OFF \
    ..

make -j$(nproc)
```

## 테스트 실행

### 전체 테스트 스위트

```bash
# CMake/CTest 사용
cd build
ctest

# 직접 실행
./scc_test
```

### 특정 모듈 테스트

```bash
# 그래프 모듈만 테스트
./scc_test graph

# 알고리즘 모듈들만 테스트
./scc_test tarjan kosaraju

# 성능 테스트
./scc_test performance
```

### 테스트 옵션

```bash
# 상세 출력
ctest --verbose

# 실패 시 출력
ctest --output-on-failure

# 병렬 테스트 실행
ctest -j$(nproc)
```

## 설치

### 시스템 전역 설치

```bash
# Linux/macOS
sudo make install

# 또는
sudo cmake --install . --config Release
```

### 사용자 정의 경로 설치

```bash
# 설치 경로 지정
cmake -DCMAKE_INSTALL_PREFIX=/opt/scc ..
make install
```

### Windows 설치

```cmd
REM 관리자 권한으로 실행
cmake --install . --config Release

REM 또는 특정 경로에 설치
cmake --install . --config Release --prefix C:\scc
```

## 패키지 사용

### CMake 프로젝트에서 사용

```cmake
find_package(scc REQUIRED)
target_link_libraries(your_target scc::scc)
```

### pkg-config 사용

```bash
# 컴파일 플래그 확인
pkg-config --cflags scc

# 링크 플래그 확인
pkg-config --libs scc

# 컴파일 예제
gcc -o myprogram myprogram.c $(pkg-config --cflags --libs scc)
```

## 문제 해결

### 일반적인 빌드 문제

1. **CMake를 찾을 수 없음**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install cmake
   
   # CentOS/RHEL
   sudo yum install cmake
   
   # macOS
   brew install cmake
   ```

2. **컴파일러 오류**
   - GCC 4.9+ 또는 Clang 3.5+ 사용 확인
   - C99 표준 지원 확인

3. **테스트 실패**
   ```bash
   # 상세 로그와 함께 테스트 재실행
   ctest --verbose --rerun-failed
   ```

4. **OpenMP 지원 없음**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libomp-dev
   
   # CentOS/RHEL
   sudo yum install openmp-devel
   ```

### Windows 특정 문제

1. **Visual Studio Build Tools 없음**
   - Visual Studio 또는 Build Tools for Visual Studio 설치 필요

2. **경로 문제**
   - 공백이 포함된 경로 사용 시 따옴표로 감싸기

### 성능 최적화

1. **Release 빌드 사용**
   ```bash
   -DCMAKE_BUILD_TYPE=Release
   ```

2. **네이티브 최적화 활성화**
   ```bash
   -DCMAKE_C_FLAGS="-O3 -march=native -flto"
   ```

3. **병렬 알고리즘 활성화**
   ```bash
   -DSCC_ENABLE_PARALLEL=ON
   ```

## 개발자를 위한 정보

### 디버그 빌드

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### 정적 분석

```bash
# Clang Static Analyzer
scan-build make

# Cppcheck
cppcheck --enable=all src/
```

### 메모리 검사

```bash
# Valgrind
valgrind --leak-check=full ./scc_test

# AddressSanitizer
cmake -DCMAKE_C_FLAGS="-fsanitize=address" ..
```

### 프로파일링

```bash
# Perf 사용
perf record ./scc_test performance
perf report

# Gprof 사용 (프로파일링 활성화된 빌드 필요)
cmake -DSCC_ENABLE_PROFILING=ON ..
```

## 기여 가이드

빌드 시스템 수정 시:

1. **CMakeLists.txt** 수정 후 모든 플랫폼에서 테스트
2. **빌드 스크립트** 업데이트 (`scripts/build.sh`, `scripts/build.bat`)
3. **문서 업데이트** (이 파일 포함)
4. **CI/CD 설정** 확인 (`.github/workflows/` 등)

## 라이센스

이 빌드 시스템은 SCC 라이브러리와 동일한 라이센스를 따릅니다.