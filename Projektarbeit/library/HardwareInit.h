#ifndef __Hardware_Init_H__
#define __Hardware_Init_H__
#endif

#include <Arduino.h>
#include <SD.h>
#include <TeensyID.h>
#include <string>
#include <Time.h>
#include <TimeLib.h>

// defines
#define TIME_HEADER  "T"

// const
const uint32_t 	intSerialCommunicationSpeed 		= 115200;
const uint16_t 	intFunctionWaitDelay 				= 250;
const uint8_t	intSDCardAudioChipSelect 			= 10;
const uint8_t	intEthernetChipSelect				= 20;
const uint8_t 	intSdCardEthernetChipSelect			= 21;
const uint8_t 	intResetPinEthernetChip				= 38; // WIZ820io nReset Pin

// variables init
uint8_t 	serial[4];
uint8_t 	mac[6];
uint32_t 	uid[4];
uint8_t 	uuid[16];
uint8_t 	intErrorTimeInit 			= 0;
uint8_t 	intErrorSdCardInit 			= 0;
uint8_t 	intSerialCommunikationInit 	= 0;
// led without delay begin
uint8_t		intLedBlinkState = LOW;             // ledState used to set the LED
uint32_t	intLedBlinkTimerPreviousMillis 	= 0;        // will store last time LED was updated

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
uint32_t	intLedBlinkTimerInterval 		= 1000;           // interval at which to blink (milliseconds)
// led without delay end

// deklare namespace
Sd2Card 	card;
SdVolume 	volume;
SdFile 		root;

// deklare funktion
time_t 		getTeensy3Time();
uint32_t 	processSyncMessage();


/* Serial communication init */
void serialCommunicationInit()
{
	Serial.begin(intSerialCommunicationSpeed);
	
	Serial.println("\nInitializing serial kommunikation...");
	Serial.begin(intSerialCommunicationSpeed);
	//while (!Serial);
	delay(100);
	Serial.print("Serial speed is set to: ");
	Serial.print(intSerialCommunicationSpeed);
	Serial.println(" Baud");
	Serial.println("Initialization done.");
	delay(intFunctionWaitDelay);
}
	
/* time init */
void timeInit()
{
	Serial.println("\nInitializing time...");

	
	setSyncProvider(getTeensy3Time);
	
	if (timeStatus() != timeSet)
	{

		Serial.println("Unable to sync with the RTC");
		intErrorTimeInit = 1;
	}
	else
	{
		Serial.print("RTC has set the system time to: ");
		Serial.print(now()-3600);
		Serial.println(" milis since 01.01.1970 UTC +1");
	}

	if (Serial.available())
  	{
		time_t t = processSyncMessage();
 
		if (t != 0)
		{
			Teensy3Clock.set(t);
			setTime(t);
		}
	}

	Serial.println("Initialization done.");
  	delay(intFunctionWaitDelay);
}	

