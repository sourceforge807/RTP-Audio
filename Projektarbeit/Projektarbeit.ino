#include <InternalTemperature.h>
#include <Dhcp.h>
#include <Dns.h>
#include <Ethernet.h>
#include <EthernetClient.h>
#include <EthernetServer.h>
#include <EthernetUdp.h>
#include <w5100.h>
#include <TimeLib.h>
#include <SPI.h>
#include "library/HardwareInit.h"
#include "library/RTPPacketBuilder.h"
#include "library/AudioRecorder.h"
//#include "library/AudioSender.h"
#include <Bounce.h>

// #include <SD.h> //???? include durch HardwareInit????

// Use these with the Teensy Audio Shield
#define         SPI_MOSI_PIN                      7
#define         SPI_MISO_PIN                      12
#define         SPI_SCK_PIN                       14

const uint8_t   intPinLedReady                    = 29;
const uint8_t   intPinLedSample                   = 30;
const uint8_t   intPinButtonStop                  = 33;
const uint8_t   intPinButtonStart                 = 34;
const uint8_t   intPinButtonSend                  = 35;
//const uint16_t  intLocalMulticastPort             = 60000;
//uint16_t const * const  intPointerLocalMulticastPort     = &intLocalMulticastPort;
//const uint8_t   intEthernetPowerDown            = 37;
const uint8_t   intAudioInput                     = AUDIO_INPUT_LINEIN; // which input on the audio shield will be used? //const int myInput = AUDIO_INPUT_MIC;

uint8_t         boolSGTLEnable                    = 0;
uint8_t         boolSGTLinputSelect               = 0;
//uint8_t         *intPointerSamplingStatusMode     = &intSamplingStatusMode;
//uint8_t         intEthernetHasInitialized;
uint8_t         *intPointerEthernetHasInitialized = &intEthernetHasInitialized;

byte            byteTeensyMac[6];
byte            *bytePointerTeensyMac             = &byteTeensyMac[0];
byte            byteMulticastPseudoMac[]          = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
byte            *bytePointerMulticastPseudoMac    = &byteMulticastPseudoMac[0];


//byte ii[256];

//IPAddress       byteStaticTeensyIP(192, 168, 1, 25);
//byte            *bytePointerStaticTeensyIP        = &byteStaticTeensyIP[0];
//IPAddress       byteReceiverMulticastIP(239, 0, 0, 0); // [RFC1112,JBP]
//byte            *bytePointerReceiverMulticastIP   = &byteReceiverMulticastIP[0];


Bounce          buttonStart                 = Bounce(intPinButtonStart, 10);
Bounce          buttonStop                  = Bounce(intPinButtonStop, 10);  // 10 = 10 ms debounce time
Bounce          buttonSend                  = Bounce(intPinButtonSend, 10);

// namespace
InternalTemperature Temperature;



void setup()
{
  Serial.println("Setup begin:");
  pinMode(intPinLedReady, OUTPUT);
  pinMode(intPinLedSample, OUTPUT);
  //pinMode(intEthernetPowerDown, OUTPUT);
  pinMode(intPinButtonStop, INPUT_PULLUP);
  pinMode(intPinButtonStart, INPUT_PULLUP);
  pinMode(intPinButtonSend, INPUT_PULLUP);

  resetEthernetChip();

  SPI.setMOSI(SPI_MOSI_PIN);
  SPI.setMISO(SPI_MISO_PIN);
  SPI.setSCK(SPI_SCK_PIN);
  Ethernet.init(intEthernetChipSelect);
  SPI.begin();

  Serial.println("\nInitializing SGTL...");
  // Disable/ Enable the audio shield, select input and set line in level
  AudioNoInterrupts();
  sgtl5000_1.disable();
  delay(intFunctionWaitDelay);
  boolSGTLEnable = sgtl5000_1.enable();
  delay(intFunctionWaitDelay);
  if (boolSGTLEnable == 1)
  {
    Serial.println("SGTL enable");
  }
  else
  {
    Serial.println("SGTL cannot be enable. Unespected ERROR.");
  }

  AudioMemory(60); // Audio connections require memory, and the record queue. uses this memory to buffer incoming audio.

  boolSGTLinputSelect = sgtl5000_1.inputSelect(intAudioInput);
  if (boolSGTLinputSelect == 1)
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
  if (intSGTLLineInLevel == 1)
  {
    Serial.println("Line in level set to: 5");
  }
  else
  {
    Serial.println("SGTL LINE_IN cannot be set. Unespected ERROR.");
  }

  mixer1.gain(0, 1);
  mixer1.gain(1, 1);
  AudioInterrupts();
  Serial.println("Initialization done.");


  serialCommunicationInit();
  timeInit();
  sdCardInit();

  teensyInfo(0, bytePointerTeensyMac);

  //teensyFreeMemory();

  Temperature.begin();
  Serial.print("\nTeensy Chip Temperature: ");
  Serial.print(Temperature.readTemperatureC(), 1);
  Serial.println("°C");
  if(Temperature.readTemperatureC() > 105 || Temperature.readTemperatureC() < -40)
  {
    Serial.println("Temperature out of operating characteristics");
  }
  else
  {
    Serial.println("Temperature within operating characteristics");
  }

  Serial.println("\nSetup end. Teensy ready...");
  analogWrite(intPinLedReady, 100);
  delay(intFunctionWaitDelay);
}//end setup




