#include <blloader/utility/dleEncoder.h>

dle_returncodes_t encode_dle(const uint8_t* sourceStream,
		size_t sourceLen, uint8_t* destStream, size_t maxDestLen,
		size_t* encodedLen, bool addStxEtx) {
	size_t encodedIndex = 0, sourceIndex = 0;
	uint8_t nextByte = 0;

	if (maxDestLen < 2) {
		return STREAM_TOO_SHORT;
	}
	if (addStxEtx) {
		destStream[0] = STX_CHAR;
		++encodedIndex;
	}

	while (encodedIndex < maxDestLen && sourceIndex < sourceLen)
	{
		nextByte = sourceStream[sourceIndex];
		// STX, ETX and CR characters in the stream need to be escaped with DLE
		if (nextByte == STX_CHAR || nextByte == ETX_CHAR) {
			if (encodedIndex + 1 >= maxDestLen) {
				return STREAM_TOO_SHORT;
			}
			else {
				destStream[encodedIndex] = DLE_CHAR;
				++encodedIndex;
				/* Escaped byte will be actual byte + 0x40. This prevents
				 * STX, ETX, and carriage return characters from appearing
				 * in the encoded data stream at all, so when polling an
				 * encoded stream, the transmission can be stopped at ETX.
				 * 0x40 was chosen at random with special requirements:
				 *  - Prevent going from one control char to another
				 *  - Prevent overflow for common characters */
				destStream[encodedIndex] = nextByte + 0x40;
			}
		}
		// DLE characters are simply escaped with DLE.
		else if (nextByte == DLE_CHAR) {
			if (encodedIndex + 1 >= maxDestLen) {
			    return STREAM_TOO_SHORT;
			}
			else {
				destStream[encodedIndex] = DLE_CHAR;
				++encodedIndex;
				destStream[encodedIndex] = DLE_CHAR;
			}
		}
		else {
			destStream[encodedIndex] = nextByte;
		}
		++encodedIndex;
		++sourceIndex;
	}

	if (sourceIndex == sourceLen && encodedIndex < maxDestLen) {
		if (addStxEtx) {
			destStream[encodedIndex] = ETX_CHAR;
			++encodedIndex;
		}
		*encodedLen = encodedIndex;
		return DLE_OK;
	}
	else {
		return STREAM_TOO_SHORT;
	}
}

dle_returncodes_t decode_dle(const uint8_t *sourceStream,
		size_t sourceStreamLen, size_t *readLen, uint8_t *destStream,
		size_t maxDestStreamlen, size_t *decodedLen) {
	size_t encodedIndex = 0, decodedIndex = 0;
	uint8_t nextByte = 0;

	if (*sourceStream != STX_CHAR) {
		return DECODING_ERROR;
	}
	++encodedIndex;

	while ((encodedIndex < sourceStreamLen) && (decodedIndex < maxDestStreamlen)
			&& (sourceStream[encodedIndex] != ETX_CHAR)
			&& (sourceStream[encodedIndex] != STX_CHAR)) {
		if (sourceStream[encodedIndex] == DLE_CHAR) {
			nextByte = sourceStream[encodedIndex + 1];
			// The next byte is a DLE character that was escaped by another
			// DLE character, so we can write it to the destination stream.
			if (nextByte == DLE_CHAR) {
				destStream[decodedIndex] = nextByte;
			}
			else {
			    /* The next byte is a STX, DTX or 0x0D character which
			     * was escaped by a DLE character. The actual byte was
				 * also encoded by adding + 0x40 to preven having control chars,
				 * in the stream at all, so we convert it back. */
				if (nextByte == 0x42 || nextByte == 0x43 || nextByte == 0x4D) {
					destStream[decodedIndex] = nextByte - 0x40;
				}
				else {
				    return DECODING_ERROR;
				}
			}
			++encodedIndex;
		}
		else {
			destStream[decodedIndex] = sourceStream[encodedIndex];
		}

		++encodedIndex;
		++decodedIndex;
	}

	if (sourceStream[encodedIndex] != ETX_CHAR) {
		return DECODING_ERROR;
	}
	else {
		*readLen = ++encodedIndex;
		*decodedLen = decodedIndex;
		return DLE_OK;
	}
}

