/*
 * File:   EM_LED_ESP32.ino
 * Author: Nilesh Mundphan
 *
 * Created on April 1, 2021, 11:03 PM
 */

#include "LEDMatrix.h"
#include "FreeMonoBold12pt7b.h"
#include "KRDEV_ALL_08.h"
#include "FS.h"
#include "SPIFFS.h"
#define FORMAT_SPIFFS_IF_FAILED true

#include <WiFi.h>
#include <WebSocketsServer.h>

#define SSID          "LEDDISPLAY"     // Name of the access point
#define SSID_PASSWORD "1234567890"     // Password for the access point

WebSocketsServer webSocket = WebSocketsServer(8080);

#define MAX_MSG_SIZE 128
#define YAXISOFFSET   14

int     curser      = 0;
int     menu_index  = 0;
int     brightness  = 0;

int     x           = 0;
int     last_x      = 0;

long    previousMillis  = 0;
long    interval        = 1000;
long    msg_speed       = 25;
long    msg_pause       = 1000;

char    display_message[MAX_MSG_SIZE];
char    default_msg[] = "Embedded Makers";

boolean scroll_enable = true;
boolean displayon = false;
boolean wifi_data = false;
size_t msg_len;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

LEDMatrix disp(2,4,5,18,19,21);//(EN,A,B,SH,ST,DS)

void Log(String str){
  Serial.println(String("LOG: ") + str);
}

void Data(String str){
  Serial.println(String("DATA:") + str);
} 


void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\r\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("- failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void testFileIO(fs::FS &fs, const char * path,uint8_t *buf,size_t len){
    Serial.printf("Testing file Write I/O with %s\r\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    file.write(buf, len);
    file.close();
}
void testFileIOR(fs::FS &fs, const char * path){
    Serial.printf("Testing file Read I/O with %s\r\n", path);
    File file = fs.open(path);
    
    if(!file){
        Serial.println("- failed to open file for writing");
        return;
    }
    if(file.size()>1){
      uint8_t buf[file.size()];
      Serial.printf("File Size %d\r\n", file.size());
      file.read(buf, file.size());
      disp.update_tbuff(buf,file.size());
      msg_len=file.size()/2;
      wifi_data=true;
    }        
    file.close();
}


void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\r\n", path);
    if(fs.remove(path)){
        Serial.println("- file deleted");
    } else {
        Serial.println("- delete failed");
    }
}
void IRAM_ATTR onTimer() {
    //uint32_t start = micros();
    portENTER_CRITICAL_ISR(&timerMux);
    disp.update_disp();
    portEXIT_CRITICAL_ISR(&timerMux);
    //uint32_t end = micros() - start;
    //Serial.printf(" Refresh %d us\r\n",end);
    //Serial.println("Refresh Display"); 
}

void Timer_Setup(void) {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 400, true);
    timerAlarmEnable(timer);
}

void display_setup(){
    disp.init();
    disp.brightness(50);    
    disp.setFont(&FreeMonoBold12pt7b);    
    disp.DisplayClear();
    disp.setCursor(0,YAXISOFFSET);
}

void setup(){
    delay(3000);
    Serial.begin(115200);  //Starts Serial Connection 
    if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)){
        Serial.println("SPIFFS Mount Failed");
        return;
    }
    listDir(SPIFFS, "/", 0);
    //deleteFile(SPIFFS,"/test.txt");
    testFileIOR(SPIFFS,"/msg.txt");
    display_setup();
    Serial.println("Stated Main Code");
    WiFi.softAP(SSID,SSID_PASSWORD);
    IPAddress myIP = WiFi.softAPIP();
    Log(String("IP Address: ") + IP2String(myIP));
    webSocket.onEvent(webSocketEvent);
    webSocket.begin();
    delay(3000);    
    Timer_Setup();
    displayon=true;
    getMessage();
    //disp.setCursor(0,0);    
}

void loop(){
    webSocket.loop();
    if(displayon){       
        unsigned long currentMillis = millis();     
        if(currentMillis - previousMillis > interval){
            previousMillis = currentMillis;       
            
            if(scroll_enable){   
                if(x == 0){
                    interval=msg_pause;
                }
                else{
                    interval=msg_speed;
                }
               
                if(x < last_x)
                {
                        disp.DisplayClear();
                        disp.cpy_buff(x++);       
                }else{
                        getMessage();
                }
            }
            else{
                getMessage();
            }
        }
    } 
}
void getMessage(){
  if(wifi_data){
    x=0;
    disp.setCursor(0,0);
    last_x=msg_len;
  }else{
    strcpy(display_message,default_msg);
    disp.DisplayClear();
    disp.setCursor(0,YAXISOFFSET);
    disp.print_line(display_message);
    x=0;
    last_x=disp.cursor_x;
    disp.cpy_buff(0);
  }
}
String IP2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ;
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    //Serial.printf("\nEvent %i\n",type);
    if (timer) {
      // Stop and free timer
      timerEnd(timer);
      timer = NULL;
    }  
    switch(type) {
        case WStype_DISCONNECTED:
            Log("Disconnected! " + String(num));
            break;
        case WStype_CONNECTED: {
              
              IPAddress ip = webSocket.remoteIP(num);
              Log("Connected to :" + IP2String(ip));
              // send message to client
              webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT: {
            Log("Recived date len :" + String(length));
            Data(String("#")+String((char*)payload)+String("$"));                
        }
        break;
        case WStype_BIN:{
            Log("Recived BIN len :" + String(length)); 
            disp.update_tbuff(payload,length);
            testFileIO(SPIFFS,"/msg.txt",payload,length);    
            testFileIOR(SPIFFS,"/msg.txt");             
            msg_len=length/2;
            wifi_data=true;
        }
        break;
    }    
  Serial.println();
  Timer_Setup();
}