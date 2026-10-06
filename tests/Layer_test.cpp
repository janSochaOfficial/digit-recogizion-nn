#include "DigitRecognisionNN/Activators.hpp"
#include "DigitRecognisionNN/Layer.hpp"
#include <gtest/gtest.h>

namespace {
constexpr size_t INPUT_SIZE = 3;
constexpr size_t OUTPUT_SIZE = 2;
constexpr double TOLERANCE = 1e-12;

using VectorO = blaze::StaticVector<double, OUTPUT_SIZE>;
using Matrix = blaze::DynamicMatrix<double>;

struct ReluActivator {
    VectorO operator()(const VectorO &v) const {
        return Activators::relu<double, OUTPUT_SIZE>(v);
    }
};

struct SoftmaxActivator {
    VectorO operator()(const VectorO &v) const {
        return Activators::softmax<double, OUTPUT_SIZE>(v);
    }
};

struct IdentityActivator {
    VectorO operator()(const VectorO &v) const { return v; }
};

struct DoublingActivator {
    VectorO operator()(const VectorO &v) const { return v * 2.0; }
};

template <typename ActivatorT>
using TestLayer = Layer<double, ActivatorT, INPUT_SIZE, OUTPUT_SIZE>;

// 3x2 weights: [[1, 2], [3, 4], [5, 6]]
Matrix makeWeights() {
    Matrix w(INPUT_SIZE, OUTPUT_SIZE);
    w = blaze::DynamicMatrix<double>{{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
    return w;
}
} // namespace

TEST(LayerConstructionTest, RandomInitHasCorrectWeightShape) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    EXPECT_EQ(layer.getWeights().rows(), INPUT_SIZE);
    EXPECT_EQ(layer.getWeights().columns(), OUTPUT_SIZE);
}

TEST(LayerConstructionTest, RandomInitBiasIsZero) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    for (size_t i = 0; i < OUTPUT_SIZE; ++i) {
        EXPECT_DOUBLE_EQ(layer.getBias()[i], 0.0);
    }
}

TEST(LayerConstructionTest, RandomInitWeightsAreNotAllZero) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    bool anyNonZero = false;
    for (size_t r = 0; r < INPUT_SIZE; ++r) {
        for (size_t c = 0; c < OUTPUT_SIZE; ++c) {
            anyNonZero = anyNonZero || layer.getWeights()(r, c) != 0.0;
        }
    }
    EXPECT_TRUE(anyNonZero);
}

TEST(LayerConstructionTest, TwoRandomLayersDiffer) {
    TestLayer<ReluActivator> a{ReluActivator{}};
    TestLayer<ReluActivator> b{ReluActivator{}};
    EXPECT_NE(a.getWeights(), b.getWeights());
}

TEST(LayerConstructionTest, ExplicitWeightsAndBiasAreStored) {
    const VectorO bias{0.5, -0.5};
    TestLayer<ReluActivator> layer{ReluActivator{}, makeWeights(), bias};
    EXPECT_EQ(layer.getWeights(), makeWeights());
    EXPECT_EQ(layer.getBias(), bias);
}

TEST(LayerConstructionTest, CachesStartEmpty) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    EXPECT_EQ(layer.getZCache().rows(), 0U);
    EXPECT_EQ(layer.getOutputCache().rows(), 0U);
}

TEST(LayerAccessorsTest, SetWeights) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    layer.setWeights(makeWeights());
    EXPECT_EQ(layer.getWeights(), makeWeights());
}

TEST(LayerAccessorsTest, SetBias) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    const VectorO bias{1.0, 2.0};
    layer.setBias(bias);
    EXPECT_EQ(layer.getBias(), bias);
}

TEST(LayerActivateTest, DelegatesToActivator) {
    TestLayer<DoublingActivator> layer{DoublingActivator{}};
    const VectorO input{1.5, -2.0};
    const VectorO output = layer.activate(input);
    EXPECT_DOUBLE_EQ(output[0], 3.0);
    EXPECT_DOUBLE_EQ(output[1], -4.0);
}

TEST(LayerActivateTest, SetActivatorReplacesBehaviour) {
    TestLayer<DoublingActivator> layer{DoublingActivator{}};
    layer.setActivator(DoublingActivator{});
    const VectorO input{1.0, 1.0};
    EXPECT_DOUBLE_EQ(layer.activate(input)[0], 2.0);
}

TEST(LayerActivateTest, SetActivatorSwitchesStatefulActivator) {
    struct Scale {
        double factor;
        VectorO operator()(const VectorO &v) const { return v * factor; }
    };
    Layer<double, Scale, INPUT_SIZE, OUTPUT_SIZE> layer{Scale{1.0}};
    const VectorO input{2.0, 3.0};
    EXPECT_DOUBLE_EQ(layer.activate(input)[0], 2.0);
    layer.setActivator(Scale{10.0});
    EXPECT_DOUBLE_EQ(layer.activate(input)[0], 20.0);
}

