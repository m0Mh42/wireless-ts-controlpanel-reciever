#ifndef _H_SAFETY_PROTOCOL
#define _H_SAFETY_PROTOCOL 1

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SAFETY_FRAME_SIZE 32
#define PROTOCOL_MAGIC_PREAMBLE 0xA5
#define PROTOCOL_VERSION 0x01

#ifdef __cplusplus
namespace IndustrialSafety::Safety {

/**
 * @brief Frame types adhering to Black Channel specification
 */
enum class FrameType : uint8_t {
    HEARTBEAT    = 0x10,
    COMMAND      = 0x20,
    ESTOP_ACTIVE = 0x30,
    TELEMETRY    = 0x40,
    ACK          = 0x50,
    SET_CHANNEL  = 0x60
};

/**
 * @brief Structured Application Payload (14 Bytes)
 */
struct ApplicationPayload {
    uint16_t buttons_primary;       // Primary 16-bit button mask (15 buttons)
    uint16_t buttons_complement;    // Bitwise complement (~buttons_primary)
    uint8_t  active_unit;           // Active printing unit (0-15 from PLC)
    uint8_t  status_flags;          // TS Status (OFF, ON, BATTERYLOW, etc.)
    uint16_t battery_mv;            // Handheld battery voltage in millivolts
    uint16_t diagnostic_flags;      // RF channel, link quality, telemetry
    uint16_t spare1;                // Reserved alignment
    uint16_t spare2;                // Reserved alignment
};

/**
 * @brief Unpacked Safety Frame Structure (32 Bytes total on wire)
 */
struct SafetyFrame {
    uint8_t  preamble{PROTOCOL_MAGIC_PREAMBLE};
    uint8_t  version{PROTOCOL_VERSION};
    uint16_t sender_id{0};
    uint16_t target_id{0};
    uint32_t sequence_number{0};
    uint16_t session_id{0};
    FrameType frame_type{FrameType::HEARTBEAT};
    uint8_t  payload_length{sizeof(ApplicationPayload)};
    ApplicationPayload payload{};
    uint32_t crc32{0};
};

/**
 * @brief Error Codes for Black Channel Integrity Verification
 */
enum class SafetyValidationResult : uint8_t {
    VALID = 0,
    ERR_FRAME_NULL_OR_UNDERSIZED,
    ERR_BAD_PREAMBLE,
    ERR_UNSUPPORTED_VERSION,
    ERR_CRC32_MISMATCH,
    ERR_SENDER_ID_MISMATCH,
    ERR_TARGET_ID_MISMATCH,
    ERR_SESSION_ID_MISMATCH,
    ERR_SEQUENCE_STALE_OR_REPLAY,
    ERR_COMPLEMENTARY_BIT_FAULT,
    ERR_PAYLOAD_LENGTH_INVALID,
    ERR_ESTOP_TRIPPED
};

const char* validation_result_to_string(SafetyValidationResult result);

class SafetyProtocolEngine {
public:
    static bool serialize_frame(const SafetyFrame& frame, uint8_t* out_buffer, size_t buffer_size);

    static SafetyValidationResult deserialize_and_validate(
        const uint8_t* in_buffer,
        size_t buffer_size,
        uint16_t expected_sender_id,
        uint16_t expected_target_id,
        uint16_t expected_session_id,
        uint32_t last_valid_sequence,
        uint32_t max_sequence_gap,
        SafetyFrame& out_frame
    );

    static void pack_buttons(uint16_t buttons, ApplicationPayload& payload);
    static bool verify_buttons(const ApplicationPayload& payload);
};

} // namespace IndustrialSafety::Safety

using namespace IndustrialSafety::Safety;

#endif // __cplusplus

#endif // _H_SAFETY_PROTOCOL
