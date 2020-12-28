#include "lz77.h"

#define LZ77_MATCH_LENGTH_BITS 5

#define LZ77_MATCH_OFFSET_BITS (16 - LZ77_MATCH_LENGTH_BITS)

#if LZ77_MATCH_OFFSET_BITS + LZ77_MATCH_LENGTH_BITS != 16
    #error LZ77_MATCH_OFFSET_BITS + LZ77_MATCH_LENGTH_BITS must equal 16
#endif
#if LZ77_MATCH_LENGTH_BITS > 8 || LZ77_MATCH_LENGTH_BITS < 2
    #error LZ77_MATCH_LENGTH_BITS must be in 2-8 range
#endif

static const uint16_t WindowSize = (1 << LZ77_MATCH_OFFSET_BITS) - 1;
static const uint16_t MatchMaxLenght = (1 << LZ77_MATCH_LENGTH_BITS) - 1; // also used as bitmask for extracting length from match

static uint16_t findMatch(uint8_t *input, uint32_t current_position, uint32_t input_length) {
    const uint32_t WindowStartPosition = WindowSize > current_position ? 0 : current_position - WindowSize;
    uint32_t best_match_position = 0;
    uint16_t best_match_length = 0;
    uint32_t max_length = input_length - current_position;
    if (max_length > MatchMaxLenght) {
        max_length = MatchMaxLenght;
    }

    if (current_position > 0) {
        for (uint32_t match_position = WindowStartPosition; match_position < current_position; match_position++) {
            uint16_t match_length = 0;
            for (; match_length < max_length; match_length++) {
                if (input[match_position + match_length] != input[current_position + match_length]) {
                    break;
                }
            }
            if (match_length > best_match_length) {
                // better match found
                best_match_length = match_length;
                best_match_position = match_position;
            }
        }
    }

    // pack result
    if (best_match_length < 2) {
        // no match or just one element match found - literal reference
        return (uint16_t)(input[current_position] << 8);
    }
    else {
        // valid match found - offset and length reference
        uint16_t offset = current_position - best_match_position;
        return (uint16_t)(offset << LZ77_MATCH_LENGTH_BITS | (best_match_length & MatchMaxLenght));
    }
}

uint32_t lz77_compress(uint8_t *input, uint32_t input_length, uint8_t *output, uint32_t output_limit) {
    uint32_t input_cursor = 0, output_cursor = 0;
    uint8_t output_bitmask = 0;
    uint8_t *ouptut_literal_bitmask;

    while (input_cursor < input_length) {
        if (output_bitmask == 0) {
            output_bitmask = 1;
            ouptut_literal_bitmask = &output[output_cursor++];
            *ouptut_literal_bitmask = 0;
        }
        uint16_t match = findMatch(input, input_cursor, input_length);

        uint8_t h = match >> 8;
        uint8_t l = match & 0xFF;

        uint16_t consumed = match & MatchMaxLenght;
        if (consumed == 0) {
            // literal match
            consumed++;
            if (output_cursor < output_limit) {
                output[output_cursor++] = h;
            }
            else {
                output_cursor++;
            }
            // mark literal match bit
            *ouptut_literal_bitmask |= output_bitmask;
        }
        else {
            // window match
            if (output_cursor < output_limit - 1) {
                output[output_cursor++] = h;
                output[output_cursor++] = l;
            }
            else {
                output_cursor += 2;
            }
        }
        input_cursor += consumed;
        output_bitmask <<= 1;
    }
    return output_cursor;
}

uint32_t lz77_decompress(uint8_t *input, uint32_t input_length, uint8_t *output, uint32_t output_limit) {
    uint32_t output_cursor = 0;
    uint8_t input_literal_bitmask = input[0];
    uint8_t input_bitmask = 0;

    for (uint32_t input_cursor = 0; input_cursor < input_length;) {
        if (input_bitmask == 0) {
            input_bitmask = 1;
            input_literal_bitmask = input[input_cursor++];
        }

        if (input_literal_bitmask & input_bitmask) {
            // literal match
            if (input_cursor < input_length && output_cursor < output_limit) {
                output[output_cursor++] = input[input_cursor++];
            }
            else {
                output_cursor++;
                input_cursor++;
            }
        }
        else {
            // window match
            if (input_cursor < input_length - 1) {
                uint8_t h = input[input_cursor++];
                uint8_t l = input[input_cursor++];

                uint16_t match = (h << 8) | l;
                uint16_t length = match & MatchMaxLenght;

                uint16_t offset = match >> LZ77_MATCH_LENGTH_BITS;
                for (uint16_t i = 0; i < length; i++) {
                    if (output_cursor < output_limit) {
                        output[output_cursor] = output[output_cursor - offset];
                    }
                    output_cursor++;
                }
            }
            else {
                input_cursor += 2;
            }
        }

        input_bitmask <<= 1;
    }
    return output_cursor;
}
