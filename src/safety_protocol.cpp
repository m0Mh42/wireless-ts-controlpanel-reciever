#include "safety_protocol.h"
#include "safety_crc32.h"
#include <string.h>

namespace IndustrialSafety::Safety {

const char* validation_result_to_string(SafetyValidationResult result) {
    switch (result) {
        case SafetyValidationResult::VALID:
            return "VALID";
        case SafetyValidationResult::ERR_FRAME_NULL_OR_UNDERSIZED:
            return "ERR_UNDERSIZED";
        case SafetyValidationResult::ERR_BAD_PREAMBLE:
            return "ERR_PREAMBLE";
        case SafetyValidationResult::ERR_UNSUPPORTED_VERSION:
            return "ERR_VERSION";
        case SafetyValidationResult::ERR_CRC32_MISMATCH:
            return "ERR_CRC32";
        case SafetyValidationResult::ERR_SENDER_ID_MISMATCH:
            return "ERR_SENDER_ID";
        case SafetyValidationResult::ERR_TARGET_ID_MISMATCH:
            return "ERR_TARGET_ID";
        case SafetyValidationResult::ERR_SESSION_ID_MISMATCH:
            return "ERR_SESSION_ID";
        case SafetyValidationResult::ERR_SEQUENCE_STALE_OR_REPLAY:
            return "ERR_SEQUENCE";
        case SafetyValidationResult::ERR_COMPLEMENTARY_BIT_FAULT:
            return "ERR_COMPLEMENT";
        case SafetyValidationResult::ERR_PAYLOAD_LENGTH_INVALID:
            return "ERR_PAYLOAD_LEN";
        case SafetyValidationResult::ERR_ESTOP_TRIPPED:
            return "ERR_ESTOP";
        default:
            return "ERR_UNKNOWN";
    }
}

void SafetyProtocolEngine::pack_buttons(uint16_t buttons, ApplicationPayload& payload) {
    payload.buttons_primary = buttons;
    payload.buttons_complement = (uint16_t)(~buttons);
}

bool SafetyProtocolEngine::verify_buttons(const ApplicationPayload& payload) {
    return (uint16_t)(payload.buttons_primary ^ payload.buttons_complement) == 0xFFFFU;
}

bool SafetyProtocolEngine::serialize_frame(const SafetyFrame& frame, uint8_t* out_buffer, size_t buffer_size) {
    if (out_buffer == NULL || buffer_size < SAFETY_FRAME_SIZE) {
        return false;
    }

    out_buffer[0] = frame.preamble;
    out_buffer[1] = frame.version;

    out_buffer[2] = (uint8_t)(frame.sender_id & 0xFF);
    out_buffer[3] = (uint8_t)((frame.sender_id >> 8) & 0xFF);

    out_buffer[4] = (uint8_t)(frame.target_id & 0xFF);
    out_buffer[5] = (uint8_t)((frame.target_id >> 8) & 0xFF);

    out_buffer[6] = (uint8_t)(frame.sequence_number & 0xFF);
    out_buffer[7] = (uint8_t)((frame.sequence_number >> 8) & 0xFF);
    out_buffer[8] = (uint8_t)((frame.sequence_number >> 16) & 0xFF);
    out_buffer[9] = (uint8_t)((frame.sequence_number >> 24) & 0xFF);

    out_buffer[10] = (uint8_t)(frame.session_id & 0xFF);
    out_buffer[11] = (uint8_t)((frame.session_id >> 8) & 0xFF);

    out_buffer[12] = (uint8_t)frame.frame_type;
    out_buffer[13] = sizeof(ApplicationPayload); // 14 bytes

    // Payload (14 bytes): offsets 14..27
    out_buffer[14] = (uint8_t)(frame.payload.buttons_primary & 0xFF);
    out_buffer[15] = (uint8_t)((frame.payload.buttons_primary >> 8) & 0xFF);

    out_buffer[16] = (uint8_t)(frame.payload.buttons_complement & 0xFF);
    out_buffer[17] = (uint8_t)((frame.payload.buttons_complement >> 8) & 0xFF);

    out_buffer[18] = frame.payload.active_unit;
    out_buffer[19] = frame.payload.status_flags;

    out_buffer[20] = (uint8_t)(frame.payload.battery_mv & 0xFF);
    out_buffer[21] = (uint8_t)((frame.payload.battery_mv >> 8) & 0xFF);

    out_buffer[22] = (uint8_t)(frame.payload.diagnostic_flags & 0xFF);
    out_buffer[23] = (uint8_t)((frame.payload.diagnostic_flags >> 8) & 0xFF);

    out_buffer[24] = (uint8_t)(frame.payload.spare1 & 0xFF);
    out_buffer[25] = (uint8_t)((frame.payload.spare1 >> 8) & 0xFF);

    out_buffer[26] = (uint8_t)(frame.payload.spare2 & 0xFF);
    out_buffer[27] = (uint8_t)((frame.payload.spare2 >> 8) & 0xFF);

    // Compute CRC-32 over bytes 0..27 (28 bytes)
    uint32_t computed_crc = crc32_calculate_ieee(out_buffer, 28, 0xFFFFFFFFUL);

    out_buffer[28] = (uint8_t)(computed_crc & 0xFF);
    out_buffer[29] = (uint8_t)((computed_crc >> 8) & 0xFF);
    out_buffer[30] = (uint8_t)((computed_crc >> 16) & 0xFF);
    out_buffer[31] = (uint8_t)((computed_crc >> 24) & 0xFF);

    return true;
}

SafetyValidationResult SafetyProtocolEngine::deserialize_and_validate(
    const uint8_t* in_buffer,
    size_t buffer_size,
    uint16_t expected_sender_id,
    uint16_t expected_target_id,
    uint16_t expected_session_id,
    uint32_t last_valid_sequence,
    uint32_t max_sequence_gap,
    SafetyFrame& out_frame
) {
    if (in_buffer == NULL || buffer_size < SAFETY_FRAME_SIZE) {
        return SafetyValidationResult::ERR_FRAME_NULL_OR_UNDERSIZED;
    }

    // 1. Preamble
    if (in_buffer[0] != PROTOCOL_MAGIC_PREAMBLE) {
        return SafetyValidationResult::ERR_BAD_PREAMBLE;
    }

    // 2. Version
    if (in_buffer[1] != PROTOCOL_VERSION) {
        return SafetyValidationResult::ERR_UNSUPPORTED_VERSION;
    }

    // 3. CRC-32 verification
    uint32_t expected_crc = ((uint32_t)in_buffer[28]) |
                            (((uint32_t)in_buffer[29]) << 8) |
                            (((uint32_t)in_buffer[30]) << 16) |
                            (((uint32_t)in_buffer[31]) << 24);

    uint32_t actual_crc = crc32_calculate_ieee(in_buffer, 28, 0xFFFFFFFFUL);
    if (actual_crc != expected_crc) {
        return SafetyValidationResult::ERR_CRC32_MISMATCH;
    }

    // Unpack Header
    out_frame.preamble = in_buffer[0];
    out_frame.version = in_buffer[1];
    out_frame.sender_id = ((uint16_t)in_buffer[2]) | (((uint16_t)in_buffer[3]) << 8);
    out_frame.target_id = ((uint16_t)in_buffer[4]) | (((uint16_t)in_buffer[5]) << 8);
    out_frame.sequence_number = ((uint32_t)in_buffer[6]) |
                                (((uint32_t)in_buffer[7]) << 8) |
                                (((uint32_t)in_buffer[8]) << 16) |
                                (((uint32_t)in_buffer[9]) << 24);
    out_frame.session_id = ((uint16_t)in_buffer[10]) | (((uint16_t)in_buffer[11]) << 8);
    out_frame.frame_type = (FrameType)in_buffer[12];
    out_frame.payload_length = in_buffer[13];
    out_frame.crc32 = expected_crc;

    // Unpack Payload
    out_frame.payload.buttons_primary = ((uint16_t)in_buffer[14]) | (((uint16_t)in_buffer[15]) << 8);
    out_frame.payload.buttons_complement = ((uint16_t)in_buffer[16]) | (((uint16_t)in_buffer[17]) << 8);
    out_frame.payload.active_unit = in_buffer[18];
    out_frame.payload.status_flags = in_buffer[19];
    out_frame.payload.battery_mv = ((uint16_t)in_buffer[20]) | (((uint16_t)in_buffer[21]) << 8);
    out_frame.payload.diagnostic_flags = ((uint16_t)in_buffer[22]) | (((uint16_t)in_buffer[23]) << 8);
    out_frame.payload.spare1 = ((uint16_t)in_buffer[24]) | (((uint16_t)in_buffer[25]) << 8);
    out_frame.payload.spare2 = ((uint16_t)in_buffer[26]) | (((uint16_t)in_buffer[27]) << 8);

    // 4. Sender ID Check
    if (expected_sender_id != 0xFFFF && out_frame.sender_id != expected_sender_id) {
        return SafetyValidationResult::ERR_SENDER_ID_MISMATCH;
    }

    // 5. Target ID Check
    if (expected_target_id != 0xFFFF && out_frame.target_id != expected_target_id) {
        return SafetyValidationResult::ERR_TARGET_ID_MISMATCH;
    }

    // 6. Session ID Check
    if (expected_session_id != 0 && out_frame.session_id != expected_session_id) {
        return SafetyValidationResult::ERR_SESSION_ID_MISMATCH;
    }

    // 7. Monotonic Sequence Ordering Check
    if (last_valid_sequence > 0) {
        if (out_frame.sequence_number <= last_valid_sequence) {
            return SafetyValidationResult::ERR_SEQUENCE_STALE_OR_REPLAY;
        }
        if (max_sequence_gap > 0 && (out_frame.sequence_number - last_valid_sequence) > max_sequence_gap) {
            return SafetyValidationResult::ERR_SEQUENCE_STALE_OR_REPLAY;
        }
    }

    // 8. Payload Length Check
    if (out_frame.payload_length != sizeof(ApplicationPayload)) {
        return SafetyValidationResult::ERR_PAYLOAD_LENGTH_INVALID;
    }

    // 9. Complementary Dual-Rail Buttons Check
    if (!verify_buttons(out_frame.payload)) {
        return SafetyValidationResult::ERR_COMPLEMENTARY_BIT_FAULT;
    }

    // 10. Frame Type ESTOP check
    if (out_frame.frame_type == FrameType::ESTOP_ACTIVE) {
        return SafetyValidationResult::ERR_ESTOP_TRIPPED;
    }

    return SafetyValidationResult::VALID;
}

} // namespace IndustrialSafety::Safety
