#include <unistd.h>
#include <stdio.h>
#include "gpio_lib.c"

#define MSBFIRST 1
#define LSBFIRST 0
#define true 1
#define false 0
typedef int boolean;
typedef int byte;
int exit_arduino_program = 0;
char serial_input_char = 0;
void serial_println_char(char text);
void serial_println_int(int inp);
void serial_println(char *text);
void serial_print(char *text);
void serial_begin(int bt);
void serial_write(char text);
int serial_available();
char serial_read();
void setup();
void loop();
int list[] = {19+64,21+64,20+64,22+64,14+32,16+32,15+32,17+32};
//"pc19","pc21","pc20","pc22",...
//http://linux-sunxi.org/Cubietruck
//http://forum.cubietech.com/forum.php?mod=viewthread&tid=1440
struct srl {
void ( *println_int ) (int inp);
void ( *println_char ) (char text);
void ( *write ) (char text);
void ( *println ) (char *text);
void ( *print ) (char *text);
void ( *begin ) (int bt);
int ( *available ) ();
char ( *read ) ();
} Serial = {serial_println_int,serial_println_char,serial_write,serial_println,serial_print,serial_begin,serial_available,serial_read};
void serial_begin(int bt){
    //Virtual Serial Port doesn't need begin
}
void serial_println_int(int inp){
    printf("%d \n",inp);
}
void serial_println_char(char text){
    printf("%c \n",text);
}
void serial_write(char text){
    printf("%c",text);
}
void serial_println(char *text){
    printf("%s \n",text);
}
void serial_print(char *text){
    printf("%s",text);
}
int serial_available(){
    if((serial_input_char = getchar()) == -1){
        return 0;
    }else{
        return 1;
    }
}
char serial_read(){
    if(serial_input_char != 0){
        int a = serial_input_char;
        serial_input_char = 0;
        return a;
    }else{
	if((serial_input_char = getchar()) == -1){
        	return 0;
	}else{
		return serial_input_char;
	}
    }
}
int pinMode(int pin,int mode){
if(pin < sizeof(list)/sizeof(*list))
return sunxi_gpio_set_cfgpin(list[pin],mode);
else
return -1;
}
int digitalWrite(int pin,int value){
if(pin < sizeof(list)/sizeof(*list))
return sunxi_gpio_output(list[pin],value);
else
return -1;
}
int digitalRead(int pin){
if(pin < sizeof(list)/sizeof(*list))
return sunxi_gpio_input(list[pin]);
else
return -1;
}
void shiftOut(int myDataPin, int myClockPin,int wfst, int myDataOut) {
  //Just support MSB
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
      pinState=1;
    }
    else {	
      pinState=0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }
  //stop shifting
  digitalWrite(myClockPin, 0);
}
int delay(int msec){
usleep(msec*1000);
}
int init(){
    if(SETUP_OK!=sunxi_gpio_init()){
        printf("Failed to initialize GPIO\n");
        return 1;
    }else{
        return 0;
    }
}
int argc;
char **argv;
int main(int iargc,char **iargv){
argc = iargc;
argv = iargv;
init();
setup();
while(1){
    loop();
    if(exit_arduino_program)
        break;
}
return 0;
}
