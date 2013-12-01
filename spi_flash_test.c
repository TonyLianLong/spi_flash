#include <stdio.h>
#include <stdlib.h>
#include "arduino.h"
#define CS 0
#define DO 1
#define DIO 2
#define CLK 3

#define MSB 0
#define LSB 1
//define DEBUG
char text[2] = {'\0'};
char data[101];
boolean data3[17];
const char data2[100]="这是TLL的SPI FLASH！";
void spi_out(int myDataPin, int myClockPin, byte myDataOut) {
    // This shifts 8 bits out MSB first,
    //on the rising edge of the clock,
    //clock idles low
    
    //internal function setup
    int i=0;
    int pinState;
    pinMode(myClockPin, OUTPUT);
    pinMode(myDataPin, OUTPUT);
    
    //clear everything out just in case to
    //prepare shift register for bit shifting
    digitalWrite(myDataPin, 0);
    digitalWrite(myClockPin, 0);
    
    //for each bit in the byte myDataOut
    //NOTICE THAT WE ARE COUNTING DOWN in our for loop
    //This means that %00000001 or "1" will go through such
    //that it will be pin Q0 that lights.
    for (i=7; i>=0; i--)  {
        digitalWrite(myClockPin, 0);
        
        //if the value passed to myDataOut and a bitmask result
        // true then... so if we are at i=6 and our value is
        // %11010100 it would the code compares it to %01000000
        // and proceeds to set pinState to 1.
        if ( myDataOut & (1<<i) ) {
            pinState= 1;
        }
        else {
            pinState= 0;
        }
        usleep(10);
        //Sets the pin to HIGH or LOW depending on pinState
        digitalWrite(myDataPin, pinState);
        //register shifts bits on upstroke of clock pin
        digitalWrite(myClockPin, 1);
        //zero the data pin after shift to prevent bleed through
        digitalWrite(myDataPin, 0);
        usleep(10);
    }
    
    //stop shifting
    digitalWrite(myClockPin, 0);
    usleep(1);
}
void initspi(){
    pinMode(CS,OUTPUT);
    pinMode(DO,INPUT);
    pinMode(DIO,OUTPUT);
    pinMode(CLK,OUTPUT);
    Serial.println("start!");
    while(Serial.read() != 'l');
    Serial.println("OK");
    digitalWrite(DIO,LOW);
    digitalWrite(CLK,LOW);
    digitalWrite(CS,LOW);
}
void readid(unsigned int mlsb,char *text){
    digitalWrite(CS,LOW);
    usleep(1);
    spi_out(DIO,CLK,0x90);
    spi_out(DIO,CLK,0x00);
    spi_out(DIO,CLK,0x00);
    usleep(1);
    if(mlsb)
        spi_out(DIO,CLK,0x00);
    else
        spi_out(DIO,CLK,0x01);
    int a = 0;
    while(1){
        digitalWrite(CLK,HIGH);
        printf("A %d\n",digitalRead(DO));
        usleep(10);
        boolean getd = digitalRead(DO);
        if(a!=8){
            if(a == 0)
                *(text) = getd;
            else
                *(text) = (*(text)<<1) | getd;
        }else{
            text++;
            *(text) = getd;
        }
        printf("B %d\n",getd);
        a++;
        if(a == 16)break;
        digitalWrite(CLK,LOW);
        usleep(1);
    }
    digitalWrite(CS,HIGH);
    usleep(1);
}
void readflash(unsigned int length,char addr1,char addr2,char addr3,char *data){
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x03);
    spi_out(DIO,CLK,addr1);
    spi_out(DIO,CLK,addr2);
    spi_out(DIO,CLK,addr3);
    int a = 0;
    int rl = length*8;
    while(1){
        digitalWrite(CLK,HIGH);
        boolean getd = digitalRead(DO);
        if(a%8 != 0){
            *(data) = (*(data)<<1) | getd;
        }else{
            if(a != 0)
                data++;
            *(data) = getd;
        }
#ifdef DEBUG
        printf("Read: %d\n",getd);
#endif
        a++;
        if(a == rl)break;
        digitalWrite(CLK,LOW);
    }
    digitalWrite(CS,HIGH);
}
void readregister(boolean *data){
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x05);
    int a = 0;
    int rl = 8;
    while(1){
        digitalWrite(CLK,HIGH);
        boolean getd = digitalRead(DO);
        *data = getd;
        data++;
        #ifdef DEBUG
        printf("REG: %d\n",getd);
#endif
        a++;
        if(a == rl){
#ifdef DEBUG
            printf("BUSY: %d\n",getd);
#endif
            //BUSY
            break;
        }
        digitalWrite(CLK,LOW);
    }
    digitalWrite(CS,HIGH);
}
boolean isbusy(){
    boolean getd;
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x05);
    int a = 0;
    int rl = 8;
    while(1){
        digitalWrite(CLK,HIGH);
        getd = digitalRead(DO);
        a++;
        if(a == rl){
#ifdef DEBUG
            printf("BUSY: %d\n",getd);
#endif
            //BUSY
            break;
        }
        digitalWrite(CLK,LOW);
    }
    digitalWrite(CS,HIGH);
    return getd;
}
void eraseflash(char addr1,char addr2,char addr3){
    //It will erase 4K data
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x06);
    //Write Enable
    digitalWrite(CS,HIGH);
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x20);
    spi_out(DIO,CLK,addr1);
    spi_out(DIO,CLK,addr2);
    spi_out(DIO,CLK,addr3);
    digitalWrite(CS,HIGH);
}
int eraseall(){
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x06);
    //Write Enable
    digitalWrite(CS,HIGH);
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0xC7);
    digitalWrite(CS,HIGH);
}
int writeflash(unsigned int length,char addr1,char addr2,char addr3,const char *data){
    if(/*length%8 != 0 || */length > 256 ||length == 0)return 1;
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x06);
    //Write Enable
    digitalWrite(CS,HIGH);
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x02);
    spi_out(DIO,CLK,addr1);
    spi_out(DIO,CLK,addr2);
    spi_out(DIO,CLK,addr3);
    for(int i=0;i<length;i++){
        spi_out(DIO,CLK,*(data));
#ifdef DEBUG
       printf("write data: %c\n",*(data));
#endif
        data++;
    }
    digitalWrite(CS,HIGH);
}
void setup(){
    Serial.begin(9600);
    Serial.println("OK");
    initspi();
    Serial.println("OK");
    readid(MSB,text);
    printf("ID %x %x\n",text[0],text[1]);
    //Serial.println(text[0],HEX);
    //Serial.println(text[1],HEX);
    Serial.println("Read");
    readflash(100,0x00,0x00,0x00,data);
    data[99] = '\0';
    printf("READ1:%s\n",data);
    //eraseflash(0x00,0x00,0x00);
    while(isbusy());
    //writeflash(100,0x00,0x00,0x00,data2);
    while(isbusy());
    readflash(100,0x00,0x00,0x00,data);
    data[100] = '\0';
    printf("READ2:%s\n",data);
    /*for(int i=0;i<8;i++){
     digitalWrite(CLK,HIGH);
     digitalWrite(DIO,code[i]);
     digitalWrite(CLK,LOW);
     }*/
    /*for(int i=0;i<24;i++){
     digitalWrite(CLK,HIGH);
     digitalWrite(DIO,LOW);
     digitalWrite(CLK,LOW);
     }*/
    //pinMode(CLK,INPUT);
    Serial.println("End");
}
void loop(){
    
}
