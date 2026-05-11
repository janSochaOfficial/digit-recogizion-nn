cmake_minimum_required(VERSION 3.16)

project(CppTemplate CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(BUILD_TESTS "Build unit tests" ON)


if(BUILD_TESTS)
    include(FetchContent)
 
    FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.15.2  # pin to a release
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
endif()

# ─── Warning & hardening flags ────────────────────────────────────────────────
if(MSVC)
    set(COMMON_FLAGS
        /W4             # High warning level
        /WX             # Treat warnings as errors
        /permissive-    # Strict standard conformance
        /w14242         # Narrowing conversions
        /w14254         # Operator precedence
        /w14263         # Member function hides virtual
        /w14265         # Class with virtual but non-virtual destructor
        /w14287         # Unsigned/negative constant mismatch
        /we4289         # Loop control variable used outside loop
        /w14296         # Expression always true/false
        /w14311         # Pointer truncation
        /w14545         # Expression before comma
        /w14546         # Function call before comma
        /w14547         # Operator before comma has no effect
        /w14549         # Operator before comma has no effect
        /w14555         # Expression has no effect
        /w14619         # Pragma warning: invalid warning number
        /w14640         # Thread-unsafe static initialization
        /w14826         # Conversion is sign-extended
        /w14905         # Wide string cast to LPSTR
        /w14906         # String cast to LPWSTR
        /w14928         # Illegal copy-initialization
        /guard:cf       # Control Flow Guard
        /sdl            # Security Development Lifecycle checks
    )
    set(COMMON_LINK_FLAGS "")
else()
    set(COMMON_FLAGS
        # ── Warnings ──────────────────────────────────────────────────────────
        -Wall                       # Standard set of warnings
        -Wextra                     # Extra warnings beyond -Wall
        -Wpedantic                  # Strict ISO compliance
        -Werror                     # Treat warnings as errors
        -Wshadow                    # Variable shadowing
        -Wnon-virtual-dtor          # Class with virtual but non-virtual destructor
        -Wold-style-cast            # C-style casts
        -Wcast-align                # Potentially dangerous pointer casts
        -Wunused                    # Anything unused
        -Woverloaded-virtual        # Overloaded (not overridden) virtual function
        -Wconversion                # Implicit type conversions
        -Wsign-conversion           # Signed/unsigned conversion
        -Wnull-dereference          # Possible null dereference
        -Wdouble-promotion          # Float implicitly promoted to double
        -Wformat=2                  # printf/scanf format string vulnerabilities
        -Wformat-security           # Format string security issues
        -Wimplicit-fallthrough      # Missing break in switch
        -Wmisleading-indentation    # Misleading indentation (dangling else etc.)

        # ── Hardening (compiler flags only) ───────────────────────────────────
        -fstack-protector-strong    # Stack buffer overflow protection
        -D_GLIBCXX_ASSERTIONS       # STL bounds and iterator checks
        -fPIE                       # Position-independent executable (compiler flag)
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND COMMON_FLAGS
            -Wduplicated-cond           # Duplicated condition in if/else chain
            -Wduplicated-branches       # Identical if/else branches
            -Wlogical-op                # Suspicious logical operator use
            -Wuseless-cast              # Casting to the same type
        )
    endif()

    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        list(APPEND COMMON_FLAGS -D_FORTIFY_SOURCE=3)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        list(APPEND COMMON_FLAGS -fstack-clash-protection)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|i[3-6]86")
            list(APPEND COMMON_FLAGS -fcf-protection=full)
        endif()
    endif()

    set(COMMON_LINK_FLAGS
        -Wl,-z,relro                # Read-only relocations after linking
        -Wl,-z,now                  # Resolve all symbols at startup (no lazy binding)
        -Wl,-z,noexecstack          # Non-executable stack
        -pie                        # Required alongside -fPIE for ASLR
    )
endif()

# ─── Sanitizers (Debug only) ──────────────────────────────────────────────────
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT MSVC)
    set(SANITIZER_FLAGS
        -fsanitize=address          # Detect memory errors (heap/stack/global)
        -fsanitize=undefined        # Detect undefined behaviour
        -fsanitize=leak             # Detect memory leaks
        -fno-omit-frame-pointer     # Better stack traces with sanitizers
    )
endif()

