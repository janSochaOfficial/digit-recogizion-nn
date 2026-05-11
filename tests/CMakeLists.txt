add_executable (unit_tests
    ExampleClass_test.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        GTest::gtest_main   # includes main() for you
        CppTemplate            # link against your actual library target
)


target_include_directories(unit_tests
    PRIVATE ${PROJECT_SOURCE_DIR}/include
)

# Register with CTest
include(GoogleTest)
gtest_discover_tests(unit_tests)