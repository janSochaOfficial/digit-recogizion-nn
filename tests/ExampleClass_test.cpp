#include <gtest/gtest.h>
#include <sstream>
#include "CppTemplate/ExampleClass.hpp"

TEST(ExampleClassTest, PrintWritesHelloWorlds) {
    ExampleClass obj;
    std::ostringstream oss;

    ExampleClass::print(oss);

    EXPECT_EQ(oss.str(), "Hello Worlds");
}