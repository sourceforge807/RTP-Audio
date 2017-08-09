#ifndef __Audio_Recorder_H__
#define __Audio_Recorder_H__
#endif

#include <Audio.h>
#include <Wire.h>
#include <SerialFlash.h>
#include <time.h>



// GUItool: begin automatically generated code
AudioInputI2S            i2s1;
AudioRecordQueue         queue1;
AudioMixer4              mixer1;
AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 1, mixer1, 1);
AudioConnection          patchCord3(mixer1, queue1);
AudioControlSGTL5000     sgtl5000_1;
// GUItool: end automatically generated code


uint8_t         intSamplingStatusMode         	= 0;  // Remember which mode we're doing 0=stopped, 1=recording, 2=sending
uint8_t			intEthernetHasInitialized		= 0;
byte            byteRTPSendBuffer[524]        	= "";
uint32_t        intSampleTime                 	= 0;
uint64_t        intRTPSequenceNumber          	= 0;
byte            *bytePointerRTPSendBuffer     	= &byteRTPSendBuffer[0];
uint32_t        *intPointerSampleTime         	= &intSampleTime;
uint64_t        *intPointerRTPSequenceNumber  	= &intRTPSequenceNumber;
const uint16_t  intMulticastPort             	= 60000;
//const uint16_t 	*intPointerMulticastPort = &intMulticastPort;

IPAddress       byteStaticTeensyIP(192, 168, 1, 25);
//byte			byteStaticTeensyIP[4];
byte            *bytePointerStaticTeensyIP		= &byteStaticTeensyIP[0];
IPAddress       byteReceiverMulticastIP(239, 0, 0, 0); // [RFC1112,JBP]
//byte			byteReceiverMulticastIP[4];
byte            *bytePointerReceiverMulticastIP	= &byteReceiverMulticastIP[0];
byte			byteSerialMonitor[12];
char			charTimeBuffer[80];

byte ii[256];

//namespace
//File frec;
EthernetUDP Udp;

void startRecording(uint8_t *intPointerEthernetHasInitialized)
{
	Serial.println("Recording started...\n");
  	/*
	if (SD.exists("RECORD.RAW"))
	{
		Serial.println("Remove RECORD.RAW");
    	SD.remove("RECORD.RAW");
  	}
  	
  	frec = SD.open("RECORD.RAW", FILE_WRITE);
  	if(frec)
  	{
    	queue1.begin();
    	intSamplingStatusMode = 1;
   	}
   	*/
   	queue1.begin();
   	if(intPointerEthernetHasInitialized == 1)
   	{
	    intSamplingStatusMode = 2;
	}
	else
	{
		intSamplingStatusMode = 1;
	}
}


void stopRecording()
{
	queue1.end();
  	if (intSamplingStatusMode == 1 || intSamplingStatusMode == 2)
  	{
    	while (queue1.available() > 0)
    	{
      		//frec.write((byte*)queue1.readBuffer(), 256);
      		queue1.clear();
			//queue1.freeBuffer();
      		Serial.println("Free buffer...");
    	}
    	//frec.close();
  	}
  	intSamplingStatusMode = 0;
  	Serial.println("Recording stopped.");
}


