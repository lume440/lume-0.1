#include <Arduino.h>

#include "FastLED.h"

// 2015 Lume440: I am maos.
//Status: Working. This script reads the mic signal, calculates the signals amplitude, converts that value to the pixels setBrightness variable to control the lights 
//TODO Comment out functions, combine with sdCardAmplitudeDatav1.ino
//ring
#if FASTLED_VERSION < 3001000
#error "Requires FastLED 3.1 or later; check github for latest code."
#endif

//mic
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

//ring
#define DATA_PIN    6
//#define CLK_PIN   4
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS    64
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          96
//frames per second, original = 120
#define FRAMES_PER_SECOND  120

//button sadaetup
uint8_t Switch = 3;
uint8_t Led =13;
boolean LedState =LOW;
int SwitchState =0;
int SwitchDebounce;
int LastSwitchState=HIGH;
int LastSwitchDebounce=LOW;
int Counter=100;

unsigned long LastDebounceTime = 0;
unsigned long DebounceDelay = 50;

void setup() {
  delay(3000); // 3 second delay for recovery
  Serial.begin(9600); //serial output to monitor mic signal

  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(Switch,INPUT);
  digitalWrite(Switch,HIGH);
  pinMode(Led,OUTPUT);
}


// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = { 
  bpm };

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0; // rotating "base color" used by many of the patterns



void loop()
{
  // Call the current pattern function once, updating the 'leds' array
  gPatterns[gCurrentPatternNumber]();

  // send the 'leds' array out to the actual LED strip
  FastLED.show();  
  // insert a delay to keep the framerate modest
  FastLED.delay(1000/FRAMES_PER_SECOND); 

  // do some periodic updates
  EVERY_N_MILLISECONDS( 20 ) { 
    gHue++; 
  } // slowly cycle the "base color" through the rainbow
  EVERY_N_SECONDS( 10 ) { 
    nextPattern(); 
  } // change patterns periodically

  //mic

}

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void nextPattern()
{
  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE( gPatterns);
}



void bpm()
{ //Start button
  int CurrentSwitch = digitalRead(Switch);
  if (CurrentSwitch != LastSwitchDebounce)
  {
    LastDebounceTime = millis();
  } 
  if ((millis() - LastDebounceTime) > DebounceDelay) 
  {
    if (CurrentSwitch != LastSwitchState) 
    {
      if (CurrentSwitch == LOW)
      {
        Counter=Counter+5;
        Serial.println(Counter);
      } 
    }
    LastSwitchState=CurrentSwitch;
  }
  LastSwitchDebounce = CurrentSwitch;
  //mic
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

  Serial.println(volts);
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  Counter = (102);
  uint8_t BeatsPerMinute = Counter;
  CRGBPalette16 palette = CloudColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    fill_solid( leds, NUM_LEDS, CRGB(50,0,200)); 
    FastLED.setBrightness(volts * 100);
  }
}