/* SD card init */
void sdCardInit()
{
	// TODO: init other SD Cards
	Serial.println("\nInitializing SD card on audio shield...");

	
	if (!(SD.begin(intSDCardAudioChipSelect)))
	{
		Serial.println("Unable to access the SD card");
		delay(intFunctionWaitDelay);
	}

	if (!card.init(SPI_HALF_SPEED, intSDCardAudioChipSelect))
	{
  		Serial.println("initialization failed. Things to check:");
		Serial.println("* is a card inserted?");
   		Serial.println("* is your wiring correct?");
   		Serial.println("* did you change the chipSelect pin to match your shield or module?");
   	}
	else
	{
   		Serial.println("Wiring is correct and a card is present.");

	}


	Serial.print("Card type: ");
		
	switch(card.type())
	{
 		case SD_CARD_TYPE_SD1:
   			Serial.println("SD1");
   		break;
 		case SD_CARD_TYPE_SD2:
   			Serial.println("SD2");
   		break;
 		case SD_CARD_TYPE_SDHC:
   			Serial.println("SDHC");
   		break;
 		default:
   			Serial.println("Unknown");
   	}


	if (!volume.init(card))
	{
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
	}

	uint32_t volumesize;
	Serial.print("Volume type is FAT");
	Serial.println(volume.fatType(), DEC);

	volumesize = volume.blocksPerCluster();
	volumesize *= volume.clusterCount();
  
	if (volumesize < 8388608ul)
	{
		Serial.print("Volume size (bytes): ");
		Serial.println(volumesize * 512);
	}
  
	Serial.print("Volume size (Kbytes): ");
	volumesize /= 2;
	Serial.println(volumesize);
	Serial.print("Volume size (Mbytes): ");
	volumesize /= 1024;
	Serial.println(volumesize);
  
	Serial.println("Files found on the card (name, date and size in bytes): ");
	root.openRoot(volume);
	root.ls(LS_R | LS_DATE | LS_SIZE);

	Serial.println("Initialization done.");

	delay(intFunctionWaitDelay);
}

	
void teensyInfo(uint8_t intWhichInformation, byte *byteTeensyMac)
{

	Serial.println("\nReading Teensy...");
	teensySN(serial);
	teensyMAC(mac);
	kinetisUID(uid);
	teensyUUID(uuid);
	
	switch(intWhichInformation)
	{
		case 1:
			Serial.println("Reading Serial from hardware...");
		//	Serial.printf("Array Serialnumber: %02X-%02X-%02X-%02X \n", serial[0], serial[1], serial[2], serial[3]);
			Serial.printf("Serialnumber: %s\n", teensySN());
  		break;
  		
  		case 2:
			Serial.println("Reading UUID from hardware...");
		//	Serial.printf("Array 128-bit UUID RFC4122: %02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X\n", uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);
			Serial.printf("128-bit UUID RFC4122: %s\n", teensyUUID());
  		break;
		
		case 3:
			Serial.println("Reading Chip ID from hardware...");
		//	Serial.printf("Array 128-bit UniqueID from chip: %08X-%08X-%08X-%08X\n", uid[0], uid[1], uid[2], uid[3]);
  			Serial.printf("128-bit UniqueID from chip: %s\n", kinetisUID());
  		break;
		
		case 4:
			Serial.println("Reading MAC from hardware...");
		//	Serial.printf("Array MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		  	Serial.printf("MAC Address: %s\n", teensyMAC());
		//  byte teensyMacAddress1[] = {0x04, 0xE9, 0xE5, 0x04, 0xD7, 0x30 };
		//  byte teensyMacAddress2[] = {mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] };
		//  Multicast mac: Der IP-Multicast-Adresse 224.0.0.1 ist somit die Multicast-MAC-Adresse 01-00-5e-00-00-01 fest zugeordnet. -> https://de.wikipedia.org/wiki/MAC-Adresse
  		break;
  		
  		case 5:
  			Serial.println("Reading USB Serialnumber from hardware...");
			Serial.printf("USB Serialnumber: %u \n", teensyUsbSN());
  		break;
  		
  		default:
  			Serial.printf("Serialnumber: %s\n", teensySN());
  			Serial.printf("128-bit UUID RFC4122: %s\n", teensyUUID());
  			Serial.printf("128-bit UniqueID from chip: %s\n", kinetisUID());
  			Serial.printf("MAC Address: %s\n", teensyMAC());
  			Serial.printf("USB Serialnumber: %u \n", teensyUsbSN());
  		break;
  	}
	
	uint8_t i = 0;
	while (mac[i] != 0)
	{
		byteTeensyMac[i] = mac[i];
		i++;
	}
	
	Serial.println("Reading done.");

  	delay(intFunctionWaitDelay);
}

/*string ethernetInit(uint8_t intSerialKommunikationInit)
{
	string IPAddress;
	Ethernet.init(inEthernetChipSelect);
	
	teensyMAC(mac);
		
	if (intSerialKommunikationInit == 1)
	{
		Serial.println("\nInitializing ethernet...");
		//	Serial.printf("Array MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		Serial.printf("MAC Address: %s\n", teensyMAC());
		//Serial.println("\nStarting Network using DHCP.");
		// TODO: init IP	
		Serial.println("Initialization done.");
	}
	
	if (Ethernet.begin(mac) == 0)
	{
		if (intSerialKommunikationInit == 1)
		{
			Serial.println("Failed to configure Ethernet using DHCP, using Static Mode"); // If DHCP Mode failed, start in Static Mode
		}
		IPAddress = "192.168.001.005";
	}
	else
	{
		IPAddress = Ethernet.localIP();
	}
	
	delay(1000);
	return IPAddress;
}*/

