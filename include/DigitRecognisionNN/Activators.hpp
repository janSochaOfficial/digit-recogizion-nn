#ifndef ACTIVATORS_HPP
#define ACTIVATORS_HPP

#include <blaze/Math.h>

namespace Activators {
template <typename T> struct Relu {
    using Vector = blaze::DynamicVector<T>;

    Vector operator()(const Vector &input) const {
        Vector output = blaze::max(input, T{0});
        return output;
    }
};

template <typename T> struct Softmax {
    using Vector = blaze::DynamicVector<T>;

    Vector operator()(const Vector &input) const {
        const T max = blaze::max(input);
        Vector output = blaze::exp(input - max);
        const T sum = blaze::sum(output);
        output *= 1 / sum;
        return output;
    }
};

} // namespace Activators

#endif
