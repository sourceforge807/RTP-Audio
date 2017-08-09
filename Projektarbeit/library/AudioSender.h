#ifndef __Audio_Sender_H__
#define __Audio_Sender_H__
#endif

#include <Arduino.h>

#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <w5100.h>

byte            byteRTPSendRecordedBuffer[524]        	= "";
uint16_t		intMulticastPort						= 0;


//namespace
File data;
EthernetUDP Udp;

//void sendRecorded(byte *byteTeensyMac, byte *byteStaticTeensyIP, byte *byteMulticastPseudoMac, byte *byteReceiverMulticastIP, uint16_t const* intLocalMulticastPort, uint8_t *intSamplingStatusMode)
void sendRecorded(byte *byteTeensyMac, byte *byteStaticTeensyIP, byte *byteMulticastPseudoMac, byte *byteReceiverMulticastIP, uint16_t const* intLocalMulticastPort, uint8_t *intSamplingStatusMode)
{
	Serial.print("Begin sending: ");
	intMulticastPort = (*intLocalMulticastPort);
	if (SD.exists("RECORD.RAW"))
	{
		data = SD.open("RECORD.RAW", FILE_WRITE);
		Serial.print("\nFile found");
		Serial.print("\nFilesize: ");
		Serial.print(data.size() / 1024);
		Serial.print("kbyte");
    	//SD.remove("RECORD.RAW");
  	}
	  	
	
  	if(data)
  	{
  		
  		// start the Ethernet connection:
  		Serial.println("\nInitialize Ethernet without DHCP:");
  		Ethernet.begin(byteTeensyMac, byteStaticTeensyIP);

  		Serial.print("Assigned IP: ");
  		//Serial.println(byteStaticTeensyIP);
  		Serial.println(Ethernet.localIP());
  		// give the Ethernet shield a second to initialize:
  		delay(1000);
  		Udp.beginMulticast(Ethernet.localIP(), intMulticastPort);
  		delay(1000);
  		


  		    for(uint32_t f = 0; f < 524; f++)
    		{
      			
      			Serial.print(f);
      			delay(25);
    		}
    		Serial.print("\n");

  		
  		/********************************************************************************
		 * possible way to send data over udp rtp										*
		 * error in audio memory queue1.read() after initializing Ethernet.begin()		*
		 * 																				*
		 ********************************************************************************/
		/*
  		while(sizeof(data.read()) >= 1)
  		{
  			byte buf;
  			data.read(&buf, 524);
  			
  			
  			//for(uint32_t f = 0; f < 524; f++)
    		//{
      		//	Serial.print(buf, BIN);
      		//	Serial.print(".");
      		//	delay(1000);
    		//}
    		//Serial.print("\n");
			
			
  			Udp.beginPacket(byteReceiverMulticastIP, intMulticastPort);
    		Udp.write(buf, 524);
    		Udp.endPacket(); 
		}
		*/
		data.close();
    	Serial.print("Sending end.\n");
		//Serial.print(*intSamplingStatusMode);
   	}
   	*intSamplingStatusMode = 0;
	
}
