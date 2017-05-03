// gb_volca : output nanoloop sync to volca
//
// PINOUT
//
// PORTB: VNxxxxxx
//
// -> PIN 13 goes to volca
// -> PIN 12 is nanoloop sync input
// -> PIN 11 LED
// -> PIN 2  LED SEG DIO
// -> PIN 3  LED SEG CLK

#include "TM1637.h"
#define CLK 3//pins definitions for TM1637 and can be changed to other ports       
#define DIO 2

TM1637 tm1637(CLK,DIO);

int8_t msg_c001[4] = { 0xC, 0, 0, 0x1 };
int8_t msg_f001[4] = { 0xF, 0, 0, 0x1 };


int8_t msg_nova[4] = { 55, 63, 62, 119 };
int8_t msg_chip[4] = { 57, 118, 6, 115 };

int i1 = 0;
int i2 = 0;
int i3 = 0;
int i  = 0;

void setup() {
  // initialize digital pin 13 as an output.
  //        +----------> pin 13 : out : volca bass sync
  //        |+---------> pin 12 : in  : gameboy 
  //        ||+--------> pin 11 : out : LED
  //        |||
  //        vvv
  //DDRB = B00101111;
  PORTB = 0;
  //PORTB &=  B11010111;

  pinMode(11, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, INPUT);
  

  digitalWrite(11, LOW);

  for(i=0; i<2; i++) {
    digitalWrite(11, HIGH);   
    delay(200);              
    digitalWrite(11, LOW);    
    delay(300);              
  }

  tm1637.init();
  tm1637.set(5);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  int j=0;

  for(j=0; j<4; j++){
    for(i=0; i<8; i++) {
      tm1637.set(i);
      tm1637.my_display4(msg_nova);      
      delay(10);
    }
    delay(100);
    for(i=0; i<8; i++) {
      tm1637.set(7-i);
      tm1637.my_display4(msg_nova);  
      delay(15);
    }
    delay(100);
  }
  
  tm1637.clearDisplay();
 
  tm1637.set(5);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;

  for(i=0; i<8; i++) {
    digitalWrite(11, HIGH);   
    //tm1637.display(msg_c001);  
    delay(75);              
    digitalWrite(11, LOW);    
    //tm1637.clearDisplay();
    delay(75);              
  }


  for(j=0; j<4; j++){
    for(i=0; i<8; i++) {
      tm1637.set(i);
      tm1637.my_display4(msg_chip);  
      delay(25);
    }
    delay(100);
    for(i=0; i<8; i++) {
      tm1637.set(7-i);
      tm1637.my_display4(msg_chip);  
      delay(50);
    }
    tm1637.clearDisplay();
    delay(50);
    delay(100);
    msg_c001[3]++;
  }
  
  Serial.begin(115200);
  
}

void loop() {
  while(PINB & B00010000) {           // while nano loop sync signal is high
    if(i2==0)  PORTB |=  B00101000;     // set output to 1        
  }   
    if(i1==3) { 
    Serial.println("B");
    i1=0;                  // loop 
    i3++;
    if(i3 == 16) i3 = 0;
    
    tm1637.display(0, i3);  
    tm1637.display(3, i3/4+1);      
  }
  while(!(PINB& B00010000)) {
    if(i2==0) PORTB &=  B11010111;     // set output to 0   
  }  

  i1++; i2++;
  


  if(i2==6) i2=0;  
 
}

