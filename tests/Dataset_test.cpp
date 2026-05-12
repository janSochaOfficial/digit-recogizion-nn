#include "DigitRecognisionNN/Dataset.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include <sstream>

namespace {
constexpr const char* TEST_DATASET_DIR = "./test_dataset";
constexpr const char* TEST_IMAGES_FILE = "./test_dataset/test-images.idx3-ubyte";
constexpr const char* TEST_LABELS_FILE = "./test_dataset/test-labels.idx1-ubyte";
constexpr uint32 MAGIC_IMAGES = 0x00000803U;
constexpr uint32 MAGIC_LABELS = 0x00000801U;
constexpr uint32 IMAGE_SIZE = 28;
constexpr uint32 TEST_IMAGE_COUNT = 5;

void createTestDirectory() {
    std::filesystem::create_directories(TEST_DATASET_DIR);
}

void removeTestDirectory() {
    if (std::filesystem::exists(TEST_DATASET_DIR)) {
        std::filesystem::remove_all(TEST_DATASET_DIR);
    }
}

void writeBigEndian(std::ofstream& file, uint32 value) {
    uint32 swapped = __builtin_bswap32(value);
    file.write(reinterpret_cast<const char*>(&swapped), sizeof(swapped));
}

void createValidTestImageFile(const std::string& filepath, uint32 imageCount) {
    std::ofstream file(filepath, std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "Failed to create test image file";
    
    writeBigEndian(file, MAGIC_IMAGES);
    writeBigEndian(file, imageCount);
    writeBigEndian(file, IMAGE_SIZE);
    writeBigEndian(file, IMAGE_SIZE);
    
    for (uint32 img = 0; img < imageCount; ++img) {
        for (uint32 pixel = 0; pixel < IMAGE_SIZE * IMAGE_SIZE; ++pixel) {
            byte pixelValue = static_cast<byte>((img * 10 + pixel) % 256);
            file.write(reinterpret_cast<const char*>(&pixelValue), 1);
        }
    }
    file.close();
}

void createValidTestLabelFile(const std::string& filepath, uint32 labelCount) {
    std::ofstream file(filepath, std::ios::binary);
    ASSERT_TRUE(file.is_open()) << "Failed to create test label file";
    
    writeBigEndian(file, MAGIC_LABELS);
    writeBigEndian(file, labelCount);
    
    for (uint32 i = 0; i < labelCount; ++i) {
        byte label = static_cast<byte>(i % 10);
        file.write(reinterpret_cast<const char*>(&label), 1);
    }
    file.close();
}

} // anonymous namespace

class DatasetAccessor : public Dataset {
    public:
    using Dataset::loadImages;
    using Dataset::loadLabels;
    using Dataset::checkIfFileExists;
    using Dataset::normalizeScale;
    using Dataset::readLittleEdian;
    using Dataset::toHexString;
    using Dataset::readImage;
};

class DatasetTest : public ::testing::Test {
protected:
    void SetUp() override {
        DatasetAccessor::clear();
        createTestDirectory();
    }
    
