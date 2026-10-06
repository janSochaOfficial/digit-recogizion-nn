#include "DigitRecognisionNN/Dataset.hpp"
#include "DigitRecognisionNN/Layer.hpp"
#include "DigitRecognisionNN/Activators.hpp"
#include <iostream>

int main() {

    constexpr size_t INPUT_SIZE = 728;
    constexpr size_t OUTPUT_SIZE = 128;

    using ActT = decltype(&Activators::softmax<double, OUTPUT_SIZE>);

    Layer<double, ActT, INPUT_SIZE, OUTPUT_SIZE> firstLayer(Activators::softmax<double, OUTPUT_SIZE>);

    // Dataset::loadTrainingDataset();
    // auto firstImage = Dataset::getImages()[0];

    // Dataset::displayImage(firstImage);
    // std::cout << static_cast<int>(Dataset::getLabels()[0]) << std::endl <<
    // Dataset::getLabels().size();
    return 0;
}
