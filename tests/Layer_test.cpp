#include "DigitRecognisionNN/Activators.hpp"
#include "DigitRecognisionNN/Layer.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

namespace {
constexpr size_t INPUT_SIZE = 3;
constexpr size_t OUTPUT_SIZE = 2;
constexpr double TOLERANCE = 1e-12;

using VectorO = blaze::DynamicVector<double>;
using Matrix = blaze::DynamicMatrix<double>;

using ReluActivator = Activators::Relu<double>;
using SoftmaxActivator = Activators::Softmax<double>;

struct IdentityActivator {
    VectorO operator()(const VectorO &vec) const { return vec; }
};

struct DoublingActivator {
    VectorO operator()(const VectorO &vec) const { return vec * 2.0; }
};

template <typename ActivatorT> using TestLayer = Layer<double, ActivatorT>;

// 3x2 weights: [[1, 2], [3, 4], [5, 6]]
Matrix makeWeights() {
    Matrix weights(INPUT_SIZE, OUTPUT_SIZE);
    weights = blaze::DynamicMatrix<double>{{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}};
    return weights;
}
} // namespace

TEST(LayerConstructionTest, RandomInitHasCorrectWeightShape) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    EXPECT_EQ(layer.getWeights().rows(), INPUT_SIZE);
    EXPECT_EQ(layer.getWeights().columns(), OUTPUT_SIZE);
}

TEST(LayerConstructionTest, RandomInitBiasIsZero) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    for (size_t i = 0; i < OUTPUT_SIZE; ++i) {
        EXPECT_DOUBLE_EQ(layer.getBias()[i], 0.0);
    }
}

TEST(LayerConstructionTest, RandomInitWeightsAreNotAllZero) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    bool anyNonZero = false;
    for (size_t row = 0; row < INPUT_SIZE; ++row) {
        for (size_t col = 0; col < OUTPUT_SIZE; ++col) {
            anyNonZero = anyNonZero || layer.getWeights()(row, col) != 0.0;
        }
    }
    EXPECT_TRUE(anyNonZero);
}

TEST(LayerConstructionTest, TwoRandomLayersDiffer) {
    TestLayer<ReluActivator> first{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    TestLayer<ReluActivator> second{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    EXPECT_NE(first.getWeights(), second.getWeights());
}

TEST(LayerConstructionTest, ExplicitWeightsAndBiasAreStored) {
    const VectorO bias{0.5, -0.5};
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{},
                                   makeWeights(), bias};
    EXPECT_EQ(layer.getWeights(), makeWeights());
    EXPECT_EQ(layer.getBias(), bias);
}

TEST(LayerConstructionTest, SizeGettersReturnConstructorArguments) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    EXPECT_EQ(layer.getInputSize(), INPUT_SIZE);
    EXPECT_EQ(layer.getOutputSize(), OUTPUT_SIZE);
}

TEST(LayerConstructionTest, MismatchedWeightsOrBiasThrow) {
    EXPECT_THROW(
        (TestLayer<ReluActivator>{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{},
                                  Matrix(2, 2), VectorO(OUTPUT_SIZE)}),
        std::invalid_argument);
    EXPECT_THROW(
        (TestLayer<ReluActivator>{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{},
                                  makeWeights(), VectorO(OUTPUT_SIZE + 1)}),
        std::invalid_argument);
}

TEST(LayerConstructionTest, CachesStartEmpty) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    EXPECT_EQ(layer.getZCache().rows(), 0U);
    EXPECT_EQ(layer.getOutputCache().rows(), 0U);
}

TEST(LayerAccessorsTest, SetWeights) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    layer.setWeights(makeWeights());
    EXPECT_EQ(layer.getWeights(), makeWeights());
}

TEST(LayerAccessorsTest, SetBias) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    const VectorO bias{1.0, 2.0};
    layer.setBias(bias);
    EXPECT_EQ(layer.getBias(), bias);
}

TEST(LayerActivateTest, DelegatesToActivator) {
    TestLayer<DoublingActivator> layer{INPUT_SIZE, OUTPUT_SIZE,
                                       DoublingActivator{}};
    const VectorO input{1.5, -2.0};
    const VectorO output = layer.activate(input);
    EXPECT_DOUBLE_EQ(output[0], 3.0);
    EXPECT_DOUBLE_EQ(output[1], -4.0);
}

TEST(LayerActivateTest, SetActivatorReplacesBehaviour) {
    TestLayer<DoublingActivator> layer{INPUT_SIZE, OUTPUT_SIZE,
                                       DoublingActivator{}};
    layer.setActivator(DoublingActivator{});
    const VectorO input{1.0, 1.0};
    EXPECT_DOUBLE_EQ(layer.activate(input)[0], 2.0);
}

TEST(LayerActivateTest, SetActivatorSwitchesStatefulActivator) {
    struct Scale {
        double factor;
        VectorO operator()(const VectorO &vec) const { return vec * factor; }
    };
    Layer<double, Scale> layer{INPUT_SIZE, OUTPUT_SIZE, Scale{1.0}};
    const VectorO input{2.0, 3.0};
    EXPECT_DOUBLE_EQ(layer.activate(input)[0], 2.0);
    layer.setActivator(Scale{10.0});
    EXPECT_DOUBLE_EQ(layer.activate(input)[0], 20.0);
}

