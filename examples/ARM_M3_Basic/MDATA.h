#include <Wire.h>
#define BUZZER  PA0
#define LED1    PB12
#define LED2    PB13

#define EEPROMCHIP 0x50
#define MAX_MSG_SIZE 128
#define MAX_DATA_SIZE 1100

#define MAX_MSG 5

int     msg_index     = 0;
int     Numberofmsg=0;
struct MessageData {
    char Type;
    char Speed;
    char Bright;
    char Message[MAX_MSG_SIZE];
};

MessageData Messages[MAX_MSG];
char rxBuffer[MAX_DATA_SIZE];

void Log(String str){
    Serial.println(String("LOG: ") + str);
}

void Data(String str){
    Serial.println(String("DATA:") + str);
}

void displayMessage(MessageData msg){
    Log("------------------");
    Log(String("Type: ")+String(msg.Type)+String(" Speed: ")+String(msg.Speed)+String(" Bright: ")+String(msg.Bright));
    Log(String(msg.Message));   
}

template <class T> int checksum(const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i,ch=0;
    for (i = 0; i < sizeof(value); i++){
          ch+=*p++;
    }
    return ch;
}

void EEPROM_Write(int address,uint8_t wbyte){
  
    Wire.beginTransmission(EEPROMCHIP);
    Wire.write((int)(address >> 8));   // MSB
    Wire.write((int)(address & 0xFF)); // LSB
    Wire.write(wbyte);
    Wire.endTransmission();
    delay(5);
}

uint8_t EEPROM_Read(int address){
    
    byte rdata = 0xFF;
    //delay(5);
    Wire.beginTransmission(EEPROMCHIP);
    Wire.write((int)(address >> 8));   // MSB
    Wire.write((int)(address & 0xFF)); // LSB
    Wire.endTransmission();
   
    Wire.requestFrom(EEPROMCHIP,1);
   
    if (Wire.available()) rdata = Wire.read();
  
    return rdata;
}

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++){
          EEPROM_Write(ee++, *p++);
    }
    //EEPROM.commit();
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
    {
          *p++ = EEPROM_Read(ee++);
    }
    return i;
}

void UpdateEEPROM(){
    int checksumval = checksum(Messages);
    Log(String("Messages: ")+ String(Numberofmsg));        
    Log("Data Checksum :" + String(checksumval)); 
    EEPROM_writeAnything(2,Numberofmsg);
    EEPROM_writeAnything(20,Messages);     
    EEPROM_writeAnything(10,checksumval);
}

void ReadEEPROM(){
    int ids,rchecksum;
    Log(String("Reading EEPROM"));
    EEPROM_readAnything(2,ids);
    EEPROM_readAnything(20,Messages);
    EEPROM_readAnything(10,rchecksum);
    
    Log("No Messages   :" + String(ids));
    Log("Read Checksum :" + String(rchecksum));
    Log("Cal  Checksum :" + String(checksum(Messages)));
        
    if( ids > 0 && rchecksum == checksum(Messages)){
        digitalWrite(LED2,HIGH);
        delay(700);
        digitalWrite(LED2,LOW);
        Log("Matched Checksum !!!");       
        Numberofmsg=ids;
        Log(String("Messages: ")+ String(ids));
        for(int i=0;i<ids;i++){
            displayMessage(Messages[i]);
        }
    }
    else{
        Numberofmsg=0;
        msg_index=0;
        digitalWrite(LED1,HIGH);
        delay(700);
        digitalWrite(LED1,LOW);        
        Log("Checksum Failed");
    }    
}

//EEPROM Access Codes
void cleanEEPROM(){
    for(int i=0;i<MAX_DATA_SIZE;i++){
        EEPROM_Write(i,0);
    }
}

