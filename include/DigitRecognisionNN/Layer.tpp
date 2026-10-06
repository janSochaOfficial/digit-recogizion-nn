#include "DigitRecognisionNN/Layer.hpp"

#define LAYER_TEMPLATE                                                         \
    template <typename T, typename ActivatorT, size_t input_size,              \
              size_t output_size>
#define LAYER Layer<T, ActivatorT, input_size, output_size>

LAYER_TEMPLATE
void LAYER::generateWeights() {
    const T std_dev = std::sqrt(T{2} / static_cast<T>(input_size));
    std::normal_distribution<T> distribution(T{0}, std_dev);
    weights_ = Matrix(input_size, output_size);
    for (size_t row = 0; row < input_size; row++) {
        for (size_t col = 0; col < output_size; col++) {
            weights_.at(row, col) = distribution(Generator);
        }
    }
}

LAYER_TEMPLATE void LAYER::generateBias() { bias_ = VectorO(T{0}); }

LAYER_TEMPLATE
LAYER::Layer(ActivatorT activator) : activator_(activator) {
    generateWeights();
    generateBias();
}

LAYER_TEMPLATE
LAYER::Layer(ActivatorT activator, Matrix weights, VectorO bias)
    : activator_(activator), weights_(weights), bias_(bias) {}

LAYER_TEMPLATE
void LAYER::setActivator(ActivatorT activator) { activator_ = activator; }

LAYER_TEMPLATE
typename LAYER::VectorO LAYER::activate(const VectorO &values) {
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
const typename LAYER::VectorO &LAYER::getBias() const { return bias_; }

LAYER_TEMPLATE
void LAYER::setWeights(Matrix weights) { weights_ = std::move(weights); }

LAYER_TEMPLATE
void LAYER::setBias(VectorO bias) { bias_ = std::move(bias); }

LAYER_TEMPLATE
const typename LAYER::Matrix &LAYER::compute(const LAYER::Matrix &input) {
    z_cache_ = input * weights_;

    output_cache_.resize(z_cache_.rows(), output_size);
    for (size_t i = 0; i < z_cache_.rows(); ++i) {
        blaze::row(z_cache_, i) += blaze::trans(bias_);

        const VectorO z_row = blaze::trans(blaze::row(z_cache_, i));
        blaze::row(output_cache_, i) = blaze::trans(activator_(z_row));
    }
    return output_cache_;
}

#undef LAYER
#undef LAYER_TEMPLATE