TEST(LayerComputeTest, IdentityActivatorSingleSample) {
    const VectorO bias{0.0, 0.0};
    TestLayer<IdentityActivator> layer{
        INPUT_SIZE, OUTPUT_SIZE, IdentityActivator{}, makeWeights(), bias};
    const Matrix input{{1.0, 1.0, 1.0}};
    const Matrix &output = layer.compute(input);
    ASSERT_EQ(output.rows(), 1U);
    ASSERT_EQ(output.columns(), OUTPUT_SIZE);
    EXPECT_NEAR(output(0, 0), 9.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 12.0, TOLERANCE);
}

TEST(LayerComputeTest, AddsBias) {
    const VectorO bias{1.0, -2.0};
    TestLayer<IdentityActivator> layer{
        INPUT_SIZE, OUTPUT_SIZE, IdentityActivator{}, makeWeights(), bias};
    const Matrix input{{1.0, 1.0, 1.0}};
    const Matrix &output = layer.compute(input);
    EXPECT_NEAR(output(0, 0), 10.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 10.0, TOLERANCE);
}

TEST(LayerComputeTest, BatchProcessesEachRowIndependently) {
    const VectorO bias{1.0, 1.0};
    TestLayer<IdentityActivator> layer{
        INPUT_SIZE, OUTPUT_SIZE, IdentityActivator{}, makeWeights(), bias};
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
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{},
                                   makeWeights(), bias};
    const Matrix input{{-1.0, -1.0, -1.0}, {1.0, 1.0, 1.0}};
    const Matrix &output = layer.compute(input);
    EXPECT_DOUBLE_EQ(output(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(output(0, 1), 0.0);
    EXPECT_NEAR(output(1, 0), 9.0, TOLERANCE);
    EXPECT_NEAR(output(1, 1), 12.0, TOLERANCE);
}

TEST(LayerComputeTest, SoftmaxRowsSumToOne) {
    const VectorO bias{0.1, -0.1};
    TestLayer<SoftmaxActivator> layer{INPUT_SIZE, OUTPUT_SIZE,
                                      SoftmaxActivator{}, makeWeights(), bias};
    const Matrix input{{0.1, 0.2, 0.3}, {-0.5, 0.0, 0.5}};
    const Matrix &output = layer.compute(input);
    for (size_t i = 0; i < output.rows(); ++i) {
        EXPECT_NEAR(output(i, 0) + output(i, 1), 1.0, TOLERANCE);
    }
}

TEST(LayerComputeTest, ZCacheHoldsPreActivationIncludingBias) {
    const VectorO bias{1.0, 1.0};
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{},
                                   makeWeights(), bias};
    const Matrix input{{-1.0, -1.0, -1.0}};
    layer.compute(input);
    const Matrix &zCache = layer.getZCache();
    ASSERT_EQ(zCache.rows(), 1U);
    EXPECT_NEAR(zCache(0, 0), -8.0, TOLERANCE);
    EXPECT_NEAR(zCache(0, 1), -11.0, TOLERANCE);
}

TEST(LayerComputeTest, OutputCacheMatchesReturnValue) {
    TestLayer<ReluActivator> layer{INPUT_SIZE, OUTPUT_SIZE, ReluActivator{}};
    const Matrix input{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    const Matrix &output = layer.compute(input);
    EXPECT_EQ(&output, &layer.getOutputCache());
    EXPECT_EQ(layer.getOutputCache().rows(), 2U);
    EXPECT_EQ(layer.getOutputCache().columns(), OUTPUT_SIZE);
}

TEST(LayerComputeTest, WrongInputWidthThrows) {
    TestLayer<IdentityActivator> layer{INPUT_SIZE, OUTPUT_SIZE,
                                       IdentityActivator{}};
    EXPECT_THROW(layer.compute(Matrix(1, INPUT_SIZE + 1)),
                 std::invalid_argument);
}

TEST(LayerComputeTest, HandlesChangingBatchSize) {
    const VectorO bias{0.0, 0.0};
    TestLayer<IdentityActivator> layer{
        INPUT_SIZE, OUTPUT_SIZE, IdentityActivator{}, makeWeights(), bias};
    layer.compute(Matrix{{1.0, 1.0, 1.0}, {2.0, 2.0, 2.0}});
    EXPECT_EQ(layer.getOutputCache().rows(), 2U);
    layer.compute(Matrix{{1.0, 1.0, 1.0}});
    EXPECT_EQ(layer.getOutputCache().rows(), 1U);
    EXPECT_NEAR(layer.getOutputCache()(0, 0), 9.0, TOLERANCE);
}

TEST(LayerComputeTest, UsesWeightsSetAfterConstruction) {
    TestLayer<IdentityActivator> layer{INPUT_SIZE, OUTPUT_SIZE,
                                       IdentityActivator{}};
    layer.setWeights(makeWeights());
    layer.setBias(VectorO{0.0, 0.0});
    const Matrix &output = layer.compute(Matrix{{1.0, 0.0, 0.0}});
    EXPECT_NEAR(output(0, 0), 1.0, TOLERANCE);
    EXPECT_NEAR(output(0, 1), 2.0, TOLERANCE);
}
