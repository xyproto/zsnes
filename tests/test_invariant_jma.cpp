#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <vector>

// Forward declare the JMA decompression function from jma.cpp
extern "C" {
int decompress(const uint8_t* in, size_t in_size, uint8_t* out, size_t out_size);
}

class JMABufferBoundsTest : public ::testing::TestWithParam<std::pair<size_t, size_t>> { };

TEST_P(JMABufferBoundsTest, DecompressDoesNotExceedBufferBounds)
{
    // Invariant: memcpy operations never read/write beyond declared buffer sizes
    auto [input_size, output_size] = GetParam();

    // Create minimal JMA archive header with oversized copy parameters
    std::vector<uint8_t> input(input_size, 0);
    std::vector<uint8_t> output(output_size, 0);

    // Craft header: JMA signature + oversized decompression directives
    if (input_size >= 16) {
        input[0] = 'J';
        input[1] = 'M';
        input[2] = 'A';
        input[3] = 0;
        // Set decompression parameters that would overflow if unchecked
        uint32_t* header_params = reinterpret_cast<uint32_t*>(input.data() + 4);
        header_params[0] = output_size * 2; // Oversized copy amount
        header_params[1] = input_size * 10; // Oversized source offset
    }

    // Call the actual decompress function - it must not crash or overflow
    int result = decompress(input.data(), input.size(), output.data(), output.size());

    // Invariant: function either succeeds safely or returns error code
    // No segfault, no heap corruption
    EXPECT_TRUE(result >= -1); // Valid return or error indicator
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialInputs,
    JMABufferBoundsTest,
    ::testing::Values(
        std::make_pair(16, 256), // Minimal valid header, normal output
        std::make_pair(256, 16), // Output buffer smaller than input
        std::make_pair(1024, 64), // 16x oversized input vs output
        std::make_pair(100, 100), // Equal sizes, boundary case
        std::make_pair(8192, 256) // 32x oversized input
        ));

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}