TEST(LayerComputeTest, IdentityActivatorSingleSample) {
    const VectorO bias{0.0, 0.0};
    TestLayer<IdentityActivator> layer{IdentityActivator{}, makeWeights(),
                                       bias};
    const Matrix input{{1.0, 1.0, 1.0}};
    const Matrix &output = layer.compute(input);
    ASSERT_EQ(output.rows(), 1U);
    ASSERT_EQ(output.columns(), OUTPUT_SIZE);
    EXPECT_NEAR(output(0, 0), 9.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 12.0, TOLERANCE);
}

TEST(LayerComputeTest, AddsBias) {
    const VectorO bias{1.0, -2.0};
    TestLayer<IdentityActivator> layer{IdentityActivator{}, makeWeights(),
                                       bias};
    const Matrix input{{1.0, 1.0, 1.0}};
    const Matrix &output = layer.compute(input);
    EXPECT_NEAR(output(0, 0), 10.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 10.0, TOLERANCE);
}

TEST(LayerComputeTest, BatchProcessesEachRowIndependently) {
    const VectorO bias{1.0, 1.0};
    TestLayer<IdentityActivator> layer{IdentityActivator{}, makeWeights(),
                                       bias};
    const Matrix input{{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};
    const Matrix &output = layer.compute(input);
    ASSERT_EQ(output.rows(), 3U);
    EXPECT_NEAR(output(0, 0), 2.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 3.0, TOLERANCE);
    EXPECT_NEAR(output(1, 0), 4.0, TOLERANCE);
    EXPECT_NEAR(output(1, 1), 5.0, TOLERANCE);
    EXPECT_NEAR(output(2, 0), 6.0, TOLERANCE);
    EXPECT_NEAR(output(2, 1), 7.0, TOLERANCE);
}

TEST(LayerComputeTest, ReluClampsNegativeOutputs) {
    const VectorO bias{0.0, 0.0};
    TestLayer<ReluActivator> layer{ReluActivator{}, makeWeights(), bias};
    const Matrix input{{-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0}};
    const Matrix &output = layer.compute(input);
    EXPECT_DOUBLE_EQ(output(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(output(0, 1), 0.0);
    EXPECT_NEAR(output(1, 0), 9.0, TOLERANCE);
    EXPECT_NEAR(output(1, 1), 12.0, TOLERANCE);
}

TEST(LayerComputeTest, SoftmaxRowsSumToOne) {
    const VectorO bias{0.1, -0.1};
    TestLayer<SoftmaxActivator> layer{SoftmaxActivator{}, makeWeights(), bias};
    const Matrix input{{0.1, 0.2, 0.3}, {-0.5, 0.0, 0.5}};
    const Matrix &output = layer.compute(input);
    for (size_t i = 0; i < output.rows(); ++i) {
        EXPECT_NEAR(output(i, 0) + output(i, 1), 1.0, TOLERANCE);
    }
}

TEST(LayerComputeTest, ZCacheHoldsPreActivationIncludingBias) {
    const VectorO bias{1.0, 1.0};
    TestLayer<ReluActivator> layer{ReluActivator{}, makeWeights(), bias};
    const Matrix input{{-1.0, -1.0, -1.0}};
    layer.compute(input);
    const Matrix &z = layer.getZCache();
    ASSERT_EQ(z.rows(), 1U);
    EXPECT_NEAR(z(0, 0), -8.0, TOLERANCE);
    EXPECT_NEAR(z(0, 1), -11.0, TOLERANCE);
}

TEST(LayerComputeTest, OutputCacheMatchesReturnValue) {
    TestLayer<ReluActivator> layer{ReluActivator{}};
    const Matrix input{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    const Matrix &output = layer.compute(input);
    EXPECT_EQ(&output, &layer.getOutputCache());
    EXPECT_EQ(layer.getOutputCache().rows(), 2U);
    EXPECT_EQ(layer.getOutputCache().columns(), OUTPUT_SIZE);
}

TEST(LayerComputeTest, HandlesChangingBatchSize) {
    const VectorO bias{0.0, 0.0};
    TestLayer<IdentityActivator> layer{IdentityActivator{}, makeWeights(),
                                       bias};
    layer.compute(Matrix{{1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}});
    EXPECT_EQ(layer.getOutputCache().rows(), 2U);
    layer.compute(Matrix{{1.0, 1.0, 1.0}});
    EXPECT_EQ(layer.getOutputCache().rows(), 1U);
    EXPECT_NEAR(layer.getOutputCache()(0, 0), 9.0, TOLERANCE);
}

TEST(LayerComputeTest, UsesWeightsSetAfterConstruction) {
    TestLayer<IdentityActivator> layer{IdentityActivator{}};
    layer.setWeights(makeWeights());
    layer.setBias(VectorO{0.0, 0.0});
    const Matrix &output = layer.compute(Matrix{{1.0, 0.0, 0.0}});
    EXPECT_NEAR(output(0, 0), 1.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 2.0, TOLERANCE);
}