void continueRecording(uint8_t *intPointerEthernetHasInitialized)
//void continueRecording()
{
   	//Serial.printf("%lld\n", intRTPSequenceNumber);
   	RTPHeaderBuilder(bytePointerRTPSendBuffer, intPointerRTPSequenceNumber, intPointerSampleTime); // build RTP packet header
    
   	// Fetch 2 blocks from the audio library and copy into a 524 byte RTPSendBuffer (12 byte RTP header + 512 byte data). Max 4096KByte buffer by Wiz820IO
   	memcpy(byteRTPSendBuffer, bytePointerRTPSendBuffer, 12);
   	memcpy(byteRTPSendBuffer+12, queue1.readBuffer(), 256);
   	queue1.freeBuffer();
   	memcpy(byteRTPSendBuffer+268, queue1.readBuffer(), 256);
   	queue1.freeBuffer();

   	//frec.write(byteRTPSendBuffer, 524);
    	
	if(intEthernetHasInitialized == 1)
	{
		Udp.beginPacket(bytePointerReceiverMulticastIP, intMulticastPort);
   		Udp.write(byteRTPSendBuffer, 524);
   		Udp.endPacket();
	}
		

	uint16_t		intShowRTPSequenceNumber			= (byteRTPSendBuffer[2] << 8 | byteRTPSendBuffer[3]);
	uint32_t		intShowRTPTimestamp					= (byteRTPSendBuffer[4] << 24 | byteRTPSendBuffer[5] << 16 | byteRTPSendBuffer[6] << 8 |byteRTPSendBuffer[7]);
	uint32_t		intShowRTPSSRC						= (byteRTPSendBuffer[8] << 24 | byteRTPSendBuffer[9] << 16 | byteRTPSendBuffer[10] << 8 | byteRTPSendBuffer[11]);
	uint16_t		intShowFirstBlockLeft 				= (byteRTPSendBuffer[12] << 8 | byteRTPSendBuffer[13]);
	uint16_t		intShowFirstBlockRight 				= (byteRTPSendBuffer[14] << 8 | byteRTPSendBuffer[15]);
	uint16_t		intShowSecondBlockLeft 				= (byteRTPSendBuffer[16] << 8 | byteRTPSendBuffer[17]);
	uint16_t		intShowSecondBlockRight 			= (byteRTPSendBuffer[18] << 8 | byteRTPSendBuffer[19]);
	uint16_t		intShowThirdBlockLeft 				= (byteRTPSendBuffer[16] << 8 | byteRTPSendBuffer[20]);
	uint16_t		intShowThirdBlockRight 				= (byteRTPSendBuffer[18] << 8 | byteRTPSendBuffer[21]);

		
		
		
	Serial.print("2Bit Version, 1Bit Padding, 1Bit Extension, 4Bit CSRC Count: ");
	Serial.println(byteRTPSendBuffer[0], BIN);
	Serial.print("1Bit Marker, 7Bit Payload Type: ");
	Serial.println(byteRTPSendBuffer[1], BIN);
	Serial.print("16Bit inkrement Sequence Number: ");
	Serial.println(intShowRTPSequenceNumber);
	Serial.print("32Bit Timestamp (in samplerate units): ");
	Serial.print(intShowRTPTimestamp);
	Serial.print("   ");
	Serial.print("Timestamp to string: ");
	Serial.print(hour(intShowRTPTimestamp));
	Serial.print(":");
	Serial.print(minute(intShowRTPTimestamp));
	Serial.print(":");
	Serial.print(hour(intShowRTPTimestamp));
	Serial.print("   ");
	Serial.print(day(intShowRTPTimestamp));
	Serial.print(".");
	Serial.print(month(intShowRTPTimestamp));
	Serial.print(".");
	Serial.println(year(intShowRTPTimestamp));
	Serial.print("32Bit SSRC(Synchronization Source (SSRC) identifier)");
	Serial.print(intShowRTPSSRC, HEX);
	Serial.print("  >>  ");
	Serial.print(byteRTPSendBuffer[8], HEX);
	Serial.print(".");
	Serial.print(byteRTPSendBuffer[9], HEX);
	Serial.print(".");
	Serial.print(byteRTPSendBuffer[10], HEX);
	Serial.print(".");
	Serial.println(byteRTPSendBuffer[11], HEX);
	Serial.print("Example of 3 sample time units (4 blocks of 8Bit = 1 sample time unit): ");
	Serial.print(intShowFirstBlockLeft, BIN);
	Serial.print(".");
	Serial.print(intShowFirstBlockRight, BIN);
	Serial.print(".");
	Serial.print(intShowSecondBlockLeft, BIN);
	Serial.print(".");
	Serial.print(intShowSecondBlockRight, BIN);
	Serial.print(".");
	Serial.print(intShowThirdBlockLeft, BIN);
	Serial.print(".");
	Serial.print(intShowThirdBlockRight, BIN);
	Serial.print("\n\n");
	delay(50);

	/*
   	for(uint32_t f = 0; f < 524; f++)
   	{
   		Serial.print(byteRTPSendBuffer[f], BIN);
   		Serial.print(".");
   	}
   	*/
}
