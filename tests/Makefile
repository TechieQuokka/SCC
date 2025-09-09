# SCC 테스트 스위트 Makefile

# 컴파일러 설정
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -O2
INCLUDES = -I../include -I../src

# 디렉토리 설정
SRC_DIR = ../src
INCLUDE_DIR = ../include
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# 소스 파일들
SRC_FILES = $(SRC_DIR)/graph.c \
            $(SRC_DIR)/scc.c \
            $(SRC_DIR)/tarjan.c \
            $(SRC_DIR)/kosaraju.c \
            $(SRC_DIR)/memory.c \
            $(SRC_DIR)/utils.c \
            $(SRC_DIR)/graph_io.c

TEST_FILES = test_framework.c \
             test_graph.c \
             test_scc.c \
             test_tarjan.c \
             test_kosaraju.c \
             test_memory.c \
             test_utils.c \
             test_io.c \
             test_integration.c \
             test_performance.c \
             test_main.c

# 오브젝트 파일들
SRC_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
TEST_OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(TEST_FILES))

# 실행 파일
TARGET = $(BUILD_DIR)/scc_test

.PHONY: all clean test help directories

all: directories $(TARGET)

# 디렉토리 생성
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)

# 메인 타겟
$(TARGET): $(SRC_OBJS) $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

# 소스 파일 컴파일
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 테스트 파일 컴파일
$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 테스트 실행
test: $(TARGET)
	@echo "=== SCC 라이브러리 테스트 실행 ==="
	$(TARGET)

# 특정 모듈 테스트
test-graph: $(TARGET)
	$(TARGET) graph

test-scc: $(TARGET)
	$(TARGET) scc

test-tarjan: $(TARGET)
	$(TARGET) tarjan

test-kosaraju: $(TARGET)
	$(TARGET) kosaraju

test-memory: $(TARGET)
	$(TARGET) memory

test-utils: $(TARGET)
	$(TARGET) utils

test-io: $(TARGET)
	$(TARGET) io

test-integration: $(TARGET)
	$(TARGET) integration

test-performance: $(TARGET)
	$(TARGET) performance

# 정리
clean:
	rm -rf $(BUILD_DIR)

# 도움말
help:
	@echo "사용 가능한 타겟:"
	@echo "  all             - 테스트 스위트 빌드"
	@echo "  test            - 모든 테스트 실행"
	@echo "  test-<모듈>     - 특정 모듈 테스트 실행"
	@echo "  clean           - 빌드 파일 정리"
	@echo "  help            - 이 도움말 표시"
	@echo ""
	@echo "사용 가능한 모듈:"
	@echo "  graph, scc, tarjan, kosaraju, memory, utils, io, integration, performance"

# 의존성 (헤더 파일 변경 시 재컴파일)
$(SRC_OBJS): $(INCLUDE_DIR)/*.h
$(TEST_OBJS): test_framework.h $(INCLUDE_DIR)/*.h