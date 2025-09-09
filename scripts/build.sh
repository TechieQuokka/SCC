#!/bin/bash

# SCC 라이브러리 빌드 스크립트
# 사용법: ./scripts/build.sh [옵션]

set -e

# 기본 설정
BUILD_TYPE="Release"
BUILD_DIR="build"
INSTALL_PREFIX=""
ENABLE_TESTS="ON"
ENABLE_EXAMPLES="ON"
ENABLE_PARALLEL="OFF"
ENABLE_VISUALIZATION="OFF"
CLEAN_BUILD=false
RUN_TESTS=false
INSTALL=false
VERBOSE=false

# 도움말 표시
show_help() {
    cat << EOF
SCC 라이브러리 빌드 스크립트

사용법: $0 [옵션]

옵션:
  -h, --help              이 도움말 표시
  -c, --clean             기존 빌드 디렉토리 정리 후 빌드
  -t, --test              빌드 후 테스트 실행
  -i, --install           빌드 후 설치
  -v, --verbose           상세 출력
  
빌드 설정:
  --debug                 디버그 빌드 (기본: Release)
  --build-dir DIR         빌드 디렉토리 지정 (기본: build)
  --prefix DIR            설치 경로 지정
  --no-tests              테스트 비활성화
  --no-examples           예제 비활성화
  --enable-parallel       병렬 알고리즘 활성화
  --enable-viz            그래프 시각화 활성화

예제:
  $0                      기본 빌드
  $0 -c -t               정리 후 빌드하고 테스트 실행
  $0 --debug --test      디버그 빌드 후 테스트
  $0 --prefix /usr/local --install  /usr/local에 설치

EOF
}

# 명령행 인수 파싱
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -t|--test)
            RUN_TESTS=true
            shift
            ;;
        -i|--install)
            INSTALL=true
            shift
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --prefix)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --no-tests)
            ENABLE_TESTS="OFF"
            shift
            ;;
        --no-examples)
            ENABLE_EXAMPLES="OFF"
            shift
            ;;
        --enable-parallel)
            ENABLE_PARALLEL="ON"
            shift
            ;;
        --enable-viz)
            ENABLE_VISUALIZATION="ON"
            shift
            ;;
        *)
            echo "알 수 없는 옵션: $1"
            echo "도움말을 보려면 $0 --help를 실행하세요."
            exit 1
            ;;
    esac
done

# 색상 출력을 위한 함수들
if [[ -t 1 ]]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m'
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 시간 측정
start_time=$(date +%s)

log_info "SCC 라이브러리 빌드 시작"
log_info "빌드 타입: $BUILD_TYPE"
log_info "빌드 디렉토리: $BUILD_DIR"

# 프로젝트 루트 디렉토리로 이동
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

# 기존 빌드 디렉토리 정리
if [[ "$CLEAN_BUILD" == true ]]; then
    log_info "기존 빌드 디렉토리 정리 중..."
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
        log_success "빌드 디렉토리 정리 완료"
    fi
fi

# 빌드 디렉토리 생성
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# CMake 설정 구성
CMAKE_ARGS=(
    "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    "-DSCC_BUILD_TESTS=$ENABLE_TESTS"
    "-DSCC_BUILD_EXAMPLES=$ENABLE_EXAMPLES"
    "-DSCC_ENABLE_PARALLEL=$ENABLE_PARALLEL"
    "-DSCC_ENABLE_VISUALIZATION=$ENABLE_VISUALIZATION"
)

if [[ -n "$INSTALL_PREFIX" ]]; then
    CMAKE_ARGS+=("-DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX")
fi

if [[ "$VERBOSE" == true ]]; then
    CMAKE_ARGS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
fi

# CMake 구성
log_info "CMake 구성 중..."
if [[ "$VERBOSE" == true ]]; then
    echo "실행 명령: cmake ${CMAKE_ARGS[*]} .."
fi

if cmake "${CMAKE_ARGS[@]}" ..; then
    log_success "CMake 구성 완료"
else
    log_error "CMake 구성 실패"
    exit 1
fi

# 병렬 빌드를 위한 CPU 수 확인
if command -v nproc >/dev/null 2>&1; then
    JOBS=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    JOBS=4
fi

# 빌드 실행
log_info "빌드 실행 중... (병렬 작업 수: $JOBS)"
if [[ "$VERBOSE" == true ]]; then
    BUILD_ARGS="--verbose"
else
    BUILD_ARGS=""
fi

if cmake --build . --config "$BUILD_TYPE" --parallel "$JOBS" $BUILD_ARGS; then
    log_success "빌드 완료"
else
    log_error "빌드 실패"
    exit 1
fi

# 테스트 실행
if [[ "$RUN_TESTS" == true ]] && [[ "$ENABLE_TESTS" == "ON" ]]; then
    log_info "테스트 실행 중..."
    
    if [[ "$VERBOSE" == true ]]; then
        ctest --output-on-failure --verbose
    else
        ctest --output-on-failure
    fi
    
    if [[ $? -eq 0 ]]; then
        log_success "모든 테스트 통과"
    else
        log_warning "일부 테스트 실패"
    fi
fi

# 설치
if [[ "$INSTALL" == true ]]; then
    log_info "설치 중..."
    
    if [[ -n "$INSTALL_PREFIX" ]] && [[ ! -w "$INSTALL_PREFIX" ]]; then
        log_warning "설치 권한이 필요할 수 있습니다. sudo를 사용하여 설치하세요."
        exit 1
    fi
    
    if cmake --install . --config "$BUILD_TYPE"; then
        log_success "설치 완료"
        if [[ -n "$INSTALL_PREFIX" ]]; then
            log_info "설치 위치: $INSTALL_PREFIX"
        fi
    else
        log_error "설치 실패"
        exit 1
    fi
fi

# 빌드 시간 계산
end_time=$(date +%s)
duration=$((end_time - start_time))
minutes=$((duration / 60))
seconds=$((duration % 60))

log_success "전체 작업 완료"
if [[ $minutes -gt 0 ]]; then
    log_info "소요 시간: ${minutes}분 ${seconds}초"
else
    log_info "소요 시간: ${seconds}초"
fi

# 빌드 결과 요약
echo
log_info "빌드 결과 요약:"
echo "  - 빌드 타입: $BUILD_TYPE"
echo "  - 빌드 디렉토리: $BUILD_DIR"
echo "  - 테스트: $ENABLE_TESTS"
echo "  - 예제: $ENABLE_EXAMPLES"
echo "  - 병렬 처리: $ENABLE_PARALLEL"
echo "  - 시각화: $ENABLE_VISUALIZATION"

if [[ -f "libscc.a" ]] || [[ -f "libscc.so" ]] || [[ -f "libscc.dylib" ]]; then
    log_success "라이브러리 파일이 성공적으로 생성되었습니다."
fi

if [[ "$ENABLE_TESTS" == "ON" ]] && [[ -f "scc_test" ]]; then
    log_info "테스트 실행: ./scc_test"
fi

if [[ "$ENABLE_EXAMPLES" == "ON" ]]; then
    if ls examples/scc_* >/dev/null 2>&1; then
        log_info "예제 프로그램들이 examples/ 디렉토리에 빌드되었습니다."
    fi
fi