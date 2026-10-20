#include "DigitRecognisionNN/Layer.hpp"

#include <stdexcept>

#define LAYER_TEMPLATE template <typename T, typename ActivatorT>
#define LAYER Layer<T, ActivatorT>

LAYER_TEMPLATE
void LAYER::generateWeights() {
    const T std_dev = std::sqrt(T{2} / static_cast<T>(input_size_));
    std::normal_distribution<T> distribution(T{0}, std_dev);
    weights_ = Matrix(input_size_, output_size_);
    for (size_t row = 0; row < input_size_; row++) {
        for (size_t col = 0; col < output_size_; col++) {
            weights_.at(row, col) = distribution(generator());
        }
    }
}

LAYER_TEMPLATE void LAYER::generateBias() {
    bias_ = Vector(output_size_, T{0});
}

LAYER_TEMPLATE
LAYER::Layer(size_t input_size, size_t output_size, ActivatorT activator)
    : input_size_(input_size), output_size_(output_size),
      activator_(std::move(activator)) {
    generateWeights();
    generateBias();
}

LAYER_TEMPLATE
LAYER::Layer(size_t input_size, size_t output_size, ActivatorT activator,
             Matrix weights, Vector bias)
    : input_size_(input_size), output_size_(output_size),
      activator_(std::move(activator)) {
    setWeights(std::move(weights));
    setBias(std::move(bias));
}

LAYER_TEMPLATE
size_t LAYER::getInputSize() const { return input_size_; }

LAYER_TEMPLATE
size_t LAYER::getOutputSize() const { return output_size_; }

LAYER_TEMPLATE
void LAYER::setActivator(ActivatorT activator) {
    activator_ = std::move(activator);
}

LAYER_TEMPLATE
typename LAYER::Vector LAYER::activate(const Vector &values) {
    return activator_(values);
}

LAYER_TEMPLATE
const typename LAYER::Matrix &LAYER::getOutputCache() const {
    return output_cache_;
}

LAYER_TEMPLATE
const typename LAYER::Matrix &LAYER::getZCache() const { return z_cache_; }

LAYER_TEMPLATE
const typename LAYER::Matrix &LAYER::getWeights() const { return weights_; }

LAYER_TEMPLATE
const typename LAYER::Vector &LAYER::getBias() const { return bias_; }

LAYER_TEMPLATE
void LAYER::setWeights(Matrix weights) {
    if (weights.rows() != input_size_ || weights.columns() != output_size_) {
        throw std::invalid_argument(
            "Layer weights must have shape input_size x output_size");
    }
    weights_ = std::move(weights);
}

LAYER_TEMPLATE
void LAYER::setBias(Vector bias) {
    if (bias.size() != output_size_) {
        throw std::invalid_argument("Layer bias must have output_size entries");
    }
    bias_ = std::move(bias);
}

LAYER_TEMPLATE
const typename LAYER::Matrix &LAYER::compute(const LAYER::Matrix &input) {
    if (input.columns() != input_size_) {
        throw std::invalid_argument("Layer input must have input_size columns");
    }
    z_cache_ = input * weights_;

    output_cache_.resize(z_cache_.rows(), output_size_);
    for (size_t i = 0; i < z_cache_.rows(); ++i) {
        blaze::row(z_cache_, i) += blaze::trans(bias_);

        const Vector z_row = blaze::trans(blaze::row(z_cache_, i));
        blaze::row(output_cache_, i) = blaze::trans(activator_(z_row));
    }
    return output_cache_;
}

#undef LAYER
#undef LAYER_TEMPLATE
