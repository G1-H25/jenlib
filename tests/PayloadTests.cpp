//! @file tests/PayloadTests.cpp
//! @brief Tests for BlePayload bounds and access safety.

#include <cstdint>
#include "unity.h"

#include <jenlib/ble/Payload.h>

using namespace ble;

//! @test Append within capacity succeeds and size increments correctly
void test_payload_append_within_capacity(void) {
    BlePayload p;
    p.clear();
    for (std::size_t i = 0; i < kMaxPayload; ++i) {
        TEST_ASSERT_TRUE(p.append_u8(static_cast<std::uint8_t>(i & 0xFF)));
        TEST_ASSERT_EQUAL_UINT32(i + 1, p.size);
    }
    // Next append should fail
    TEST_ASSERT_FALSE(p.append_u8(0xAA));
}

//! @test Append raw enforces bounds
void test_payload_append_raw_bounds(void) {
    BlePayload p;
    p.clear();
    const std::uint8_t buf[10] = {0,1,2,3,4,5,6,7,8,9};
    TEST_ASSERT_TRUE(p.append_raw(buf, 10));
    TEST_ASSERT_EQUAL_UINT32(10, p.size);
    // Try to append more than remaining capacity
    const std::size_t remaining = kMaxPayload - 10;
    TEST_ASSERT_TRUE(p.append_raw(buf, remaining));
    TEST_ASSERT_EQUAL_UINT32(kMaxPayload, p.size);
    TEST_ASSERT_FALSE(p.append_raw(buf, 1));
}

//! @test Read helpers enforce bounds and advance index properly
void test_payload_read_bounds_and_progress(void) {
    BlePayload p;
    p.clear();
    TEST_ASSERT_TRUE(p.append_u8(0x11));
    TEST_ASSERT_TRUE(p.append_u16le(0x2233));
    TEST_ASSERT_TRUE(p.append_u32le(0x44556677));

    std::size_t i = 0;
    std::uint8_t b = 0;
    std::uint16_t u16 = 0;
    std::uint32_t u32 = 0;

    TEST_ASSERT_TRUE(read_u8(p, i, b));
    TEST_ASSERT_EQUAL_UINT8(0x11, b);
    TEST_ASSERT_EQUAL_UINT32(1, i);

    TEST_ASSERT_TRUE(read_u16le(p, i, u16));
    TEST_ASSERT_EQUAL_UINT16(0x2233, u16);
    TEST_ASSERT_EQUAL_UINT32(3, i);

    TEST_ASSERT_TRUE(read_u32le(p, i, u32));
    TEST_ASSERT_EQUAL_UINT32(0x44556677, u32);
    TEST_ASSERT_EQUAL_UINT32(p.size, i);

    // Out-of-bounds reads fail and do not advance index
    std::uint8_t dummy = 0;
    TEST_ASSERT_FALSE(read_u8(p, i, dummy));
    TEST_ASSERT_EQUAL_UINT32(p.size, i);
}

//! @test Signed 16-bit append/read roundtrip
void test_payload_i16_roundtrip(void) {
    BlePayload p;
    p.clear();
    const std::int16_t v = static_cast<std::int16_t>(-1234);
    TEST_ASSERT_TRUE(p.append_i16le(v));
    std::size_t i = 0;
    std::int16_t out = 0;
    TEST_ASSERT_TRUE(read_i16le(p, i, out));
    TEST_ASSERT_EQUAL_INT16(v, out);
}


