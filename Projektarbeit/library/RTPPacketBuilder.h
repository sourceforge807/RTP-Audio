#ifndef RTPPacketBuilder_h
#define RTPPacketBuilder_h
#endif

const uint8_t 		intRTPVersion		= 2;			// 2bit
const uint8_t		intRTPPadding		= 0;			// 1bit
const uint8_t		intRTPExtension		= 0;			// 1bit
const uint8_t		intRTPCSRCCount		= 0;			// 4bit, set by multiplexer -> RFC3550
const uint8_t		intRTPMarker		= 0;			// 1bit
const uint8_t		intRTPPayloadType	= 10;			// 7bit, PT=10(DEZ), BIN = B0001010, encoding name=L16, media type=A (Audio), clock rate=44,100Hz, channels=2 -> RFC3551#section 6
const uint32_t		intRTPSSRC			= 0xE504D730;	// temp, const or define? DEC = 3842299696, HEX = 0xE5.04.D7.30, BIN = 1110.0101.0000.0100.1101.0111.0011.0000, from Teensy mac


//byte 				byteBuff[524];
/* 
 * uint16_t sequenceNumber 16bit, initial value is should be random
 * uint32_t timestamp 32bit, initial value should be random, 	for fixed-rate audio the timestamp clock would likely increment by one for each sampling period.
 * 																If an audio application reads blocks covering 160 sampling periods from the input device, the timestamp would be
 *																increased by 160 for each such block (160 for 8kHz audio)
 * uint32_t ssrc 32bit, identifier should be chosen randomly (set by Teensy mac)
 * CSRC list[0-15] 32bits each, CSRC identifiers are inserted by mixers -> see CSRC count = 0 (CSRC list optional)
 */

int RTPSSRC(void)
{
	return intRTPSSRC;
}


void RTPHeaderBuilder(byte *byteRTPHeader, uint64_t *intRTPSequenceNumber, uint32_t *intRTPTimestamp)
{
	uint32_t SSRC = RTPSSRC(); // why errorfree over funktion?
	
 	// header begin...
	byteRTPHeader[0] = (intRTPVersion << 6 | intRTPPadding << 5 | intRTPExtension << 4 | intRTPCSRCCount); // headerbyte 0

	byteRTPHeader[1] = (intRTPMarker << 7 | intRTPPayloadType); // headerbyte 1

	// uint16_t sequence number
	byteRTPHeader[2] = (*intRTPSequenceNumber)>> 8;
	byteRTPHeader[3] = (*intRTPSequenceNumber);

	// uint32_t timestamp
	byteRTPHeader[4] = (*intRTPTimestamp) >> 24;
	byteRTPHeader[5] = (*intRTPTimestamp) >> 16;
	byteRTPHeader[6] = (*intRTPTimestamp) >> 8;
	byteRTPHeader[7] = (*intRTPTimestamp);
	//Serial.printf("%lld\n", (*intRTPTimestamp));

	// uint32_t ssrc
	byteRTPHeader[8] = SSRC >> 24;
	byteRTPHeader[9] = SSRC >> 16;
	byteRTPHeader[10] = SSRC >> 8;
	byteRTPHeader[11] = SSRC;

/*
	// uint32_t ssrc
	byteRTPHeader[8] = intRTPSSRC >> 24;
	byteRTPHeader[9] = intRTPSSRC >> 16;
	byteRTPHeader[10] = intRTPSSRC >> 8;
	byteRTPHeader[11] = intRTPSSRC;
*/

    if(*intRTPSequenceNumber >= 65535) // max possible sequence number
    {
    	*intRTPSequenceNumber = 0;
	}
    *intRTPSequenceNumber += 1;
	
	if(*intRTPTimestamp >= 4294967295) // max uin64_t
	{
		*intRTPTimestamp = 0;
	}
	*intRTPTimestamp += 256; // https://stackoverflow.com/questions/6255988/how-to-calculate-effective-time-offset-in-rtp , increment by 256 because 512 samples L/R -> 256 sample time units
}
