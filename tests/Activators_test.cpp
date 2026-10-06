#include "DigitRecognisionNN/Activators.hpp"
#include <cmath>
#include <gtest/gtest.h>

namespace {
constexpr double TOLERANCE = 1e-12;
using Vec3 = blaze::StaticVector<double, 3>;
} // namespace

TEST(ActivatorsReluTest, PositiveValuesUnchanged) {
    const Vec3 input{1.0, 2.5, 100.0};
    const Vec3 output = Activators::relu<double, 3>(input);
    EXPECT_EQ(output, input);
}

TEST(ActivatorsReluTest, NegativeValuesBecomeZero) {
    const Vec3 input{-1.0, -0.5, -100.0};
    const Vec3 output = Activators::relu<double, 3>(input);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_DOUBLE_EQ(output[i], 0.0);
    }
}

TEST(ActivatorsReluTest, MixedValues) {
    const Vec3 input{-2.0, 0.0, 3.0};
    const Vec3 output = Activators::relu<double, 3>(input);
    EXPECT_DOUBLE_EQ(output[0], 0.0);
    EXPECT_DOUBLE_EQ(output[1], 0.0);
    EXPECT_DOUBLE_EQ(output[2], 3.0);
}

TEST(ActivatorsReluTest, DoesNotModifyInput) {
    const Vec3 input{-1.0, 2.0, -3.0};
    const Vec3 copy = input;
    Activators::relu<double, 3>(input);
    EXPECT_EQ(input, copy);
}

TEST(ActivatorsReluTest, WorksWithFloat) {
    const blaze::StaticVector<float, 2> input{-1.0F, 4.0F};
    const auto output = Activators::relu<float, 2>(input);
    EXPECT_FLOAT_EQ(output[0], 0.0F);
    EXPECT_FLOAT_EQ(output[1], 4.0F);
}

TEST(ActivatorsSoftmaxTest, SumsToOne) {
    const Vec3 input{1.0, 2.0, 3.0};
    const Vec3 output = Activators::softmax<double, 3>(input);
    EXPECT_NEAR(blaze::sum(output), 1.0, TOLERANCE);
}

TEST(ActivatorsSoftmaxTest, MatchesReferenceValues) {
    const Vec3 input{1.0, 2.0, 3.0};
    const Vec3 output = Activators::softmax<double, 3>(input);
    const double denom = std::exp(1.0) + std::exp(2.0) + std::exp(3.0);
    EXPECT_NEAR(output[0], std::exp(1.0) / denom, TOLERANCE);
    EXPECT_NEAR(output[1], std::exp(2.0) / denom, TOLERANCE);
    EXPECT_NEAR(output[2], std::exp(3.0) / denom, TOLERANCE);
}

TEST(ActivatorsSoftmaxTest, UniformInputGivesUniformOutput) {
    const Vec3 input{5.0, 5.0, 5.0};
    const Vec3 output = Activators::softmax<double, 3>(input);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_NEAR(output[i], 1.0 / 3.0, TOLERANCE);
    }
}

TEST(ActivatorsSoftmaxTest, AllOutputsPositive) {
    const Vec3 input{-10.0, 0.0, 10.0};
    const Vec3 output = Activators::softmax<double, 3>(input);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_GT(output[i], 0.0);
    }
}

TEST(ActivatorsSoftmaxTest, PreservesOrdering) {
    const Vec3 input{0.1, 0.9, 0.5};
    const Vec3 output = Activators::softmax<double, 3>(input);
    EXPECT_GT(output[1], output[2]);
    EXPECT_GT(output[2], output[0]);
}

TEST(ActivatorsSoftmaxTest, ShiftInvariant) {
    const Vec3 input{1.0, 2.0, 3.0};
    const Vec3 shifted{101.0, 102.0, 103.0};
    const Vec3 a = Activators::softmax<double, 3>(input);
    const Vec3 b = Activators::softmax<double, 3>(shifted);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_NEAR(a[i], b[i], TOLERANCE);
    }
}

TEST(ActivatorsSoftmaxTest, NumericallyStableForLargeInputs) {
    const Vec3 input{1000.0, 1001.0, 1002.0};
    const Vec3 output = Activators::softmax<double, 3>(input);
    for (size_t i = 0; i < 3; ++i) {
        EXPECT_TRUE(std::isfinite(output[i]));
    }
    EXPECT_NEAR(blaze::sum(output), 1.0, TOLERANCE);
}

TEST(ActivatorsSoftmaxTest, SingleElementIsOne) {
    const blaze::StaticVector<double, 1> input{42.0};
    const auto output = Activators::softmax<double, 1>(input);
    EXPECT_NEAR(output[0], 1.0, TOLERANCE);
}
