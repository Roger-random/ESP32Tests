/* This is a modification of Adafruit's read_while_looping example
illustrating how to read all channels from the AS7341 and print out reported
values, but allow loop() to run while waiting for the readings. This is
important for compatiblity with Mozzi library to ensure glitch-free sound */
#include <Adafruit_AS7341.h>

#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
/*sine table for oscillators. choose others from mozzi
if triangle wave, square wave, etc, desired*/
#include <tables/sin2048_int8.h>
#include <mozzi_midi.h>

Adafruit_AS7341 as7341;
//TwoWire myWire2(2);
uint16_t readings[12];

/////////////////////////////////////////////////////////////////////////////
// Declarations supporting Mozzi audio generation

// I expected these to be in Mozzi library somewhere but I failed to find it.
const int note_C4 = mtof(60);
const int note_D4 = mtof(62);
const int note_E4 = mtof(64);
const int note_F4 = mtof(65);
const int note_G4 = mtof(67);
const int note_A4 = mtof(69);
const int note_B4 = mtof(71);
const int note_C5 = mtof(72);

int scale4[] = {note_C4, note_D4, note_E4, note_F4, note_G4, note_A4, note_B4, note_C5};

// Mozzi setup
Oscil <2048, AUDIO_RATE> aSin(SIN2048_DATA);

void setup()
{
  Serial.begin(115200);

  // Wait for communication with the host computer serial monitor
  while (!Serial)
  {
    Serial.println("Failed, looping forever");
    delay(1000);
  }

//  myWire2.begin();
  Wire.begin();
  delay(20);

//  if (!as7341.begin(0x39, &myWire2)){ //Use TwoWire I2C port
  if (!as7341.begin()){ //Default address and I2C port
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }

  as7341.setATIME(100);
  as7341.setASTEP(10);
  as7341.setGain(AS7341_GAIN_256X);

  as7341.startReading();

  // Start Mozzi sine wave
  aSin.setFreq(440);
  startMozzi(CONTROL_RATE);

  // Turn LED on to minimum
  as7341.setLEDCurrent(4); // 4mA
  as7341.enableLED(true);
}

// Update Mozzi sine wave frequency based on which (F1-F8) is strongest
void updateFrequency() {
  uint8_t maxIndex = 0;

  for(int i = 1; i < 10; i++) {
    if (i == 4 || i == 5) {
      // Skip Clear and IR sitting between F1-F4 and F5-F8
      continue;
    }
    if (readings[i] > readings[maxIndex]) {
      maxIndex = i;
    }
  }
  aSin.setFreq(scale4[maxIndex]);
}

void updateControl()
{
  bool timeOutFlag = yourTimeOutCheck();
  if(as7341.checkReadingProgress() || timeOutFlag )
  {
    if(timeOutFlag)
    {} //Recover/restart/retc.

    as7341.getAllChannels(readings);  //Calling this any other time may give you old data
    updateFrequency();

    as7341.startReading();
  }
}

int updateAudio(){
  return aSin.next();
}

void loop()
{
  audioHook();
}

bool yourTimeOutCheck()
{
  //Fill this in to prevent the possibility of getting stuck forever if you missed the result, or whatever
  return false;
}

void printReadings()
{
  Serial.print("ADC0/F1 415nm : ");
  Serial.println(readings[0]);
  Serial.print("ADC1/F2 445nm : ");
  Serial.println(readings[1]);
  Serial.print("ADC2/F3 480nm : ");
  Serial.println(readings[2]);
  Serial.print("ADC3/F4 515nm : ");
  Serial.println(readings[3]);
  Serial.print("ADC0/F5 555nm : ");

  /*
  // we skip the first set of duplicate clear/NIR readings
  Serial.print("ADC4/Clear-");
  Serial.println(readings[4]);
  Serial.print("ADC5/NIR-");
  Serial.println(readings[5]);
  */

  Serial.println(readings[6]);
  Serial.print("ADC1/F6 590nm : ");
  Serial.println(readings[7]);
  Serial.print("ADC2/F7 630nm : ");
  Serial.println(readings[8]);
  Serial.print("ADC3/F8 680nm : ");
  Serial.println(readings[9]);
  Serial.print("ADC4/Clear    : ");
  Serial.println(readings[10]);
  Serial.print("ADC5/NIR      : ");
  Serial.println(readings[11]);
}
