/**
 * library for building the rtp header
 *
 * @description		this library build the rtp header with a given array
 * @function 		RTPSSRC(void)
 * @function		RTPHeaderBuilder(byte *byteRTPHeader, uint64_t *intRTPSequenceNumber, uint32_t *intRTPTimestamp)
 */
#ifndef RTPPacketBuilder_h
#define RTPPacketBuilder_h
#endif

// const
const uint8_t 		intRTPVersion		= 2;			// 2bit
const uint8_t		intRTPPadding		= 0;			// 1bit
const uint8_t		intRTPExtension		= 0;			// 1bit
const uint8_t		intRTPCSRCCount		= 0;			// 4bit, set by multiplexer -> RFC3550
const uint8_t		intRTPMarker		= 0;			// 1bit
const uint8_t		intRTPPayloadType	= 10;			// 7bit, PT=10(DEZ), BIN = B0001010, encoding name=L16, media type=A (Audio), clock rate=44,100Hz, channels=2 -> RFC3551#section 6
const uint32_t		intRTPSSRC			= 0xE504D730;	// temp, const or define? DEC = 3842299696, HEX = 0xE5.04.D7.30, BIN = 1110.0101.0000.0100.1101.0111.0011.0000, from Teensy mac

/** 
 * uint16_t sequenceNumber 16bit, initial value is should be random
 * uint32_t timestamp 32bit, initial value should be random, 	for fixed-rate audio the timestamp clock would likely increment by one for each sampling period.
 * 																If an audio application reads blocks covering 160 sampling periods from the input device, the timestamp would be
 *																increased by 160 for each such block (160 for 8kHz audio)
 * uint32_t ssrc 32bit, identifier should be chosen randomly (set by Teensy mac)
 * CSRC list[0-15] 32bits each, CSRC identifiers are inserted by mixers -> see CSRC count = 0 (CSRC list optional)
 */


/**
 * RTP Synchronization Source
 *
 * @description		read the Synchronization Source
 * @param 			void
 * @return			intRTPSSRC
 */
int RTPSSRC(void)
{
	return intRTPSSRC;
}

/**
 * RTP Header Builde
 *
 * @description		build the rtp header in the first 12 bytes from the buffer; using bitshift an bitlogic
 * @param 			*byteRTPHeader
 * @param 			*intRTPSequenceNumber
 * @param 			*intRTPTimestamp
 * @return			-
 */
void RTPHeaderBuilder(byte *byteRTPHeader, uint64_t *intRTPSequenceNumber, uint32_t *intRTPTimestamp)
{
	uint32_t intSSRC = RTPSSRC(); // why errorfree with using the funktion?
	
    if(*intRTPSequenceNumber >= 65535) // max possible sequence number
    {
    	*intRTPSequenceNumber = 0;
	}
	
	if(*intRTPTimestamp >= 4294967295) // max uin64_t
	{
		*intRTPTimestamp = 0;
	}
		
 	// header begin...
	byteRTPHeader[0] 	= (intRTPVersion << 6 | intRTPPadding << 5 | intRTPExtension << 4 | intRTPCSRCCount);

	byteRTPHeader[1] 	= (intRTPMarker << 7 | intRTPPayloadType);

	// uint16_t sequence number
	byteRTPHeader[2] 	= (*intRTPSequenceNumber)>> 8;
	byteRTPHeader[3] 	= (*intRTPSequenceNumber);

	// uint32_t timestamp
	byteRTPHeader[4] 	= (*intRTPTimestamp) >> 24;
	byteRTPHeader[5] 	= (*intRTPTimestamp) >> 16;
	byteRTPHeader[6] 	= (*intRTPTimestamp) >> 8;
	byteRTPHeader[7] 	= (*intRTPTimestamp);

	// uint32_t ssrc
	byteRTPHeader[8] 	= intSSRC >> 24;
	byteRTPHeader[9] 	= intSSRC >> 16;
	byteRTPHeader[10] 	= intSSRC >> 8;
	byteRTPHeader[11] 	= intSSRC;

    *intRTPSequenceNumber += 1;
	*intRTPTimestamp += 128; // https://stackoverflow.com/questions/6255988/how-to-calculate-effective-time-offset-in-rtp , increment by 128 because 512/2 samples L/R -> 256 sample time units
}
