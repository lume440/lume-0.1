//Lume440 tuner code by maos 2016 built on top of
//generalized wave freq detection with 38.5kHz sampling rate and interrupts
//by Amanda Ghassaei
//http://www.instructables.com/id/Arduino-Frequency-Detection/
//Sept 2012
//also using blink sketch from FastLED library
//todo: figure out licensing issues (amanda's code is GNU GPL) - maos

#include "FastLED.h"
#define NUM_LEDS 150 //LE
#define DATA_PIN 7 //LED Pin

// Define the array of leds
CRGB leds[NUM_LEDS];


//clipping indicator variables
boolean clipping = 0;

//data storage variables
byte newData = 0;
byte prevData = 0;
unsigned int time = 0;//keeps time and sends vales to store in timer[] occasionally
int timer[10];//sstorage for timing of events
int slope[10];//storage for slope of events
unsigned int totalTimer;//used to calculate period
unsigned int period;//storage for period of wave
byte index = 0;//current storage index
float frequency;//storage for frequency calculations
int maxSlope = 0;//used to calculate max slope as trigger point
int newSlope;//storage for incoming slope data

//variables for decided whether you have a match
byte noMatch = 0;//counts how many non-matches you've received to reset variables if it's been too long
byte slopeTol = 3;//slope tolerance- adjust this if you need
int timerTol = 10;//timer tolerance- adjust this if you need

//variables for amp detection
unsigned int ampTimer = 0;
byte maxAmp = 0;
byte checkMaxAmp;
byte ampThreshold = 30;//raise if you have a very noisy signal

void setup(){
  
  Serial.begin(9600);
  //initialize LEDs -maos
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  
  pinMode(13,OUTPUT);//led indicator pin
  pinMode(12,OUTPUT);//output pin
  
  cli();//diable interrupts
  
  //set up continuous sampling of analog pin 0 at 38.5kHz
 
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger    
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();//enable interrupts
}

ISR(ADC_vect) {//when new ADC value ready
  
  PORTB &= B11101111;//set pin 12 low
  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >=127){//if increasing and crossing midpoint
    newSlope = newData - prevData;//calculate slope
    if (abs(newSlope-maxSlope)<slopeTol){//if slopes are ==
      //record new data and reset time
      slope[index] = newSlope;
      timer[index] = time;
      time = 0;
      if (index == 0){//new max slope just reset
        PORTB |= B00010000;//set pin 12 high
        noMatch = 0;
        index++;//increment index
      }
      else if (abs(timer[0]-timer[index])<timerTol && abs(slope[0]-newSlope)<slopeTol){//if timer duration and slopes match
        //sum timer values
        totalTimer = 0;
        for (byte i=0;i<index;i++){
          totalTimer+=timer[i];
        }
        period = totalTimer;//set period
        //reset new zero index values to compare with
        timer[0] = timer[index];
        slope[0] = slope[index];
        index = 1;//set index to 1
        PORTB |= B00010000;//set pin 12 high
        noMatch = 0;
      }
      else{//crossing midpoint but not match
        index++;//increment index
        if (index > 9){
          reset();
        }
      }
    }
    else if (newSlope>maxSlope){//if new slope is much larger than max slope
      maxSlope = newSlope;
      time = 0;//reset clock
      noMatch = 0;
      index = 0;//reset index
    }
    else{//slope not steep enough
      noMatch++;//increment no match counter
      if (noMatch>9){
        reset();
      }
    }
  }
    
  if (newData == 0 || newData == 1023){//if clipping
    PORTB |= B00100000;//set pin 13 high- turn on clipping indicator led
    clipping = 1;//currently clipping
  }
  
  time++;//increment timer at rate of 38.5kHz
  
  ampTimer++;//increment amplitude timer
  if (abs(127-ADCH)>maxAmp){
    maxAmp = abs(127-ADCH);
  }
  if (ampTimer==1000){
    ampTimer = 0;
    checkMaxAmp = maxAmp;
    maxAmp = 0;
  }
  
}

void reset(){//clea out some variables
  index = 0;//reset index
  noMatch = 0;//reset match couner
  maxSlope = 0;//reset slope
}


void checkClipping(){//manage clipping indicator LED
  if (clipping){//if currently clipping
    PORTB &= B11011111;//turn off clipping indicator led
    clipping = 0;
  }
}




void loop(){
  
  checkClipping();
  
  if (checkMaxAmp>ampThreshold){
    frequency = 38462/float(period);//calculate frequency timer rate/period
    delay(10);

    //print results
    Serial.print(frequency);
    Serial.println(" hz");
  }
//Amplitude section

//FastLED.setBrightness(frequency);
//delay(100);//delete this if you want
  

int i=0;

//Start frequency to color logic loop
//todo sort out boolean ranges as frequency buckets -maos

//below octave show black
if (frequency < 213.825) {   //A 4 220 213.825 226.54  
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,0,0) );
  FastLED.show();
  //delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
} 
//start A block
 else if (frequency >= 213.825 && frequency <= 226.54) {   //A 4 220 213.825 226.54  
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,0,255) );
  FastLED.show();
  //delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
} 
//start B block
else if (frequency >= 240.01 && frequency <= 254.284) {   //A#/Bb 4 233.08 226.54 240.01  
fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,255,0) );
  FastLED.show();
 // delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
//start C block
else if (frequency >= 254.285 && frequency <= 269.405) {   //B 4 246.94 240.01 254.285
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,255,255) );
  FastLED.show();
  //delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
//start D block
else if (frequency >= 285.42 && frequency <= 302.395) {   //C 4 261.63 254.285 269.405
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 255,255, 0) );
  FastLED.show();
 
 // delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
//start E block
else if (frequency >= 320.38 && frequency <= 339.42) {   //C#/Db 4 277.18 269.405 285.42
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 255,0,0) );
  FastLED.show();
//delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
//start F block
else if (frequency >= 339.43 && frequency <= 359.61) {   //D 4 293.66 285.42 302.395
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 255,0, 255) );
  FastLED.show();
//  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
//start G block
else if (frequency >= 380.995 && frequency <= 403.65) {   //D#/Eb 4 311.13 302.395 320.38
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 127,0,127) );
  FastLED.show();
//delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}

//start A5 block
else if (frequency >= 453.08 && frequency <= 427.65) {   //D#/Eb 4 311.13 302.395 320.38
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 0,0,255) );
  FastLED.show();
//delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
//start B5 block
else if (frequency >= 480.02 && frequency <= 508.564) {   //A#/Bb 4 233.08 226.54 240.01  
fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,255,0) );
  FastLED.show();
 // delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
//start C5 block
else if (frequency >= 508.565 && frequency <= 538.81) {   //B 4 246.94 240.01 254.285
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,255,255) );
  FastLED.show();
 // delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
//start D5 block
else if (frequency >= 570.85 && frequency <= 604.79) {   //C 4 261.63 254.285 269.405
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 255,255, 0) );
  FastLED.show();
 
//  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}

//start E5 block
else if (frequency >= 678.855 && frequency <= 640.75) {   //C#/Db 4 277.18 269.405 285.42
  fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB( 255,0,0) );
  FastLED.show();
//delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}

//above octave show black
else { fill_solid( &(leds[i]), 150 /*number of leds*/, CRGB(0,0,0) ); // EXIT THE LOOP
FastLED.show();
 //delay(100);
}
}





