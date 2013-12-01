/*const uint8_t valid_devi_id[] = {0x15};
const uint8_t valid_mamm_id[] = {0xEF};*/
struct chip {
    int devi_id;
    int manu_id;
    unsigned long long size;
    const char *name;
};
struct gchip {
    int manu_id;
    int memo_id;
    int capa_id;
    unsigned long long size;
    const char *name;
};
boolean show_process;
/*struct chip *new_chip(int devi_id,int mamm_id,char name_i[]){
    struct chip *newchip = (struct chip*)malloc(sizeof(struct chip));
    newchip->devi_id = devi_id;
    newchip->mamm_id = mamm_id;
    newchip->name = (char *)malloc(strlen(name_i));
    strcpy(newchip->name,name_i);
    return newchip;
}*/
struct chip chips[] = {{0x14,0xEF,2UL*1024*1024,"Winbond W25Q16"},{0x15,0xEF,4UL*1024*1024,"Winbond W25Q32"},{0x16,0xEF,8UL*1024*1024,"Winbond W25Q32"},{0x17,0xEF,16*1024*1024,"Winbond W25Q128"}};
struct gchip gchips[] = {{0xEF,0x40,0x16,4UL*1024*1024,"Winbond W25Q32"}};
char help_text[] = "Hello,there are some tips:\n\
spi_flash [-s] [-w <file>] [-e] [-r [file]] [-v <file>] [-1 <address1>][-2 <address2>][-3 <address3>] [-h] [-l <length>] [-y] [-f] [-b] [-t] [-d] [-i]\n\n\
arg:\n\
-s        skip ID scanning\n\
-w <file> a file to write to flash\n\
-e        skip erasing chip[warning!]/erase flash(when there is no anything else need to do)\n\
-r [file] write to a file\n\
-v <file> verifying data\n\
-1 <address> address1\n\
-2 <address> address2\n\
-3 <address> address3\n\
-h        show what the file is\n\
-l <length> length(default is all of the flash or file)\n\
-y        no-hand-needed\n\
-f        fill to zero on read(255 -> 0,on text)\n\
-b        binary file\n\
-t        have a flash test\n\
-d        display process\n\
-i        read winbond id\n\
By TLL,GPL licence.\n\
Email:1040424979@qq.com QQ:1040424979\n\
A young boy who is using mac and cubieboard.";
int check_id(int devi,int manu){
    /*int a = 0;
    for(int i=0;i<sizeof(valid_devi_id)/sizeof(uint8_t);i++){
        if(devi == valid_devi_id[i]){
            a=1;
            break;
        }
    }
    if(a == 0)return false;
    for(int i=0;i<sizeof(valid_mamm_id)/sizeof(uint8_t);i++){
        if(mamm == valid_mamm_id[i]){
            a=0;
            break;
        }
    }
    if(a != 0)
        return false;
    else
        return true;*/
    for(int i=0;i<sizeof(chips)/sizeof(chips[0]);i++){
        if(chips[i].devi_id == devi){
            if(chips[i].manu_id == manu){
                return i;
            }
        }
    }
    return -1;
}
int check_gid(int memo,int manu,int capa){
    /*int a = 0;
     for(int i=0;i<sizeof(valid_devi_id)/sizeof(uint8_t);i++){
     if(devi == valid_devi_id[i]){
     a=1;
     break;
     }
     }
     if(a == 0)return false;
     for(int i=0;i<sizeof(valid_mamm_id)/sizeof(uint8_t);i++){
     if(mamm == valid_mamm_id[i]){
     a=0;
     break;
     }
     }
     if(a != 0)
     return false;
     else
     return true;*/
    for(int i=0;i<sizeof(gchips)/sizeof(gchips[0]);i++){
        //printf("%x %x\n",gchips[i].memo_id,memo);
        if(gchips[i].memo_id == memo){
            if(gchips[i].manu_id == manu){
                if(gchips[i].capa_id == capa){
                    return i;
                }
            }
        }
    }
    return -1;
}
boolean issame(char *bufa,char *bufb,long long len1,long long len2){
    printf("File size:%lld Flash size:%lld\n",len1,len2);
    //if(len1 != len2)return 0;
    if(len1 > len2){
        printf("Invalid file size!\n");
        return 0;
    }
    for(long long a=0;a<len1;a++){
        if(show_process)
            printf("Location:%llx File:%x Flash:%x\n",a,*bufa,*bufb);
        if(*bufa != *bufb){
            return 0;
        }else{
            bufa++;
            bufb++;
        }
    }
    return 1;
}