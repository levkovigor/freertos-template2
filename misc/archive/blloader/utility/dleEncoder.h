#ifndef __DLEENCODER_H_
#define __DLEENCODER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum dle_returncodes {
DLE_OK = 0,
STREAM_TOO_SHORT = 1,
DECODING_ERROR = 2
} dle_returncodes_t;

/**
 * @brief   This DLE Encoder (Data Link Encoder) can be used to encode and
 *          decode arbitrary data with ASCII control characters
 * @details
 * List of control codes:
 * https://en.wikipedia.org/wiki/C0_and_C1_control_codes
 *
 * This encoder can be used to achieve a basic transport layer when using
 * char based transmission systems.
 * The passed source strean is converted into a encoded stream by adding
 * a STX marker at the start of the stream and an ETX marker at the end of
 * the stream. Any STX, ETX, DLE and CR occurences in the source stream are
 * escaped by a DLE character. The encoder also replaces escaped control chars
 * by another char, so STX, ETX and CR should not appear anywhere in the actual
 * encoded data stream.
 *
 * When using a strictly char based reception of packets enoded with DLE,
 * STX can be used to notify a reader that actual data will start to arrive
 * while ETX can be used to notify the reader that the data has ended.
 */


//! Start Of Text character. First character is encoded stream
static const uint8_t STX_CHAR = 0x02;
//! End Of Text character. Last character in encoded stream
static const uint8_t ETX_CHAR = 0x03;
//! Data Link Escape character. Used to escape STX, ETX and DLE occurences
//! in the source stream.
static const uint8_t DLE_CHAR = 0x10;
static const uint8_t CARRIAGE_RETURN = 0x0D;

/**
 * Encodes the give data stream by preceding it with the STX marker
 * and ending it with an ETX marker. STX, ETX and DLE characters inside
 * the stream are escaped by DLE characters and also replaced by adding
 * 0x40 (which is reverted in the decoing process).
 * @param sourceStream
 * @param sourceLen
 * @param destStream
 * @param maxDestLen
 * @param encodedLen
 * @param addStxEtx
 * Adding STX and ETX can be omitted, if they are added manually.
 * @return
 */
dle_returncodes_t encode_dle(const uint8_t *sourceStream, size_t sourceLen,
		uint8_t *destStream, size_t maxDestLen, size_t *encodedLen,
		bool addStxEtx);

/**
 * Converts an encoded stream back.
 * @param sourceStream
 * @param sourceStreamLen
 * @param readLen
 * @param destStream
 * @param maxDestStreamlen
 * @param decodedLen
 * @return
 */
dle_returncodes_t decode_dle(const uint8_t *sourceStream,
		size_t sourceStreamLen, size_t *readLen, uint8_t *destStream,
		size_t maxDestStreamlen, size_t *decodedLen);

#endif /* __DLEENCODER_H_ */
