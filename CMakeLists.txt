cmake_minimum_required(VERSION 3.10)
project(uprintf VERSION 1.0.0 LANGUAGES C)

option(UPRINTF_HEADER_ONLY "Use header-only mode" ON)
option(UPRINTF_UNICODE "Force wide (wchar_t) mode" OFF)
option(UPRINTF_BUILD_TESTS "Build tests" ON)
option(UPRINTF_BUILD_EXAMPLES "Build examples" ON)

# Header-only interface library
add_library(uprintf INTERFACE)
target_include_directories(uprintf INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(UPRINTF_UNICODE)
    target_compile_definitions(uprintf INTERFACE UPRINTF_UNICODE)
endif()

target_compile_definitions(uprintf INTERFACE UPRINTF_HEADER_ONLY)

# Compiled library (optional)
if(NOT UPRINTF_HEADER_ONLY)
    add_library(uprintf_compiled STATIC src/uprintf.c)
    target_include_directories(uprintf_compiled PUBLIC include)
    if(UPRINTF_UNICODE)
        target_compile_definitions(uprintf_compiled PUBLIC UPRINTF_UNICODE)
    endif()
endif()

# Tests
if(UPRINTF_BUILD_TESTS)
    enable_testing()

    foreach(test_name test_narrow test_wide test_snprintf test_security)
        add_executable(${test_name} tests/${test_name}.c)
        target_link_libraries(${test_name} PRIVATE uprintf)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endforeach()
endif()

# Examples
if(UPRINTF_BUILD_EXAMPLES)
    add_executable(basic examples/basic.c)
    target_link_libraries(basic PRIVATE uprintf)
endif()

# Install
include(GNUInstallDirs)

install(FILES
    include/uprintf.h
    include/uprintf_config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS uprintf EXPORT uprintfTargets)

install(EXPORT uprintfTargets
    FILE uprintfConfig.cmake
    NAMESPACE uprintf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/uprintf
)
