#ifndef _H_SAFETY_CRC32
#define _H_SAFETY_CRC32 1

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate IEEE 802.3 CRC-32 (Polynomial: 0xEDB88320)
 * @param data Pointer to input data buffer
 * @param length Number of bytes
 * @param initial_crc Initial CRC value (defaults to 0xFFFFFFFF)
 * @return Calculated 32-bit CRC with final XOR
 */
uint32_t crc32_calculate_ieee(const uint8_t* data, size_t length, uint32_t initial_crc);

/**
 * @brief Run CRC-32 self-test against standard test vector "123456789"
 * @return 1 (true) if self-test passes, 0 (false) otherwise
 */
uint8_t crc32_self_test(void);

#ifdef __cplusplus
}

namespace IndustrialSafety::Safety {
class Crc32 {
public:
    static inline uint32_t calculate_ieee(const uint8_t* data, size_t length, uint32_t initial_crc = 0xFFFFFFFFU) {
        return crc32_calculate_ieee(data, length, initial_crc);
    }
    static inline bool self_test() {
        return crc32_self_test() != 0;
    }
};
}
#endif

#endif // _H_SAFETY_CRC32
