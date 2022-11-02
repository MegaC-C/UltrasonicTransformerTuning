#include <Arduino.h>
/*  
 This code helps to adjust the pulse transformer of ultrasonic transducers on DIY boards without STM8. 
 Connect TRIG_PIN of esp32 to transformer input and ECHO_PIN to OpAmp output. 
 Face the transducer into the void so that no echo gets reflected.
 Read the serial output while adjusting the transformer to achieve a minimum ringing time. 
*/ 
  
 //---PINs--- 
 const uint8_t TRIG_PIN = 15; 
 const uint8_t ECHO_PIN = 4; 

 uint32_t ticks = xthal_get_ccount();
 uint32_t ticksDelay = 3000;
 uint32_t startRingingTime = 0;         // in µs 
 volatile uint32_t stopRingingTime = 0; // in µs 
 uint32_t ringingTime = 0;              // in µs 
 uint32_t lowestRingingTime = 1000000;  // in µs 
 uint32_t timeMeasured = 0;             // in ms 
 const uint32_t timeToMeasure = 1000;   // in ms => measure for 1s 
  
  
 void delayUsingTicks(uint32_t n){                   // delay using CPU ticks of the esp32, to get sub µs resolution 
     n = n - 120;                                    // number of ticks lost during routine
     ticks = xthal_get_ccount();                     // @240MHz 1 tick = 4.1666ns 
     while(xthal_get_ccount() - ticks < n){ 
         // do nothing, just chill 
     } 
 } 

 static void IRAM_ATTR triggerFlag (){
    stopRingingTime = micros();
}            
  
 //--------------------------------------------------------- 
 void setup() 
 { 
     Serial.begin(115200); 
         pinMode(TRIG_PIN, OUTPUT); 
         digitalWrite(TRIG_PIN, LOW); 
         pinMode(ECHO_PIN, INPUT);

         delay(2000);                               // needed to prevent false readings (sensors settle?)         
 } 
  
 //---------------------------------------------------------- 
 void loop() 
 { 
     //--- 8 pulses @40kHz, last one delayed by half the period to reduce ringing --- 
     // https://www.davidpilling.com/wiki/index.php/JSN?setskin=blog 
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay);              // delays 3000 ticks = 12.5µs @240MHz CPU 
     digitalWrite(TRIG_PIN, LOW);              // => 40kHz PWM with 50% duty cycle
     delayUsingTicks(ticksDelay);               
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, HIGH); 
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW); 
     delayUsingTicks(2*ticksDelay);              // delay twice to reduce ringing to ~680µs
     digitalWrite(TRIG_PIN, HIGH);               // without delaying the last pulse ringing time is ~740µs
     delayUsingTicks(ticksDelay); 
     digitalWrite(TRIG_PIN, LOW);     
  
     // measure all ringing ECHO signals during timeToMeasure and take the last ECHO signal to calculate ringingTime  
     timeMeasured = millis(); 
     startRingingTime = micros();
     attachInterrupt(digitalPinToInterrupt(ECHO_PIN), triggerFlag, FALLING);      
     while(millis() - timeMeasured < timeToMeasure){ 
        // do nothing for timeToMeasure, only ISR actions
     } 
     detachInterrupt(digitalPinToInterrupt(ECHO_PIN));     
     ringingTime = stopRingingTime - startRingingTime; 
     if(ringingTime < lowestRingingTime){ 
         lowestRingingTime = ringingTime; 
     } 
  
     Serial.print(lowestRingingTime); 
     Serial.println(" µs lowest ringing time"); 
     Serial.print(ringingTime); 
     Serial.println(" µs current ringing time"); 
     Serial.print(ringingTime/58); 
     Serial.println(" cm roughly current distance"); 
     Serial.println();
    }