# ─── Source files ──────────────────────────────────────────────────────────────
file(GLOB_RECURSE ALL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/include/*.h"
    "${CMAKE_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_SOURCE_DIR}/tests/*.cpp"
)

# ─── clang-format ─────────────────────────────────────────────────────────────
find_program(CLANG_FORMAT_BIN clang-format)

if(CLANG_FORMAT_BIN)
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_BIN}
            -i                          # Edit files in-place
            --style=file                # Use .clang-format config file
            --fallback-style=LLVM       # Fallback if no .clang-format found
            --verbose                   # Print which files are being formatted
            ${ALL_SOURCES}
        COMMENT "Running clang-format on src/ and include/..."
    )
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_BIN}
            --dry-run                   # Don't modify files
            --Werror                    # Exit with error if changes would be made
            --style=file
            --fallback-style=LLVM
            ${ALL_SOURCES}
        COMMENT "Checking clang-format compliance in src/ and include/..."
    )
else()
    message(WARNING "clang-format not found — format targets will not be available")
endif()


# ─── clang-tidy ───────────────────────────────────────────────────────────────
find_program(CLANG_TIDY_BIN clang-tidy)

if(CLANG_TIDY_BIN)
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        set(CMAKE_CXX_CLANG_TIDY
            ${CLANG_TIDY_BIN}
            --extra-arg=-std=c++20
            --extra-arg=-Wno-unknown-warning-option   # silences GCC-only flags
            --warnings-as-errors=*
            "--header-filter=${CMAKE_SOURCE_DIR}/\\(src\\|include\\|tests\\)/.*"
        )
    endif()

    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_BIN}
            -p ${CMAKE_BINARY_DIR}
            --extra-arg=-std=c++20
            --extra-arg=-Wno-unknown-warning-option   # silences GCC-only flags
            --warnings-as-errors=*
            "--header-filter=${CMAKE_SOURCE_DIR}/\\(src\\|include\\)|tests\\/.*"
            ${ALL_SOURCES}
        COMMENT "Running clang-tidy on src/, include/ and tests/..."
        DEPENDS ${ALL_SOURCES}
    )

    add_custom_target(tidy-fix
        COMMAND ${CLANG_TIDY_BIN}
            -p ${CMAKE_BINARY_DIR}
            --extra-arg=-std=c++20
            --extra-arg=-Wno-unknown-warning-option   # silences GCC-only flags
            --fix
            --fix-errors
            "--header-filter=${CMAKE_SOURCE_DIR}/\\(src\\|include\\|tests\\)/.*"
            ${ALL_SOURCES}
        COMMENT "Running clang-tidy with auto-fix on src/, include/ and tests/..."
        DEPENDS ${ALL_SOURCES}
    )
else()
    message(WARNING "clang-tidy not found — tidy targets will not be available")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
endif()

macro(link_to_bin target_name)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E create_symlink
            $<TARGET_FILE:${target_name}> 
            "${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:${target_name}>"
        )
    endif()
endmacro()


# ── Library ────────────────────────────────────────────────────────────────────
add_library(CppTemplate STATIC
    "${CMAKE_SOURCE_DIR}/src/ExampleClass.cpp"
)

target_include_directories(CppTemplate PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)


target_compile_options(CppTemplate PRIVATE ${COMMON_FLAGS})
target_link_options(CppTemplate PUBLIC ${COMMON_LINK_FLAGS})

if(DEFINED SANITIZER_FLAGS AND SANITIZER_FLAGS)
    target_compile_options(CppTemplate PRIVATE ${SANITIZER_FLAGS})
    target_link_options(CppTemplate PUBLIC ${SANITIZER_FLAGS})
endif()


file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*_exec.cpp")

foreach(SOURCE ${SOURCES})
    get_filename_component(TARGET_NAME ${SOURCE} NAME_WE)
    add_executable(${TARGET_NAME} ${SOURCE})
    target_include_directories(${TARGET_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")
    target_compile_options(${TARGET_NAME} PRIVATE ${COMMON_FLAGS})
    target_link_options(${TARGET_NAME} PRIVATE ${COMMON_LINK_FLAGS})
    link_to_bin(${TARGET_NAME})
    if(DEFINED SANITIZER_FLAGS AND SANITIZER_FLAGS)
        target_compile_options(${TARGET_NAME} PRIVATE ${SANITIZER_FLAGS})
        target_link_options(${TARGET_NAME} PRIVATE ${SANITIZER_FLAGS})
    endif()

endforeach()


if (BUILD_TESTS)
    add_subdirectory(tests)
endif()