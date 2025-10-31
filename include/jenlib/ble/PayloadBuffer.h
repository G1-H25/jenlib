//! @file include/jenlib/ble/PayloadBuffer.h
//! @brief Common circular buffer for BLE payloads across platforms.
//! @copyright 2025 Jennifer Gott, released under the MIT License.
//! @author Jennifer Gott (jennifer.gott@chasacademy.se)

#ifndef INCLUDE_JENLIB_BLE_PAYLOADBUFFER_H_
#define INCLUDE_JENLIB_BLE_PAYLOADBUFFER_H_

#include <array>
#include <cstddef>
#include <utility>
#include "jenlib/ble/Payload.h"

namespace jenlib::ble {

//! @brief Fixed-size circular buffer for `BlePayload` items.
struct PayloadBuffer {
    static constexpr std::size_t kMaxBufferedPayloads = 10;
    std::array<BlePayload, kMaxBufferedPayloads> payloads{};
    using iterator = std::array<BlePayload, kMaxBufferedPayloads>::iterator;
    iterator write_it = payloads.begin();
    iterator read_it = payloads.begin();
    std::size_t count = 0;

    //! @brief Push a payload into the buffer; returns false if full.
    bool push(BlePayload payload) {
        if (full()) {
            return false;
        }
        *write_it = std::move(payload);
        ++write_it;
        if (write_it == payloads.end()) {
            write_it = payloads.begin();
        }
        count++;
        return true;
    }

    //! @brief Pop the next payload into out_payload; returns false if empty.
    bool pop(BlePayload& out_payload) {
        if (empty()) {
            return false;
        }
        out_payload = std::move(*read_it);
        ++read_it;
        if (read_it == payloads.end()) {
            read_it = payloads.begin();
        }
        count--;
        return true;
    }

    //! @brief Check if buffer is empty.
    bool empty() const { return count == 0; }

    //! @brief Check if buffer is full.
    bool full() const { return count >= kMaxBufferedPayloads; }
};

}  // namespace jenlib::ble

#endif  // INCLUDE_JENLIB_BLE_PAYLOADBUFFER_H_


