#ifndef DATASET_HPP
#define DATASET_HPP

#include <array>
#include <cstdint>
#include <iostream>
#include <span>
#include <string>
#include <vector>

using uint32 = std::uint32_t;
using byte = std::uint8_t;

class Dataset {
  public:
    static constexpr uint32 IMAGE_SIZE = 28;
    static constexpr uint32 PIXEL_COUNT =
        Dataset::IMAGE_SIZE * Dataset::IMAGE_SIZE;

    Dataset() = delete;

    static std::span<const std::array<float, Dataset::PIXEL_COUNT>> getImages();
    static std::span<const byte> getLabels();

    static void loadTrainingDataset();
    static void loadTestingDataset();
    static void clear();
    static bool isDatasetLoaded();

    static void
    displayImage(const std::array<float, Dataset::PIXEL_COUNT> &image,
                 std::ostream &stream = std::cout);

  protected:
    static std::string toHexString(uint32 number);
    static uint32 readLittleEdian(std::ifstream &file);
    static std::array<float, Dataset::PIXEL_COUNT>
    readImage(std::ifstream &file);
    static float normalizeScale(byte toNormalize);
    static void checkIfFileExists(const std::string &fileName);
    static void loadImages(const std::string &fileName,
                           uint32 magic_ = 0x00000803U);
    static void loadLabels(const std::string &fileName,
                           uint32 magic_ = 0x00000801U);
};

#endif
