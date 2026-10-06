#ifndef ACTIVATORS_HPP
#define ACTIVATORS_HPP

#include <blaze/Math.h>

#define VECTOR blaze::StaticVector<T, size>

namespace Activators {
template <typename T, size_t size> VECTOR relu(const VECTOR &input) {
    VECTOR output = blaze::max(input, T{0});
    return output;
}

template <typename T, size_t size> VECTOR softmax(const VECTOR &input) {
    const T max = blaze::max(input);
    VECTOR output = blaze::exp(input - blaze::uniform(size, max));
    const T sum = blaze::sum(output);
    output *= 1 / sum;
    return output;
}

}; // namespace Activators

#undef VECTOR

#endif