    void TearDown() override {
        DatasetAccessor::clear();
        removeTestDirectory();
    }
};

TEST_F(DatasetTest, InitialStateIsEmpty) {
    EXPECT_FALSE(DatasetAccessor::isDatasetLoaded());
    EXPECT_EQ(DatasetAccessor::getImages().size(), 0);
    EXPECT_EQ(DatasetAccessor::getLabels().size(), 0);
}

TEST_F(DatasetTest, ClearRemovesAllData) {
    createValidTestImageFile(TEST_IMAGES_FILE, TEST_IMAGE_COUNT);
    createValidTestLabelFile(TEST_LABELS_FILE, TEST_IMAGE_COUNT);
    
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    DatasetAccessor::loadLabels(TEST_LABELS_FILE);
    
    ASSERT_GT(DatasetAccessor::getImages().size(), 0);
    ASSERT_GT(DatasetAccessor::getLabels().size(), 0);
    
    DatasetAccessor::clear();
    
    EXPECT_EQ(DatasetAccessor::getImages().size(), 0);
    EXPECT_EQ(DatasetAccessor::getLabels().size(), 0);
}

TEST_F(DatasetTest, ThrowsWhenImageFileDoesNotExist) {
    EXPECT_THROW({
        DatasetAccessor::checkIfFileExists("nonexistent_file.idx");
    }, std::filesystem::filesystem_error);
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
TEST_F(DatasetTest, LoadImagesWithValidFile) { 
    createValidTestImageFile(TEST_IMAGES_FILE, TEST_IMAGE_COUNT);
    
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    
    auto images = DatasetAccessor::getImages();
    EXPECT_EQ(images.size(), TEST_IMAGE_COUNT);
    
    for (const auto& image : images) {
        EXPECT_EQ(image.size(), DatasetAccessor::PIXEL_COUNT);
        for (float pixel : image) {
            EXPECT_GE(pixel, 0.0F);
            EXPECT_LE(pixel, 1.0F);
        }
    }
}

TEST_F(DatasetTest, LoadImagesThrowsOnInvalidMagicNumber) {
    std::ofstream file(TEST_IMAGES_FILE, std::ios::binary);
    constexpr uint32 INVALID_MAGIC = 0x12345678U;
    writeBigEndian(file, INVALID_MAGIC);
    writeBigEndian(file, TEST_IMAGE_COUNT);
    writeBigEndian(file, IMAGE_SIZE);
    writeBigEndian(file, IMAGE_SIZE);
    file.close();
    
    EXPECT_THROW({
        DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    }, std::runtime_error);
}

TEST_F(DatasetTest, LoadImagesThrowsOnInvalidImageSize) {
    std::ofstream file(TEST_IMAGES_FILE, std::ios::binary);
    constexpr uint32 INVALID_SIZE = 32;
    writeBigEndian(file, MAGIC_IMAGES);
    writeBigEndian(file, TEST_IMAGE_COUNT);
    writeBigEndian(file, INVALID_SIZE);
    writeBigEndian(file, INVALID_SIZE);
    file.close();
    
    EXPECT_THROW({
        DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    }, std::runtime_error);
}

TEST_F(DatasetTest, LoadLabelsWithValidFile) {
    createValidTestImageFile(TEST_IMAGES_FILE, TEST_IMAGE_COUNT);
    createValidTestLabelFile(TEST_LABELS_FILE, TEST_IMAGE_COUNT);
    
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    DatasetAccessor::loadLabels(TEST_LABELS_FILE);
    
    auto labels = DatasetAccessor::getLabels();
    EXPECT_EQ(labels.size(), TEST_IMAGE_COUNT);
    
    for (size_t i = 0; i < labels.size(); ++i) {
        EXPECT_EQ(labels[i], static_cast<byte>(i % 10));
    }
}

TEST_F(DatasetTest, LoadLabelsThrowsOnInvalidMagicNumber) {
    createValidTestImageFile(TEST_IMAGES_FILE, TEST_IMAGE_COUNT);
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    
    std::ofstream file(TEST_LABELS_FILE, std::ios::binary);
    constexpr uint32 INVALID_MAGIC = 0x87654321U;
    writeBigEndian(file, INVALID_MAGIC);
    writeBigEndian(file, TEST_IMAGE_COUNT);
    file.close();
    
    EXPECT_THROW({
        DatasetAccessor::loadLabels(TEST_LABELS_FILE);
    }, std::runtime_error);
}

TEST_F(DatasetTest, LoadLabelsThrowsWhenCountMismatch) {
    createValidTestImageFile(TEST_IMAGES_FILE, TEST_IMAGE_COUNT);
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    
    constexpr uint32 MISMATCHED_COUNT = TEST_IMAGE_COUNT + 3;
    createValidTestLabelFile(TEST_LABELS_FILE, MISMATCHED_COUNT);
    
    EXPECT_THROW({
        DatasetAccessor::loadLabels(TEST_LABELS_FILE);
    }, std::runtime_error);
}

TEST_F(DatasetTest, NormalizeScaleConvertsCorrectly) {
    EXPECT_FLOAT_EQ(DatasetAccessor::normalizeScale(0), 0.0F);
    EXPECT_FLOAT_EQ(DatasetAccessor::normalizeScale(255), 1.0F);
    EXPECT_FLOAT_EQ(DatasetAccessor::normalizeScale(127), 127.0F / 255.0F);
    EXPECT_FLOAT_EQ(DatasetAccessor::normalizeScale(128), 128.0F / 255.0F);
}

TEST_F(DatasetTest, ReadLittleEndianConvertsCorrectly) {
    std::ofstream outfile(TEST_IMAGES_FILE, std::ios::binary);
    writeBigEndian(outfile, 0x12345678U);
    outfile.close();
    
    std::ifstream infile(TEST_IMAGES_FILE, std::ios::binary);
    uint32 result = DatasetAccessor::readLittleEdian(infile);
    infile.close();
    
    EXPECT_EQ(result, 0x12345678U);
}

TEST_F(DatasetTest, ToHexStringFormatsCorrectly) {
    EXPECT_EQ(DatasetAccessor::toHexString(0x00000803U), "0x00000803");
    EXPECT_EQ(DatasetAccessor::toHexString(0x00000801U), "0x00000801");
    EXPECT_EQ(DatasetAccessor::toHexString(0x12345678U), "0x12345678");
    EXPECT_EQ(DatasetAccessor::toHexString(0xABCDEF00U), "0xABCDEF00");
}

TEST_F(DatasetTest, DisplayImageProducesOutput) {
    std::array<float, DatasetAccessor::PIXEL_COUNT> testImage{};
    
    for (size_t i = 0; i < testImage.size(); ++i) {
        testImage.at(i) = static_cast<float>(i % 256) / 255.0F;
    }
    
    std::ostringstream output;
    DatasetAccessor::displayImage(testImage, output);
    
    std::string result = output.str();
    EXPECT_FALSE(result.empty());
    
    size_t newlineCount = std::count(result.begin(), result.end(), '\n');
    EXPECT_EQ(newlineCount, IMAGE_SIZE);
}

TEST_F(DatasetTest, DisplayImageWithAllBlackPixels) {
    std::array<float, DatasetAccessor::PIXEL_COUNT> blackImage{};
    std::fill(blackImage.begin(), blackImage.end(), 0.0F);
    
    std::ostringstream output;
    DatasetAccessor::displayImage(blackImage, output);
    
    std::string result = output.str();
    EXPECT_NE(result.find(' '), std::string::npos);
}

TEST_F(DatasetTest, DisplayImageWithAllWhitePixels) {
    std::array<float, DatasetAccessor::PIXEL_COUNT> whiteImage{};
    std::fill(whiteImage.begin(), whiteImage.end(), 1.0F);
    
    std::ostringstream output;
    DatasetAccessor::displayImage(whiteImage, output);
    
    std::string result = output.str();
    EXPECT_NE(result.find('@'), std::string::npos);
}

TEST_F(DatasetTest, ReadImageNormalizesPixels) {
    std::ofstream file(TEST_IMAGES_FILE, std::ios::binary);
    writeBigEndian(file, MAGIC_IMAGES);
    writeBigEndian(file, 1);
    writeBigEndian(file, IMAGE_SIZE);
    writeBigEndian(file, IMAGE_SIZE);
    
    for (uint32 i = 0; i < DatasetAccessor::PIXEL_COUNT; ++i) {
        byte value = static_cast<byte>(i % 256);
        file.write(reinterpret_cast<const char*>(&value), 1);
    }
    file.close();
    
    std::ifstream infile(TEST_IMAGES_FILE, std::ios::binary);
    infile.ignore(16);
    
    auto image = DatasetAccessor::readImage(infile);
    infile.close();
    
    EXPECT_EQ(image.size(), DatasetAccessor::PIXEL_COUNT);
    for (float pixel : image) {
        EXPECT_GE(pixel, 0.0F);
        EXPECT_LE(pixel, 1.0F);
    }
}

TEST_F(DatasetTest, ConstantsAreCorrect) {
    EXPECT_EQ(DatasetAccessor::IMAGE_SIZE, 28);
    EXPECT_EQ(DatasetAccessor::PIXEL_COUNT, 784);
}

TEST_F(DatasetTest, MultipleLoadsClearPreviousData) {
    createValidTestImageFile(TEST_IMAGES_FILE, TEST_IMAGE_COUNT);
    createValidTestLabelFile(TEST_LABELS_FILE, TEST_IMAGE_COUNT);
    
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    DatasetAccessor::loadLabels(TEST_LABELS_FILE);
    size_t firstLoadSize = DatasetAccessor::getImages().size();
    
    DatasetAccessor::clear();
    
    constexpr uint32 SECOND_IMAGE_COUNT = 3;
    createValidTestImageFile(TEST_IMAGES_FILE, SECOND_IMAGE_COUNT);
    createValidTestLabelFile(TEST_LABELS_FILE, SECOND_IMAGE_COUNT);
    
    DatasetAccessor::loadImages(TEST_IMAGES_FILE);
    DatasetAccessor::loadLabels(TEST_LABELS_FILE);
    
    EXPECT_EQ(DatasetAccessor::getImages().size(), SECOND_IMAGE_COUNT);
    EXPECT_NE(DatasetAccessor::getImages().size(), firstLoadSize);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}