#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include "arduino.h"
#include "spi_flash.h"

#define CS 0
#define DO 1
#define DIO 2
#define CLK 3

#define MSB 0
#define LSB 1
//define DEBUG
char text[2] = {'\0'};
char text2[3] = {'\0'};
boolean yes = 0;
boolean filltozero = 0;
boolean bin = 0;
//char data[101];
//boolean data3[17];
//const char data2[100]="这是TLL的SPI FLASH！";
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
        //usleep(1);
        //Sets the pin to HIGH or LOW depending on pinState
        digitalWrite(myDataPin, pinState);
        //register shifts bits on upstroke of clock pin
        digitalWrite(myClockPin, 1);
        //zero the data pin after shift to prevent bleed through
        digitalWrite(myDataPin, 0);
        //usleep(1);
    }
    
    //stop shifting
    digitalWrite(myClockPin, 0);
    //usleep(1);
}
void initspi(){
    pinMode(CS,OUTPUT);
    pinMode(DO,INPUT);
    pinMode(DIO,OUTPUT);
    pinMode(CLK,OUTPUT);
    digitalWrite(DIO,LOW);
    digitalWrite(CLK,LOW);
    digitalWrite(CS,LOW);
}
void readid(unsigned int mlsb,char *text){
    digitalWrite(CS,LOW);
    //usleep(1);
    spi_out(DIO,CLK,0x90);
    spi_out(DIO,CLK,0x00);
    spi_out(DIO,CLK,0x00);
    //usleep(1);
    if(mlsb)
        spi_out(DIO,CLK,0x00);
    else
        spi_out(DIO,CLK,0x01);
    unsigned long long a = 0;
    while(1){
        digitalWrite(CLK,HIGH);
        #ifdef DEBUG
        printf("A %d\n",digitalRead(DO));
        #endif
        //usleep(10);
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
        #ifdef DEBUG
        printf("B %d\n",getd);
        #endif
        a++;
        if(a == 16)break;
        digitalWrite(CLK,LOW);
        //usleep(1);
    }
    digitalWrite(CS,HIGH);
    //usleep(1);
}
void read_global_id(char *data){
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x9F);
    int a = 0;
    int rl = 3*8;
    while(1){
        digitalWrite(CLK,HIGH);
        boolean getd = digitalRead(DO);
        if(a%8 != 0){
            *(data) = (*(data)<<1) | getd;
        }else{
            if(a != 0){
                data++;
            }
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
void readflash(unsigned long long length,char addr1,char addr2,char addr3,char *data){
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x03);
    spi_out(DIO,CLK,addr1);
    spi_out(DIO,CLK,addr2);
    spi_out(DIO,CLK,addr3);
    unsigned long long a = 0;
    int rl = length*8;
    while(1){
        digitalWrite(CLK,HIGH);
        boolean getd = digitalRead(DO);
        if(a%8 != 0){
            *(data) = (*(data)<<1) | getd;
        }else{
            if(a != 0){
                if(*data == 255 && filltozero == 1)
                    *data = 0;
                //Fill to zero
                data++;
            }
            *(data) = getd;
        }
#ifdef DEBUG
        printf("Read: %d\n",getd);
#endif
        if(show_process)printf("Loc:%llx Data:%x\n",a,*(data));
        //too slow
        a++;
        if(a == rl){
            if(*data == 255 && filltozero == 1)
                *data = 0;
            //Fill to zero
            break;
        }
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
int _writeflash(unsigned long long length,char addr1,char addr2,char addr3,const char *data);
int writeflash(unsigned long long length,char addr1,char addr2,char addr3,const char *data){
    //printf("write flash\n");
    if(/*length%8 != 0 || */length > 256 ||length == 0){
        //printf("length is too large or invalid!\n");
        //return 1;
        unsigned long long lb = length/256;
        long long ys = length%256;
        if(ys != 0)lb++;
        for(unsigned long long i=0;i<lb;i++){
            if(show_process)
            printf("writeflash %x %x %x addrd:%llu\n",addr1,addr2,addr3,data,*(data));
            
            if(i == (lb-1))
            _writeflash(ys,addr1,addr2,addr3,data);
            else
            _writeflash(256,addr1,addr2,addr3,data);
            data+=256;
            while(isbusy());
            //if(addr3 != 0xFF){
              //  addr3++;
            //}else{
                //addr3 = 0;
                if(addr2 != 0xFF){
                    addr2++;
                }else{
                    addr2 = 0;
                    if(addr1 != 0xFF){
                        addr1++;
                    }else{
                        printf("too large?!\n");
                        return -1;
                    }
                }
            //}
        }
    }else{
        _writeflash(length,addr1,addr2,addr3,data);
    }
}
int _writeflash(unsigned long long length,char addr1,char addr2,char addr3,const char *data){
    //printf("%lld\n",length);
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x06);
    //Write Enable
    digitalWrite(CS,HIGH);
    digitalWrite(CS,LOW);
    spi_out(DIO,CLK,0x02);
    spi_out(DIO,CLK,addr1);
    spi_out(DIO,CLK,addr2);
    spi_out(DIO,CLK,addr3);
    //printf("LEN:%lld\n",length);
    for(unsigned long long i=0;i<length;i++){
        //printf("nr:%x %d\n",*(data));
        spi_out(DIO,CLK,*(data));
//ifdef DEBUG
        //if(i==0)
        //printf("%x  ",*(data));
//endif
        data++;
    }
    digitalWrite(CS,HIGH);

}
int devi_id;
int manu_id;
int memo_id;
int capa_id;
char *file = NULL;
char *filebuf;
char *readbuf;
FILE *pF;
unsigned long long flen = 0;
unsigned long long frlen = 0;
boolean save = 0;
int will_do = -1;
//-1 unknown
//0 write
//1 read
//2 erase
//3 verify
//4 test
uint8_t addr1 = 0x00;
uint8_t addr2 = 0x00;
uint8_t addr3 = 0x00;
boolean show = 0;
boolean winbond = 0;
boolean show_process = 0;
unsigned long long flashsize = 0;
void setup(){
    long long length = 0;
    int ch;
    opterr = 0;
    int skip = 0;
    int erase = 1;
    while ((ch = getopt(argc,argv,"sw:er::1:2:3:hl:yfbtv:di"))!=-1){
        //Set CH to getopt and to know the argv
        switch(ch)
        {
            case 'i':
                winbond = 1;
                break;
            case 's':
                printf("skip id scanning!\n");
                skip = 1;
                break;
            case 'w':
                if((will_do) == -1){
                    will_do = 0;
                }else{
                    printf("%s\n",help_text);
                    exit(4);
                }
                file = (char *)malloc(sizeof(char)*strlen(optarg));
                if(!file){
                    printf("no such mem\n");
                    exit(1);
                }
                strcpy(file,optarg);
                printf("Filename:%s\n",file);
                break;
            case 'l':
                length = atoi(optarg);
                break;
            case 'e':
                erase = 0;
                break;
            case 'r':
                if(optarg != NULL){
                    save = 1;
                    file = (char *)malloc(sizeof(char)*strlen(optarg));
                    if(!file){
                        printf("no such mem\n");
                        exit(1);
                    }
                    strcpy(file,optarg);
                    printf("save to %s\n",file);
                }
                if((will_do) == -1){
                    will_do = 1;
                }else{
                    printf("%s\n",help_text);
                    exit(4);
                }
                //printf("Unsupport now.\n");
                break;
            case 'v':
                if((will_do) == -1){
                    will_do = 3;
                }else{
                    printf("%s\n",help_text);
                    exit(4);
                }
                file = (char *)malloc(sizeof(char)*strlen(optarg));
                if(!file){
                    printf("no such mem\n");
                    exit(1);
                }
                strcpy(file,optarg);
                printf("verify with %s\n",file);
                break;
            case '1':
                addr1 = atoi(optarg);
                break;
            case '2':
                addr2 = atoi(optarg);
                break;
            case '3':
                addr3 = atoi(optarg);
                break;
            case '?':
                printf("%s\n",help_text);
                exit(3);
            case 'h':
                show = 1;
                break;
            case 'y':
                yes = 1;
                break;
            case 'f':
                filltozero = 1;
                break;
            case 'b':
                bin = 1;
                break;
            case 't':
                will_do = 4;
                break;
            default:
                //printf("unknown option :%c\n",ch);
                printf("Unsupport now.\n");
                exit(4);
            case 'd':
                show_process = 1;
                break;
        }
    }
    if(will_do == -1 && erase == 0){
        will_do = 2;
    }
    if(will_do == -1){
        printf("%s\n",help_text);
        exit(3);
    }
    //For arg debug:
    //exit(0);
    if(will_do == 0){
    if(!bin)
    pF = fopen(file,"r");
    else
    pF = fopen(file,"rb");
    if(!pF){
        perror("Error in opening file");
        exit(2);
    }
    fseek(pF,0,SEEK_END);
    flen = ftell(pF);
    //get length
    fseek(pF,0,SEEK_SET);
    //turn back
    filebuf = (char *)malloc(sizeof(char)*flen+1);
    //For the last '\0'
    if(!filebuf){
        printf("no enough mem\n");
        exit(1);
    }
    frlen = fread(filebuf,sizeof(char),flen/sizeof(char),pF);
    //printf("size:%d byte,read:%d byte\n",flen,frlen);
    printf("Read size:%lld bytes\n",frlen);
    filebuf[frlen] = '\0';
    //Add '\0'
    if(show)printf("File:%s\n",filebuf);
    if(fclose(pF)){
        perror("Error in closing");
        exit(1);
    }
    }
    /*if(will_do == 1 && save == 1){
        printf("save to file unsupport!\n");
    }*/
    Serial.begin(9600);
    initspi();
    while(isbusy());
    int tp = 0;
    if(winbond){
        readid(MSB,text);
        //printf("ID %x %x\n",text[0],text[1]);
        devi_id = text[0];
        manu_id = text[1];
        if((tp = check_id(devi_id,manu_id)) == -1){
            printf("Unknown id: device id:0x%x,manu id:0x%x.If you are not using winbond flash,try to NOT to use \"-i\"\n",devi_id,manu_id);
            if(!skip)
                exit(1);
        }else{
            printf("device id:0x%x manu id:0x%x\n",devi_id,manu_id);
        }
    }else{
        //Non winbond flash
        read_global_id(text2);
        printf("manu id:0x%x ",text2[0]);
        printf("memory type id:0x%x ",text2[1]);
        printf("capacity id:0x%x\n",text2[2]);
        memo_id = text2[1];
        manu_id = text2[0];
        capa_id = text2[2];
        if((tp = check_gid(memo_id,manu_id,capa_id)) == -1){
            printf("Unknown id!If you know this flash's size,you can use \"-s -l <size>\" argu to skip scanning.\n");
            if(!skip)
                exit(1);
            else
                printf("Using default flashsize(4M).");
        }

    }
    if(!skip){
        printf("chip lib-id:%d\n",tp);
        printf("chip name:%s\n",chips[tp].name);
        flashsize = chips[tp].size;
        printf("chip size:%lld\n",chips[tp].size);
    }
    if(tp == -1)flashsize = 4UL*1024*1024;//Default 4M
    if(!yes){
        Serial.println("Enter y to start!");
        while(Serial.read() != 'y');
        Serial.println("Start!");
    }
    //If not skip,print chip name,if yes and print,may mem problem?
    //Serial.println(text[0],HEX);
    //Serial.println(text[1],HEX);
    /*Serial.println("Read");
    readflash(100,0x00,0x00,0x00,data);
    data[99] = '\0';
    printf("READ1:%s\n",data);
    eraseflash(0x00,0x00,0x00);
    while(isbusy());*/
    if(will_do == 0){
        if(erase){
                Serial.println("Erasing...");
                eraseall();
        }
        while(isbusy());
        printf("Writing bytes:%lld %lld...\n",frlen,length);
        if(length == 0){
            writeflash(frlen,addr1,addr2,addr3,filebuf);
        }else{
            if(length > (frlen)){
                printf("ERROR!Input length is larger than file's length %lld %lld!\n",frlen,length);
                exit(5);
            }
            writeflash(length,addr1,addr2,addr3,filebuf);
        }
        while(isbusy());
    }
    if(will_do == 3){
        if(!bin)
            pF = fopen(file,"r");
        else
            pF = fopen(file,"rb");
        if(!pF){
            perror("Error on opening file");
            exit(2);
        }
        fseek(pF,0,SEEK_END);
        flen = ftell(pF);
        //get length
        fseek(pF,0,SEEK_SET);
        //turn back
        filebuf = (char *)malloc(sizeof(char)*flen+1);
        //For the last '\0'
        if(!filebuf){
         printf("no enough mem\n");
         exit(1);
        }
        long long rlen = fread(filebuf,sizeof(char),flen/sizeof(char),pF);
        //printf("size:%d byte,read:%d byte\n",flen,frlen);
        printf("Read size:%lld,%lld bytes\n",rlen,flen);
        //Add '\0'
        if(show)printf("File:%s\n",filebuf,flen);
        if(fclose(pF)){
            perror("Error on closing");
            exit(1);
        }
        long long len;
        while(isbusy());
        printf("Start reading...\n");
        if(length != 0){
            if(length > flashsize){
                printf("ERROR!Input length is larger than flash's length!\n");
                exit(5);
            }
            readbuf = (char *)malloc(length+1);
            readflash(length,addr1,addr2,addr3,readbuf);
            readbuf[length] = '\0';
            len=length;
        }else{
            if(flashsize <= 0){
                printf("Unknown flash size!Give me a length!\n");
                exit(6);
            }
            readbuf = (char *)malloc(flashsize+1);
            readflash(flashsize,addr1,addr2,addr3,readbuf);
            readbuf[flashsize] = '\0';
            len=flashsize;
        }
        while(isbusy());
        if(issame(filebuf,readbuf,sizeof(char)*flen,len)){
            printf("same!\n");
        }else{
            printf("Not same!\n");
            exit(6);
        }
    }
    if(will_do == 1){
        long long len;
        while(isbusy());
        printf("Start reading...\n");
        if(length != 0){
            if(length > flashsize){
                printf("ERROR!Input length is larger than flash's length!\n");
                exit(5);
            }
            readbuf = (char *)malloc(length+1);
            readflash(length,addr1,addr2,addr3,readbuf);
            readbuf[length] = '\0';
            len=length;
        }else{
            if(flashsize <= 0){
                printf("Unknown flash size!Give me a length!\n");
                exit(6);
            }
            readbuf = (char *)malloc(flashsize+1);
            readflash(flashsize,addr1,addr2,addr3,readbuf);
            readbuf[flashsize] = '\0';
            len=flashsize;
        }
        while(isbusy());
        if(!save){
            printf("Read:%s\n",readbuf);
            //printf("Read:%d\n",readbuf[0]);
            //printf("Read:%c\n",readbuf[0]);
            //debug
        }else{
            printf("Saving...\n");
            if(!bin)
                pF = fopen(file,"w+");
            else
                pF = fopen(file,"wb+");
            if(!pF){
                perror("Error in opening file");
                exit(2);
            }
            //fseek(pF,0,SEEK_END);
            //flen = ftell(pF);
            //get length
            //fseek(pF,0,SEEK_SET);
            //turn back
            //filebuf = (char *)malloc(sizeof(char)*flen+1);
            //For the last '\0'
            /*if(!filebuf){
                printf("no enough mem\n");
                exit(1);
            }*/
            int wrlen = fwrite(readbuf,sizeof(char),len/sizeof(char),pF);
            //printf("size:%d byte,read:%d byte\n",flen,frlen);
            printf("Read size:%lld bytes\n",len);
            //Add '\0'
            //if(show)printf("File:%s\n",readbuf);
            if(fclose(pF)){
                perror("Error in closing");
                exit(1);
            }

        }
    }
    if(will_do == 2){
        Serial.println("Erasing...");
        eraseall();
        while(isbusy());
    }
    if(will_do == 4){
        if(winbond){
            printf("Read winbond ID...\n");
            readid(MSB,text);
            //printf("ID %x %x\n",text[0],text[1]);
            devi_id = text[0];
            manu_id = text[1];
            if((tp = check_id(devi_id,manu_id)) == -1){
                printf("Unknown id: device id:0x%x,manu id:0x%x.If you are not using winbond flash,try to NOT to use \"-i\"\n",devi_id,manu_id);
            }else{
                printf("device id:0x%x manu id:0x%x\n",devi_id,manu_id);
            }
        }else{
            printf("Read global ID...\n");
            read_global_id(text2);
            printf("manu id:0x%x ",text2[0]);
            printf("memory type id:0x%x ",text2[1]);
            printf("capacity id:0x%x\n",text2[2]);
            int fid = text2[0]<<16;
            //printf("allid:0x%x\n",fid);
            fid = (text2[1]<<8)|fid;
            //printf("allid:0x%x %x\n",fid,(text2[1]<<8));
            fid = fid|text2[2];
            printf("allid:0x%x\n",fid);
            if((tp = check_gid(memo_id,manu_id,capa_id)) == -1){
                printf("Unknown id!If you know this flash's size,you can use \"-s -l <size>\" argu to skip scanning.\n");
            }
        }
    }
    /*while(isbusy());
    readflash(100,0x00,0x00,0x00,data);
    data[100] = '\0';
    printf("READ2:%s\n",data);*/
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
    Serial.println("End!");
    exit(0);
}
void loop(){
    
}