void loop()
{
  buttonStart.update();
  buttonStop.update();
  buttonSend.update();

  ledBlink(intPinLedSample, intSamplingStatusMode); // led blink during recording TODO: change blink intervall for error showing


  // Respond to button presses
  if (buttonStart.fallingEdge())
  {
    if (intSamplingStatusMode == 0 || intSamplingStatusMode == 2)
    {
      // init params for RTP packet builder
      randomSeed(analogRead(0)); // read some noise from unused pin, trouble with noise reading
      intRTPSequenceNumber = random(65535); // pseudo random start number between 0 - 65535
      Serial.print("Initial RTP sequence number: ");
      Serial.printf("%lld\n", intRTPSequenceNumber);
      intSampleTime = now() - 7200; // first timestamp (+2h Berlin(CEST)) for sampling
      Serial.print("Initial sampling timestamp: ");
      Serial.println(intSampleTime);
      delay(500);

      startRecording(intPointerEthernetHasInitialized);
    }
  }

  if (buttonStop.fallingEdge())
  {
    if (intSamplingStatusMode == 1 || intSamplingStatusMode == 2)
    {
      stopRecording();
      digitalWrite(intPinLedSample, LOW);
      delay(intFunctionWaitDelay);
    }
  }

  // If we are recording, carry on...
  if (intSamplingStatusMode == 1 || intSamplingStatusMode == 2)
  {
    if (queue1.available() >= 2)
    {
//      continueRecording();
      continueRecording(intPointerEthernetHasInitialized);
    }
  }


  if (buttonSend.fallingEdge())
  {
    // start the Ethernet connection:
    Serial.println("\nInitialize Ethernet without DHCP:");
    Ethernet.begin(byteTeensyMac, byteStaticTeensyIP);

    Serial.print("Assigned IP: ");
    //Serial.println(byteStaticTeensyIP);
    Serial.println(Ethernet.localIP());
    // give the Ethernet shield a second to initialize:
    delay(1000);
    Udp.beginMulticast(byteStaticTeensyIP, intMulticastPort);
    
    intEthernetHasInitialized = 1;
    intSamplingStatusMode = 2;
    Serial.println("\nEthernet without DHCP initialized.");
    delay(1000);
  }
}//end loop

/*
  void startRecording()
  {
  Serial.println("Recording started...\n");
  if (SD.exists("RECORD.RAW")) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove("RECORD.RAW");
  }
  frec = SD.open("RECORD.RAW", FILE_WRITE);
  if(frec)
  {
    queue1.begin();
    intSamplingStatusMode = 1;
  }
  }
*/
/*
  void continueRecording()
  {
  if (queue1.available() >= 2)
  {

    //Serial.printf("%lld\n", intRTPSequenceNumber);

    RTPHeaderBuilder(bytePointerRTPSendBuffer, intPointerRTPSequenceNumber, intPointerSampleTime); // build RTP packet header

    // Fetch 2 blocks from the audio library and copy into a 524 byte RTPSendBuffer (12 byte RTP header + 512 byte data). Max 4096KByte buffer by Wiz820IO
    memcpy(byteRTPSendBuffer, bytePointerRTPSendBuffer, 12);
    memcpy(byteRTPSendBuffer+12, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(byteRTPSendBuffer+268, queue1.readBuffer(), 256);
    queue1.freeBuffer();

    frec.write(byteRTPSendBuffer, 524);

    //Udp.beginPacket(byteReceiverMulticastIP, intLocalMulticastPort);
    //Udp.write(byteRTPSendBuffer, 524);
    //Udp.endPacket();


    for(uint32_t f = 0; f < 524; f++)
    {
      Serial.print(byteRTPSendBuffer[f], BIN);
      Serial.print(".");
    }
    Serial.print("\n");
  }
  }
*/
/*
  void stopRecording()
  {
  queue1.end();
  if (intSamplingStatusMode == 1)
  {
    while (queue1.available() >= 1)
    {
      frec.write((byte*)queue1.readBuffer(), 256);
      //queue1.clear();
      queue1.freeBuffer();
      Serial.println("Free buffer...");
    }
    frec.close();
  }
  //queue1.clear();
  intSamplingStatusMode = 0;
  Serial.println("Recording stopped.");
  }
*/
