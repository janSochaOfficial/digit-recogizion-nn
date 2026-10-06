#include "DigitRecognisionNN/Dataset.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <stdexcept>

namespace {
auto &getDataLoaded_() {
    static bool dataLoaded_ = false;
    return dataLoaded_;
}

auto &getImages_() {
    static std::vector<std::array<float, Dataset::PIXEL_COUNT>> images_;
    return images_;
}

auto &getLabels_() {
    static std::vector<byte> labels_;
    return labels_;
}
constexpr uint32 IMAGE_FILE_MAGIC = 0x00000803U;
constexpr uint32 LABEL_FILE_MAGIC = 0x00000801U;

const std::string TRAINING_IMAGES_FILE_PATH =
    "./dataset/train-images.idx3-ubyte";
const std::string TRAINING_LABELS_FILE_PATH =
    "./dataset/train-labels.idx1-ubyte";

const std::string TESTING_IMAGES_FILE_PATH = "./dataset/t10k-images.idx3-ubyte";
const std::string TESTING_LABELS_FILE_PATH = "./dataset/t10k-labels.idx1-ubyte";
} // namespace

std::span<const std::array<float, Dataset::PIXEL_COUNT>> Dataset::getImages() {
    return getImages_();
}

std::span<const byte> Dataset::getLabels() { return getLabels_(); }

bool Dataset::isDatasetLoaded() { return getDataLoaded_(); }

void Dataset::clear() {
    auto &labels = getLabels_();
    auto &images = getImages_();
    auto &dataLoaded = getDataLoaded_();
    labels.clear();
    images.clear();

    dataLoaded = false;
}

void Dataset::loadTrainingDataset() {
    auto &dataLoaded = getDataLoaded_();

    checkIfFileExists(TRAINING_IMAGES_FILE_PATH);
    checkIfFileExists(TRAINING_LABELS_FILE_PATH);

    loadImages(TRAINING_IMAGES_FILE_PATH);
    loadLabels(TRAINING_LABELS_FILE_PATH);

    dataLoaded = true;
}

uint32 Dataset::readLittleEdian(std::ifstream &file) {
    uint32 result = 0;
    file.read(reinterpret_cast<char *>(&result), sizeof(result));
    return __builtin_bswap32(result);
}

std::array<float, Dataset::PIXEL_COUNT>
Dataset::readImage(std::ifstream &file) {
    std::array<byte, PIXEL_COUNT> buffer{};
    file.read(reinterpret_cast<char *>(buffer.data()), buffer.size());

    std::array<float, PIXEL_COUNT> normalized{};
    std::transform(buffer.begin(), buffer.end(), normalized.begin(),
                   normalizeScale);
    return normalized;
}

std::string Dataset::toHexString(uint32 value) {
    std::ostringstream oss;
    oss << "0x" << std::hex << std::uppercase << std::setw(8)
        << std::setfill('0') << value;
    return oss.str();
}

float Dataset::normalizeScale(byte toNormalize) {
    static constexpr float NORMALIZATION_FACTOR =
        1.0F / static_cast<float>(UINT8_MAX);
    return static_cast<float>(toNormalize) * NORMALIZATION_FACTOR;
}

void Dataset::checkIfFileExists(const std::string &fileName) {
    if (!std::filesystem::exists(fileName)) {
        throw std::filesystem::filesystem_error(
            "Training data file could not be found", fileName,
            std::make_error_code(std::errc::no_such_file_or_directory));
    }
}

void Dataset::loadImages(const std::string &fileName, uint32 magic_) {
    auto &images = getImages_();

    std::ifstream file(fileName, std::ios::binary);

    uint32 magic = readLittleEdian(file);
    uint32 image_count = readLittleEdian(file);
    uint32 image_rows = readLittleEdian(file);
    uint32 image_cols = readLittleEdian(file);

    if (magic != magic_) {
        throw std::runtime_error("Invalid MNIST image file: bad magic number. "
                                 "Expected 0x" +
                                 toHexString(magic_) + ", got 0x" +
                                 toHexString(magic));
    }

    if (image_rows != IMAGE_SIZE || image_cols != IMAGE_SIZE) {
        throw std::runtime_error("Invalid MNIST image file: bad image size. "
                                 "Expected " +
                                 std::to_string(IMAGE_SIZE) + "x" +
                                 std::to_string(IMAGE_SIZE) + ", got " +
                                 std::to_string(image_rows) + "x" +
                                 std::to_string(image_cols));
    }

    for (uint32 i = 0; i < image_count; ++i) {
        images.push_back(readImage(file));
    }
}

void Dataset::loadLabels(const std::string &fileName, uint32 magic_) {
    auto &labels = getLabels_();
    auto &images = getImages_();

    std::ifstream file(fileName, std::ios::binary);

    uint32 magic = readLittleEdian(file);
    if (magic != magic_) {
        throw std::runtime_error("Invalid MNIST image file: bad magic number. "
                                 "Expected 0x" +
                                 toHexString(magic_) + ", got 0x" +
                                 toHexString(magic));
    }
    uint32 label_count = readLittleEdian(file);

    if (label_count != images.size()) {
        throw std::runtime_error("label and image counts do not match. "
                                 "label count " +
                                 std::to_string(label_count) +
                                 ", image count " +
                                 std::to_string(images.size())

        );
    }
    byte buffer = 0;
    for (uint32 i = 0; i < label_count; ++i) {
        file.read(reinterpret_cast<char *>(&buffer), 1);
        labels.push_back(buffer);
    }
}

void Dataset::displayImage(const std::array<float, Dataset::PIXEL_COUNT> &image,
                           std::ostream &stream) {
    static constexpr std::string_view SHADES = " .:-=+*#%@";
    for (size_t row = 0; row < IMAGE_SIZE; ++row) {
        for (size_t col = 0; col < IMAGE_SIZE; ++col) {
            float pixel = image.at(row * IMAGE_SIZE + col);
            auto index = static_cast<size_t>(pixel * (SHADES.size() - 1));
            stream << SHADES[index]
                   << SHADES[index]; // doubled for aspect ratio
        }
        stream << '\n';
    }
}

void Dataset::loadTestingDataset() {
    auto &dataLoaded = getDataLoaded_();

    checkIfFileExists(TESTING_IMAGES_FILE_PATH);
    checkIfFileExists(TESTING_LABELS_FILE_PATH);

    loadImages(TESTING_IMAGES_FILE_PATH);
    loadLabels(TESTING_LABELS_FILE_PATH);

    dataLoaded = true;
}