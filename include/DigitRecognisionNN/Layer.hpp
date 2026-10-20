#ifndef LAYER_HPP
#define LAYER_HPP

#include <blaze/Math.h>
#include <cmath>
#include <functional>
#include <random>
#include <type_traits>

template <typename T, typename ActivatorT> class Layer {
    using Matrix = blaze::DynamicMatrix<T>;
    using Vector = blaze::DynamicVector<T>;
    static_assert(std::is_invocable_r_v<Vector, ActivatorT, Vector>,
                  "ActivatorT must satisfy Vector -> Vector");

    static std::mt19937 &generator() {
        static std::mt19937 engine{std::random_device{}()};
        return engine;
    }

    // NOLINTBEGIN(cppcoreguidelines-avoid-const-or-ref-data-members)
    const size_t input_size_;
    const size_t output_size_;
    // NOLINTEND(cppcoreguidelines-avoid-const-or-ref-data-members)

    ActivatorT activator_;
    Matrix z_cache_;      // pre activation
    Matrix output_cache_; // from inputs
    Matrix weights_;
    Vector bias_;

    void generateWeights();
    void generateBias();

  public:
    Layer(size_t input_size, size_t output_size, ActivatorT activator);
    Layer(size_t input_size, size_t output_size, ActivatorT activator,
          Matrix weights, Vector bias);

    [[nodiscard]] size_t getInputSize() const;
    [[nodiscard]] size_t getOutputSize() const;

    void setActivator(ActivatorT activator);
    Vector activate(const Vector &values);

    [[nodiscard]] const Matrix &getOutputCache() const;
    [[nodiscard]] const Matrix &getZCache() const;

    [[nodiscard]] const Matrix &getWeights() const;
    [[nodiscard]] const Vector &getBias() const;

    void setWeights(Matrix weights);
    void setBias(Vector bias);

    const Matrix &compute(const Matrix &input);
};

#include "Layer.tpp"

#endif
