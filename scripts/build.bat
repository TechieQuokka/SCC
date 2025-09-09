@echo off
setlocal enabledelayedexpansion

REM SCC 라이브러리 빌드 스크립트 (Windows)
REM 사용법: scripts\build.bat [옵션]

set BUILD_TYPE=Release
set BUILD_DIR=build
set INSTALL_PREFIX=
set ENABLE_TESTS=ON
set ENABLE_EXAMPLES=ON
set ENABLE_PARALLEL=OFF
set ENABLE_VISUALIZATION=OFF
set CLEAN_BUILD=false
set RUN_TESTS=false
set INSTALL=false
set VERBOSE=false

REM 명령행 인수 파싱
:parse_args
if "%~1"=="" goto end_parse
if "%~1"=="-h" goto show_help
if "%~1"=="--help" goto show_help
if "%~1"=="-c" set CLEAN_BUILD=true&& shift && goto parse_args
if "%~1"=="--clean" set CLEAN_BUILD=true&& shift && goto parse_args
if "%~1"=="-t" set RUN_TESTS=true&& shift && goto parse_args
if "%~1"=="--test" set RUN_TESTS=true&& shift && goto parse_args
if "%~1"=="-i" set INSTALL=true&& shift && goto parse_args
if "%~1"=="--install" set INSTALL=true&& shift && goto parse_args
if "%~1"=="-v" set VERBOSE=true&& shift && goto parse_args
if "%~1"=="--verbose" set VERBOSE=true&& shift && goto parse_args
if "%~1"=="--debug" set BUILD_TYPE=Debug&& shift && goto parse_args
if "%~1"=="--build-dir" set BUILD_DIR=%~2&& shift && shift && goto parse_args
if "%~1"=="--prefix" set INSTALL_PREFIX=%~2&& shift && shift && goto parse_args
if "%~1"=="--no-tests" set ENABLE_TESTS=OFF&& shift && goto parse_args
if "%~1"=="--no-examples" set ENABLE_EXAMPLES=OFF&& shift && goto parse_args
if "%~1"=="--enable-parallel" set ENABLE_PARALLEL=ON&& shift && goto parse_args
if "%~1"=="--enable-viz" set ENABLE_VISUALIZATION=ON&& shift && goto parse_args
echo 알 수 없는 옵션: %~1
echo 도움말을 보려면 %~0 --help를 실행하세요.
exit /b 1

:show_help
echo SCC 라이브러리 빌드 스크립트 (Windows)
echo.
echo 사용법: %~0 [옵션]
echo.
echo 옵션:
echo   -h, --help              이 도움말 표시
echo   -c, --clean             기존 빌드 디렉토리 정리 후 빌드
echo   -t, --test              빌드 후 테스트 실행
echo   -i, --install           빌드 후 설치
echo   -v, --verbose           상세 출력
echo.
echo 빌드 설정:
echo   --debug                 디버그 빌드 (기본: Release)
echo   --build-dir DIR         빌드 디렉토리 지정 (기본: build)
echo   --prefix DIR            설치 경로 지정
echo   --no-tests              테스트 비활성화
echo   --no-examples           예제 비활성화
echo   --enable-parallel       병렬 알고리즘 활성화
echo   --enable-viz            그래프 시각화 활성화
echo.
echo 예제:
echo   %~0                     기본 빌드
echo   %~0 -c -t              정리 후 빌드하고 테스트 실행
echo   %~0 --debug --test     디버그 빌드 후 테스트
exit /b 0

:end_parse

REM 시작 시간 기록
set START_TIME=%time%

echo [INFO] SCC 라이브러리 빌드 시작
echo [INFO] 빌드 타입: %BUILD_TYPE%
echo [INFO] 빌드 디렉토리: %BUILD_DIR%

REM 프로젝트 루트 디렉토리로 이동
cd /d "%~dp0.."

