/**
 * library for initiate the hardware
 *
 * @description		this library initiate the nessesary hardware and variables
 * @function 		serialCommunicationInit()
 * @function		timeInit()
 * @function		sdCardInit()
 * @function		teensyInfo(uint8_t intWhichInformation, byte *byteTeensyMac)
 * @function		resetEthernetChip()
 * @function		getTeensy3Time()
 * @function		processSyncMessage()
 * @function		ledBlink(uint8_t intPinLedSample, uint8_t intSamplingStatusMode)
 */
#ifndef __Hardware_Init_H__
#define __Hardware_Init_H__
#endif

// includes
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
const uint8_t 	intResetPinEthernetChip				= 38;		// WIZ820io nReset Pin

// variables
uint8_t 	intErrorTimeInit 						= 0;
uint8_t 	intErrorSdCardInit 						= 0;
uint8_t 	intSerialCommunikationInit 				= 0;
uint8_t		intLedBlinkState 						= LOW;		// ledState used to set the LED
uint32_t	intLedBlinkTimerPreviousMillis 			= 0;		// will store last time LED was updated
uint32_t	intLedBlinkTimerInterval 				= 1000;		// interval at which to blink (milliseconds)

//namespace
Sd2Card 	card;
SdVolume 	volume;
SdFile 		root;

// namespace
time_t 		getTeensy3Time();
uint32_t 	processSyncMessage();


/**
 * serial communication init
 *
 * @description		initialize the serial communication
 * @param 			-
 * @return			-
 */
void serialCommunicationInit()
{
	Serial.begin(intSerialCommunicationSpeed);
	
	Serial.println("\nInitializing serial communication...");
	Serial.begin(intSerialCommunicationSpeed);
	delay(intFunctionWaitDelay);
	Serial.print("Serial speed is set to: ");
	Serial.print(intSerialCommunicationSpeed);
	Serial.println(" Baud");
	Serial.println("Initialization done.");
	delay(intFunctionWaitDelay);
}
	
/**
 * time init
 *
 * @description		setting the time for using in timestamp
 * @param 			-
 * @return			-
 */
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

/**
 * sd card init
 *
 * @description		initialize the sd card on the audio adaptor
 * @param 			-
 * @return			-
 */
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

/**
 * teensy info
 *
 * @description		shows us information about the teensy
 * @param 			intWhichInformation
 * @param			*byteTeensyMac
 * @return			-
 */
void teensyInfo(uint8_t intWhichInformation, byte *byteTeensyMac)
{

	Serial.println("\nReading Teensy...");
	
	switch(intWhichInformation)
	{
		case 1:
			Serial.println("Reading Serial from hardware...");
			Serial.printf("Serialnumber: %s\n", teensySN());
  		break;
  		
  		case 2:
			Serial.println("Reading UUID from hardware...");
			Serial.printf("128-bit UUID RFC4122: %s\n", teensyUUID());
  		break;
		
		case 3:
			Serial.println("Reading Chip ID from hardware...");
  			Serial.printf("128-bit UniqueID from chip: %s\n", kinetisUID());
  		break;
		
		case 4:
			Serial.println("Reading MAC from hardware...");
		  	Serial.printf("MAC Address: %s\n", teensyMAC());
  		break;
  		
  		case 5:
  			Serial.println("Reading USB Serialnumber from hardware...");
			Serial.printf("USB Serialnumber: %u \n", teensyUsbSN());
  		break;
  		
  		default:  // read all infos as default
  			Serial.printf("Serialnumber: %s\n", teensySN());
  			Serial.printf("128-bit UUID RFC4122: %s\n", teensyUUID());
  			Serial.printf("128-bit UniqueID from chip: %s\n", kinetisUID());
  			Serial.printf("MAC Address: %s\n", teensyMAC());
  			Serial.printf("USB Serialnumber: %u \n", teensyUsbSN());
  		break;
  	}
	
	// set byteTeensyMac
	uint8_t i = 0;
	while (mac[i] != 0)
	{
		byteTeensyMac[i] = mac[i];
		i++;
	}
	
	Serial.println("Reading done.");
  	delay(intFunctionWaitDelay);
}

/**
 * reset the ethernet chip
 *
 * @description		WIZ820io/W5200 must be at least reset to avoid it clashing with SPI
 * @param 			-
 * @return			-
 */
void resetEthernetChip()
{
	// set pin mode to input pullup and output
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

/**
 * get teensy time
 *
 * @description		get the time of the teensy
 * @param 			-
 * @return			Teensy3Clock.get() // the time of teensy
 */
time_t getTeensy3Time()
{
	return Teensy3Clock.get();
}

/**
 * sync time over serial port
 *
 * @description		get the time of the teensy
 * @param 			-
 * @return			pctime
 */
uint32_t processSyncMessage()
{
	uint32_t pctime = 0L;  // set pctime to 0; constructor to instantiate a date that refers to zero millis after epoch time
	const uint32_t DEFAULT_TIME = 1357041600; // 2013-01-01T12:00:00+00:00 RFC 3339

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

/**
 * led blink
 *
 * @description		funktion for blinking the led without delay
 * @param 			intPinLedSample
 * @param			intSamplingStatusMode
 * @return			-
 */
void ledBlink(uint8_t intPinLedSample, uint8_t intSamplingStatusMode)
{
	// TODO: set other blink interval for sending
	uint32_t intLedBlinkTimerCurrentMillis = millis();
	if (intSamplingStatusMode == 1 || intSamplingStatusMode == 2)
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
