cmake_minimum_required(VERSION 3.8)
project(ReParser CXX)

include(ExternalProject)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" ${CMAKE_SOURCE_DIR}/cmake)
include(CodeCoverage)

set(EXTERNAL_INSTALL_LOCATION ${ReParser_SOURCE_DIR}/3rdparty)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ReParser_SOURCE_DIR}/bin)

SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage --coverage")
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")

if (APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

ExternalProject_Add(
    googletest_repo
    GIT_REPOSITORY https://github.com/google/googletest.git
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

find_package(PythonInterp 3 REQUIRED)
set(PYTHON_EXECUTABLE "python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}")
message(${PYTHON_EXECUTABLE})

add_custom_target(
    InstallDeps
    COMMAND ${ReParser_SOURCE_DIR}/install_deps.sh ${PYTHON_EXECUTABLE}
    DEPENDS install_deps.sh
    WORKING_DIRECTORY ${ReParser_SOURCE_DIR}
    COMMENT "Installing dependencies"
)

add_custom_target(coverage_report)
add_dependencies(coverage_report InstallDeps)

function(run_coverage test_sources sources coverage_target target_name coverage_name)
    message(INFO test sources ${test_sources})
    set(test_name "${target_name}-test")
    message(INFO sources "${sources}")
    add_executable(${test_name} ${test_sources} ${sources})

    target_link_libraries(
		${test_name} gtest_main gtest Threads::Threads
	)

    setup_target_for_coverage_gcovr(
            ${coverage_target} ${target_name} ${test_name} ${coverage_name}
    )
    add_dependencies(${test_name} googletest_repo InstallDeps)
    add_dependencies(coverage_report ${coverage_target})
endfunction()


include_directories(include)
add_subdirectory(src/parser-library)

add_subdirectory(tests)