// WIZ820io/W5200 must be at least reset to avoid it clashing with SPI
void resetEthernetChip(void)
{
	// TODO: if (intSerialCommunikationInit == 1) for Serial.print
	pinMode(intSDCardAudioChipSelect, INPUT_PULLUP);
	pinMode(intEthernetChipSelect, INPUT_PULLUP);
	pinMode(intSdCardEthernetChipSelect, INPUT_PULLUP);
	pinMode(intResetPinEthernetChip, INPUT_PULLUP);
	delay(intFunctionWaitDelay);  // allow time for pins to reach 3.3V
	
	pinMode(intSDCardAudioChipSelect, OUTPUT);
	pinMode(intEthernetChipSelect, OUTPUT);
	pinMode(intSdCardEthernetChipSelect, OUTPUT);
	pinMode(intResetPinEthernetChip, OUTPUT);
	
	Serial.println("\nBegin reset the WIZ820io:");
	
	Serial.println("De-select WIZ820io");
	digitalWrite(intEthernetChipSelect, HIGH); // de-select WIZ820io
	delay(intFunctionWaitDelay);
  
	Serial.println("De-select the Audio shield SD Card");
	digitalWrite(intSDCardAudioChipSelect, HIGH); // de-select the Audio shield SD Card
	delay(intFunctionWaitDelay);
	
	Serial.println("De-select the Ethernet shield SD Card");
	digitalWrite(intSdCardEthernetChipSelect, HIGH); // de-select the Ethernet shield SD Card
	delay(intFunctionWaitDelay);
	
	Serial.println("Set nReset Pin to low");
	digitalWrite(intResetPinEthernetChip, LOW); // begin reset the WIZ820io
	delay(intFunctionWaitDelay);
	
	Serial.println("End reset pulse");
	digitalWrite(intResetPinEthernetChip, HIGH);// end reset pulse
	delay(intFunctionWaitDelay);

}
  
time_t getTeensy3Time()
{
	return Teensy3Clock.get();
}

uint32_t processSyncMessage()
{
	uint32_t pctime = 0L;
	const uint32_t DEFAULT_TIME = 1357041600; 

	if(Serial.find(TIME_HEADER))
	{
 		pctime = Serial.parseInt();
  		return pctime;
  		if( pctime < DEFAULT_TIME)
  		{
   			pctime = 0L;
  		}
	}
	return pctime;
}

void teensyFreeMemory()
{
	uint8_t *heapptr;
    uint8_t *stackptr;
	unsigned SP = 0;

    stackptr = (uint8_t *)malloc(4);   // use stackptr temporarily
    heapptr = stackptr;                // save value of heap pointer
    free(stackptr);                    // free up the memory again (sets stackptr to 0)
    stackptr =  (uint8_t *)(SP);       // save value of stack pointer


    // print("HP: ");
    Serial.print(PSTR("\nHeap Pointer: "));
    Serial.println((int) heapptr);

    // print("SP: ");
    Serial.print(PSTR("Stack Pointer: "));
    Serial.println((int) stackptr);

    // print("Free: ");
    Serial.print(PSTR("Free Memory: "));
    Serial.println((int) stackptr - (int) heapptr);
    Serial.println();
}

void ledBlink(uint8_t intPinLedSample, uint8_t intSamplingStatusMode)
{
	uint32_t intLedBlinkTimerCurrentMillis = millis();
	if (intSamplingStatusMode == 1)
	{
		if(intLedBlinkTimerCurrentMillis - intLedBlinkTimerPreviousMillis > intLedBlinkTimerInterval)
		{
			intLedBlinkTimerPreviousMillis = intLedBlinkTimerCurrentMillis; // save the last time you blinked the LED

			// if the LED is off turn it on and toggle:
			if (intLedBlinkState == LOW)
			{
				intLedBlinkState = HIGH;
			}
			else
			{
				intLedBlinkState = LOW;
			}
			
			digitalWrite(intPinLedSample, intLedBlinkState);// set the LED with the ledState of the variable:
		}
	}
	else
	{
		intLedBlinkState = LOW;
	}
}

/*
void SGTLinit()
{
	// audio setup begin
	Serial.println("\nInitializing SGTL...");
	AudioMemory(100); // Audio connections require memory, and the record queue. uses this memory to buffer incoming audio.
  
	// Enable the audio shield, select input, and enable output
	uint8_t boolSGTLEnable = 0;
	boolSGTLEnable = sgtl5000_1.enable();
	if(boolSGTLEnable == 1)
	{
		Serial.println("\nSGTL enable");
	}
	else
	{
		Serial.println("SGTL cannot be enable. Unespected ERROR.");
	}

	uint8_t boolSGTLinputSelect = 0;
	boolSGTLinputSelect = sgtl5000_1.inputSelect(intAudioInput);
	if(boolSGTLinputSelect == 1)
	{
		Serial.println("Input set to: AUDIO_INPUT_LINEIN");
	}
	else
	{
		Serial.println("SGTL AUDIO_INPUT_LINEIN cannot be set. Unespected ERROR.");
	}
  
	float floatSGTLVolume = sgtl5000_1.volume(0.5);
	Serial.print("SGTL volume: ");
	Serial.println(floatSGTLVolume);

	uint8_t intSGTLLineInLevel;
	intSGTLLineInLevel = sgtl5000_1.lineInLevel(5); // default = 5
	if(intSGTLLineInLevel == 1)
	{
		Serial.println("Line in level set to: 5");
	}
	else
	{
		Serial.println("SGTL LINEIN cannot be set. Unespected ERROR.");
	}
	Serial.println("Initialization done.");
}
*/
