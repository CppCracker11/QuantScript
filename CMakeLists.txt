cmake_minimum_required(VERSION 3.16)

project(QuantScript VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM CONFIG REQUIRED)

if(LLVM_PACKAGE_VERSION VERSION_LESS "17.0.0")
    message(FATAL_ERROR
        "QuantScript requires LLVM 17 or newer. "
        "Found LLVM ${LLVM_PACKAGE_VERSION}."
    )
endif()

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM_DIR: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

file(GLOB_RECURSE QS_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(quantscript ${QS_SOURCES})

target_include_directories(quantscript PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(quantscript PRIVATE
    LLVM
)

if(UNIX)
    target_link_libraries(quantscript PRIVATE
        dl
        pthread
    )
endif()

enable_testing()

file(GLOB VALID_TESTS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/tests/valid/*.qs
)

foreach(test_file ${VALID_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)

    add_test(
        NAME QuantScriptValid_${test_name}
        COMMAND quantscript ${test_file}
    )
endforeach()

file(GLOB INVALID_TESTS
    CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/tests/invalid/*.qs
)

foreach(test_file ${INVALID_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)

    add_test(
        NAME QuantScriptInvalid_${test_name}
        COMMAND quantscript ${test_file}
    )

    set_tests_properties(
        QuantScriptInvalid_${test_name}
        PROPERTIES WILL_FAIL TRUE
    )
endforeach()