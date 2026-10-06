#ifndef LAYER_HPP
#define LAYER_HPP

#include <blaze/Math.h>
#include <cmath>
#include <functional>
#include <random>
#include <type_traits>

template <typename T, typename ActivatorT, size_t input_size,
          size_t output_size>
class Layer {
    using Matrix = blaze::DynamicMatrix<T>;
    using VectorO = blaze::StaticVector<T, output_size>;
    static_assert(std::is_invocable_r_v<VectorO, ActivatorT, VectorO>,
                  "ActivatorT must satisfy VectorO -> VectorO");

  protected:
    static inline std::mt19937 Generator{std::random_device{}()};

    ActivatorT activator_;
    Matrix z_cache_;      // pre activation
    Matrix output_cache_; // from inputs
    Matrix weights_;
    VectorO bias_;

    void generateWeights();
    void generateBias();

  public:
    Layer(ActivatorT activator);
    Layer(ActivatorT activator, Matrix weights, VectorO bias);

    void setActivator(ActivatorT activator);
    VectorO activate(const VectorO &values);

    [[nodiscard]] const Matrix &getOutputCache() const;
    [[nodiscard]] const Matrix &getZCache() const;

    [[nodiscard]] const Matrix &getWeights() const;
    [[nodiscard]] const VectorO &getBias() const;

    void setWeights(Matrix weights);
    void setBias(VectorO bias);

    const Matrix &compute(const Matrix &input);
};

#include "Layer.tpp"

#endif