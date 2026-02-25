//Lume440 tuner code by maos 2016 built on top of
//generalized wave freq detection with 38.5kHz sampling rate and interrupts
//by Amanda Ghassaei
//http://www.instructables.com/id/Arduino-Frequency-Detection/
//Sept 2012
//also using blink sketch from FastLED library
//todo: figure out licensing issues (amanda's code is GNU GPL) - maos

#include "FastLED.h"
#define NUM_LEDS 60 //LE
#define DATA_PIN 6 //LED Pin

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
if (frequency < 213.825) {   //A 4 220 213.825 226.54  
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB(250,2,60) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
} 

 else if (frequency >= 213.825 && frequency <= 226.54) {   //A 4 220 213.825 226.54  
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 85,98,112) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
} 
else if (frequency >= 226.55 && frequency <= 240.01) {   //A#/Bb 4 233.08 226.54 240.01  
fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB(78,205,196) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
else if (frequency >= 240.02 && frequency <= 254.285) {   //B 4 246.94 240.01 254.285
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 199,244,100) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
else if (frequency >= 254.286 && frequency <= 269.405) {   //C 4 261.63 254.285 269.405
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 255,107,107) );
  FastLED.show();
 
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
}
else if (frequency >= 269.406 && frequency <= 285.42) {   //C#/Db 4 277.18 269.405 285.42
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 196,77,88) );
  FastLED.show();
delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 285.43 && frequency <= 302.395) {   //D 4 293.66 285.42 302.395
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 240,65,85) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 302.396 && frequency <= 320.38) {   //D#/Eb 4 311.13 302.395 320.38
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 255,130,58) );
  FastLED.show();
delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 320.39 && frequency <= 339.43) {   //E 4 329.63 320.38 339.43
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 242,242,111) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 339.44 && frequency <= 359.61) {   //F 4 349.23 339.43 359.61
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 255,247,189) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 359.62 && frequency <= 380.995) {   //F#/Gb 4 369.99 359.61 380.995
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 149,207,183) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 380.996 && frequency <= 403.65) {   //G 4 392 380.995 403.65
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 81,81,81) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 403.66 && frequency <= 427.65) {   //G#/Ab 4 415.3 403.65 427.65
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 255,255,255) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 427.66 && frequency <= 453.08) {   //A 5 440 427.65 453.08 
fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 0,180,255) );
FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
  
}
else if (frequency >= 453.09 && frequency <= 480.02) {   //A#/Bb 5 466.16 453.08 480.02
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 118,73,172) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
   
}
else if (frequency >= 480.01 && frequency <= 508.565) {   //B 5 493.88 480.02 508.565
  fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 176,222,234) );
  FastLED.show();
  delay(100);
   // Now turn the LED off, then pause
   // leds[0] = CRGB::Black;
   // FastLED.show();
   // delay(100);
   
}
else { fill_solid( &(leds[i]), 60 /*number of leds*/, CRGB( 149,207,183) ); // EXIT THE LOOP
FastLED.show();
 delay(100);
}
}
