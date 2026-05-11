#include "DigitRecognisionNN/Dataset.hpp"
#include <gtest/gtest.h>
#include <sstream>

TEST(DatasetTest, LoadsDataset) {
    EXPECT_NO_THROW(Dataset::loadTrainingDataset());
}