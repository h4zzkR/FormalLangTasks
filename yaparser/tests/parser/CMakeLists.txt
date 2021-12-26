set(TARGET_NAME "parser-library")
set(COVERAGE_NAME "parser-coverage")
set(COVERAGE_TARGET "coverage-parser-library")
set(SOURCE_DIR_MAIN "${parser-library_SOURCE_DIR}")
set(SOURCES ${parser-source-files})

message(INFO ${SOURCES})

set(TEST_SOURCES unit.cpp)
run_coverage(
    "${TEST_SOURCES}"
    "${SOURCES}"
    "${COVERAGE_TARGET}"
    "${TARGET_NAME}"
    "${COVERAGE_NAME}"
)