cmake_minimum_required(VERSION 3.10)
project(fastcond VERSION 1.0.0 LANGUAGES C)

# Project options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(FASTCOND_BUILD_TESTS "Build tests" ON)
option(FASTCOND_BUILD_BENCHMARKS "Build benchmarks" ON)

# C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find required packages
find_package(Threads REQUIRED)

# ============================================================================
# Library target
# ============================================================================
add_library(fastcond
    fastcond/fastcond.c
    fastcond/fastcond.h
    fastcond/fastcond_patch.h
)

target_include_directories(fastcond
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fastcond>
        $<INSTALL_INTERFACE:include/fastcond>
)

target_link_libraries(fastcond
    PUBLIC
        Threads::Threads
)

# Compiler warnings
target_compile_options(fastcond PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

# ============================================================================
# Tests and Benchmarks
# ============================================================================
if(FASTCOND_BUILD_TESTS OR FASTCOND_BUILD_BENCHMARKS)
    enable_testing()

    # Find math library (needed for benchmarks)
    find_library(MATH_LIBRARY m)
    if(NOT MATH_LIBRARY)
        set(MATH_LIBRARY "")
    endif()

    # Helper function to create test variants
    function(add_test_variant TEST_NAME SOURCE_FILE)
        # Pthread version (baseline)
        add_executable(${TEST_NAME}_pt ${SOURCE_FILE})
        target_link_libraries(${TEST_NAME}_pt PRIVATE fastcond ${MATH_LIBRARY})
        
        # Fastcond weak version
        add_executable(${TEST_NAME}_wcond ${SOURCE_FILE})
        target_compile_definitions(${TEST_NAME}_wcond PRIVATE FASTCOND_PATCH_WCOND)
        target_link_libraries(${TEST_NAME}_wcond PRIVATE fastcond ${MATH_LIBRARY})
        
        # Fastcond strong version (default)
        add_executable(${TEST_NAME}_fc ${SOURCE_FILE})
        target_compile_definitions(${TEST_NAME}_fc PRIVATE FASTCOND_PATCH_COND)
        target_link_libraries(${TEST_NAME}_fc PRIVATE fastcond ${MATH_LIBRARY})
    endfunction()

    # Create test executables
    add_test_variant(qtest test/qtest.c)
    add_test_variant(strongtest test/strongtest.c)

    # Add CTest tests with appropriate arguments
    if(FASTCOND_BUILD_TESTS)
        # Quick smoke tests (small data count for fast CI)
        add_test(NAME qtest_pthread_smoke 
                 COMMAND qtest_pt 100 2 5)
        add_test(NAME qtest_fastcond_smoke 
                 COMMAND qtest_fc 100 2 5)
        add_test(NAME qtest_weak_smoke 
                 COMMAND qtest_wcond 100 2 5)
        
        add_test(NAME strongtest_pthread_smoke 
                 COMMAND strongtest_pt 100 5)
        add_test(NAME strongtest_fastcond_smoke 
                 COMMAND strongtest_fc 100 5)
        
        # NOTE: strongtest_weak is NOT tested because strongtest specifically
        # validates strong semantics (no wakeup stealing). The weak variant
        # allows wakeup stealing and will deadlock in this test scenario.
        # This is expected behavior - weak variant should only be used when
        # all waiting threads are interchangeable.

        # Verify output contains expected patterns
        set_tests_properties(qtest_pthread_smoke PROPERTIES
            PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got")
        set_tests_properties(qtest_fastcond_smoke PROPERTIES
            PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got")
        set_tests_properties(strongtest_pthread_smoke PROPERTIES
            PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got")
        set_tests_properties(strongtest_fastcond_smoke PROPERTIES
            PASS_REGULAR_EXPRESSION "sender.*sent|receiver.*got")
    endif()

    # Benchmark targets (not run by default in ctest)
    # Use scripts/benchmark.sh for running benchmarks
    if(FASTCOND_BUILD_BENCHMARKS)
        add_custom_target(benchmark
            COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "Running benchmarks via scripts/benchmark.sh"
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/benchmark.sh
            DEPENDS qtest_pt qtest_fc strongtest_pt strongtest_fc
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running performance benchmarks..."
        )
    endif()
endif()

# ============================================================================
# Installation
# ============================================================================
include(GNUInstallDirs)

install(TARGETS fastcond
    EXPORT fastcondTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
    fastcond/fastcond.h
    fastcond/fastcond_patch.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fastcond
)

# Export cmake config
install(EXPORT fastcondTargets
    FILE fastcondTargets.cmake
    NAMESPACE fastcond::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fastcond
)

# Create config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/fastcondConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/fastcondConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/fastcondConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fastcond
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/fastcondConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/fastcondConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/fastcond
)

# ============================================================================
# Print configuration summary
# ============================================================================
message(STATUS "")
message(STATUS "fastcond configuration:")
message(STATUS "  Version:           ${PROJECT_VERSION}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared libraries:  ${BUILD_SHARED_LIBS}")
message(STATUS "  Build tests:       ${FASTCOND_BUILD_TESTS}")
message(STATUS "  Build benchmarks:  ${FASTCOND_BUILD_BENCHMARKS}")
message(STATUS "  Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
