#include "DigitRecognisionNN/Activators.hpp"
#include "DigitRecognisionNN/Dataset.hpp"
#include "DigitRecognisionNN/Layer.hpp"
#include <exception>
#include <iostream>

int main() try {

    constexpr size_t INPUT_SIZE = 728;
    constexpr size_t OUTPUT_SIZE = 128;

    Layer<double, Activators::Softmax<double>> firstLayer(
        INPUT_SIZE, OUTPUT_SIZE, Activators::Softmax<double>{});

    // Dataset::loadTrainingDataset();
    // auto firstImage = Dataset::getImages()[0];

    // Dataset::displayImage(firstImage);
    // std::cout << static_cast<int>(Dataset::getLabels()[0]) << std::endl <<
    // Dataset::getLabels().size();
    return 0;
} catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
}
