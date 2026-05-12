#include "DigitRecognisionNN/Dataset.hpp"
#include <iostream>

int main() {
    Dataset::loadTrainingDataset();
    auto firstImage = Dataset::getImages()[0];

    Dataset::displayImage(firstImage);
    std::cout << static_cast<int>(Dataset::getLabels()[0]) << std::endl << Dataset::getLabels().size();
    return 0;
}
