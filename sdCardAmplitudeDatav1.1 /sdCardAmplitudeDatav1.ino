/*
     First code commit for lume! Woot 7/28/2015 @ 12:04 ish AM - maos:)
     STATUS: Working. Mc signal read, amplitude value calculated, data written to SD.
     File created=lumelog.txt
     ToDo: Combine with the amplitude light controller script @ AmplitudeColorChange.ino

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 50
 ** MISO - pin 52
 ** CLK - pin 51
 ** CS - pin 53

Initial opensource:
 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

Lume440 updates closed source, no license provided or implied. 
-maos 6/27/15
 

 */
#include <SPI.h>
#include <SD.h>
const int chipSelect = 53;
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. 
  }


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");
}
//Start the Lume 440 mic in and amplitude calculations -maos
void loop()
{  
  //set base freq/timing config and get the input signal -maos
   unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level

   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;

   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 3.3) / 1024;  // convert to volts

  // open the file. note that only one file can be open at a time,
  //close this one before opening another.
  File dataFile = SD.open("lumeLOG.txt", FILE_WRITE);

  // if the file is available, write to it:
  // char Setting = 'Amplitude: ';
  if (dataFile) {
    dataFile.println(volts);
    dataFile.close();
    // print to the serial port too:
    Serial.println(volts);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening lumeLOG.txt");
  }
}