REM 기존 빌드 디렉토리 정리
if "%CLEAN_BUILD%"=="true" (
    echo [INFO] 기존 빌드 디렉토리 정리 중...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
        echo [SUCCESS] 빌드 디렉토리 정리 완료
    )
)

REM 빌드 디렉토리 생성
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

REM CMake 설정 구성
set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DSCC_BUILD_TESTS=%ENABLE_TESTS% -DSCC_BUILD_EXAMPLES=%ENABLE_EXAMPLES% -DSCC_ENABLE_PARALLEL=%ENABLE_PARALLEL% -DSCC_ENABLE_VISUALIZATION=%ENABLE_VISUALIZATION%

if not "%INSTALL_PREFIX%"=="" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_INSTALL_PREFIX=%INSTALL_PREFIX%
)

if "%VERBOSE%"=="true" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON
)

REM CMake 구성
echo [INFO] CMake 구성 중...
if "%VERBOSE%"=="true" (
    echo 실행 명령: cmake %CMAKE_ARGS% ..
)

cmake %CMAKE_ARGS% ..
if errorlevel 1 (
    echo [ERROR] CMake 구성 실패
    exit /b 1
)
echo [SUCCESS] CMake 구성 완료

REM CPU 수 확인 (Windows)
set JOBS=%NUMBER_OF_PROCESSORS%
if "%JOBS%"=="" set JOBS=4

REM 빌드 실행
echo [INFO] 빌드 실행 중... (병렬 작업 수: %JOBS%)
set BUILD_ARGS=
if "%VERBOSE%"=="true" set BUILD_ARGS=--verbose

cmake --build . --config %BUILD_TYPE% --parallel %JOBS% %BUILD_ARGS%
if errorlevel 1 (
    echo [ERROR] 빌드 실패
    exit /b 1
)
echo [SUCCESS] 빌드 완료

REM 테스트 실행
if "%RUN_TESTS%"=="true" (
    if "%ENABLE_TESTS%"=="ON" (
        echo [INFO] 테스트 실행 중...
        
        if "%VERBOSE%"=="true" (
            ctest --output-on-failure --verbose
        ) else (
            ctest --output-on-failure
        )
        
        if errorlevel 1 (
            echo [WARNING] 일부 테스트 실패
        ) else (
            echo [SUCCESS] 모든 테스트 통과
        )
    )
)

REM 설치
if "%INSTALL%"=="true" (
    echo [INFO] 설치 중...
    
    cmake --install . --config %BUILD_TYPE%
    if errorlevel 1 (
        echo [ERROR] 설치 실패
        exit /b 1
    )
    echo [SUCCESS] 설치 완료
    if not "%INSTALL_PREFIX%"=="" (
        echo [INFO] 설치 위치: %INSTALL_PREFIX%
    )
)

REM 완료 메시지
echo.
echo [SUCCESS] 전체 작업 완료
echo [INFO] 빌드 결과 요약:
echo   - 빌드 타입: %BUILD_TYPE%
echo   - 빌드 디렉토리: %BUILD_DIR%
echo   - 테스트: %ENABLE_TESTS%
echo   - 예제: %ENABLE_EXAMPLES%
echo   - 병렬 처리: %ENABLE_PARALLEL%
echo   - 시각화: %ENABLE_VISUALIZATION%

if exist "*.lib" (
    echo [SUCCESS] 정적 라이브러리 파일이 성공적으로 생성되었습니다.
)
if exist "*.dll" (
    echo [SUCCESS] 동적 라이브러리 파일이 성공적으로 생성되었습니다.
)

if "%ENABLE_TESTS%"=="ON" (
    if exist "scc_test.exe" (
        echo [INFO] 테스트 실행: scc_test.exe
    )
)

if "%ENABLE_EXAMPLES%"=="ON" (
    if exist "examples\*.exe" (
        echo [INFO] 예제 프로그램들이 examples\ 디렉토리에 빌드되었습니다.
    )
)

endlocal