/*
 * File:   DisplayARM.ino
 * Author: Nilesh Mundphan
 *
 * Created on FEB 27, 2021, 11:03 PM
 */

#include <Wire.h>
#include <ArduinoJson.h>
#include <RTClock.h>
#include "LEDMatrix.h"
#include "FreeMonoBold12pt7b.h"
#include "KRDEV_ALL_08.h"
#include "MDATA.h"

RTClock rtclock (RTCSEL_LSE); // initialise
time_t tt;
tm_t mtt;

long bfu,afu; 
HardwareTimer timer(2);
HardwareTimer timerpwm(1);
LEDMatrix disp(PA8,PA15,PB3,PB4,PB8,PB9);

#define WIFISerial Serial3

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

long    tpreviousMillis  = 0;
long    tinterval        = 500;
boolean scroll_enable = true;
boolean displayon = false;
int rxBuffer_index=0;
uint8_t start=0;

DynamicJsonDocument doc(MAX_DATA_SIZE);
char time_str[16],tmp_time_str[16];
char date_str[16];
char display_message[MAX_MSG_SIZE];
char default_msg[128] = "Embedded Makers";

void Display_Update(){
    timer.pause();
    disp.update_disp();
    timer.refresh();
    timer.resume();
}

void TimerSetup()
{      
    timer.pause();
    //timer.setPrescaleFactor(10000);
    timer.setPrescaleFactor(5);
    timer.setOverflow(7200);
    timer.setChannel1Mode(TIMER_OUTPUT_COMPARE);
    timer.setCompare(TIMER_CH1,7200);
    timer.attachCompare1Interrupt(Display_Update);
    timer.refresh();
    timer.resume(); 
}

void display_setup(){
    pinMode(PA8,PWM);
    disp.init();
    disp.brightness(50);    
    disp.setFont(&FreeMonoBold12pt7b);    
    disp.DisplayClear();
    disp.setCursor(0,YAXISOFFSET);
    TimerSetup(); 
}

void setup() {
    
    Serial.begin(115200);  //Starts Serial Connection 
    WIFISerial.begin(9600);
    timerpwm.setPrescaleFactor(1);
    timerpwm.setOverflow(1024);
    display_setup();    
    
    msg_index=0;
    disp.DisplayClear(); 
    
    Serial.println("Stated Main Code");
    WIFISerial.flush();
    displayon=true;
    Wire.begin(I2C_FAST_MODE);  
    //cleanEEPROM();
    pinMode(BUZZER,OUTPUT);
    pinMode(LED1,OUTPUT);
    pinMode(LED2,OUTPUT);
    digitalWrite(BUZZER,HIGH);
    digitalWrite(LED1,HIGH);
    digitalWrite(LED2,HIGH);    
    delay(500);
    digitalWrite(BUZZER,LOW);
    digitalWrite(LED1,LOW);
    digitalWrite(LED2,LOW);
    ReadEEPROM();
    checkMessages();
    getMessage();
    last_x=disp.cursor_x;           
}

void loop(){
    WIFIAccess();

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
                    
                    if(Messages[msg_index].Type=='T'){
                        getTime();
                        
                        if(!strstr(tmp_time_str,time_str))
                        {
                            strcpy(tmp_time_str,time_str);
                            strcpy(display_message,time_str);
                            disp.setCursor(0,YAXISOFFSET);
                            disp.print_line(display_message);
                           
                        }   
                    } 

                    disp.cpy_buff(x++);    
                                           
               }else{
                    x=0;
                    checkMessages();
                    getMessage();
                    last_x=disp.cursor_x;
               }
            }else{
                checkMessages();
                getMessage();
            }
     }
  }
}

void getMessage(){
    if(Numberofmsg != 0 ){
        if(Messages[msg_index].Type=='D'){
            getDate();
            strcpy(display_message,date_str);
        }else if(Messages[msg_index].Type=='T'){
            getTime();
            strcpy(display_message,time_str);
            strcpy(tmp_time_str,time_str);
                        
        }else{

            strcpy(display_message,Messages[msg_index].Message);
            for (int i=0; display_message[i] != '\0';i++)
            {
                if(display_message[i] == -86 && display_message[i-1] == -62)
                {
                    display_message[i]=170;
                    for(int j=i+1; display_message[j] != '\0';j++){
                            display_message[j]=display_message[j+1];
                    }
                }
            }
        }

        if(Messages[msg_index].Type=='M'){
            disp.setCursor(0,YAXISOFFSET-3);
        }else{
            disp.setCursor(0,YAXISOFFSET);
        }
                          
        disp.DisplayClear();    
        disp.print_line(display_message);
        disp.cpy_buff(0);
                
    }else{
        strcpy(display_message,default_msg);
        disp.DisplayClear();
        disp.print_line(display_message);
        disp.cpy_buff(0);                
    }
}

void checkMessages(){
    if(Numberofmsg != 0 ){
        setDisplayForMessage();   
    }else{
        disp.setFont(&FreeMonoBold12pt7b);
        msg_speed=25;
        disp.brightness(50);        
    }
}

void setDisplayForMessage(){
    
    msg_index++;
    
    if(msg_index ==  Numberofmsg)
        msg_index=0;
    
    Serial.println();  
    Log(String("Message Index: ") +String(msg_index)); 
    displayMessage(Messages[msg_index]);
    
    if(Messages[msg_index].Type=='M'){
        //disp.setFont(&Kruti_Dev_040__Bold12pt7b);
        //disp.setFont(&KDEV08);
        disp.setFont(&KRDEV_ALL_08);
    }

    else{
        disp.setFont(&FreeMonoBold12pt7b);  
    }

    if(Messages[msg_index].Speed=='M'){
        interval = 500;
        scroll_enable = true;
        msg_speed=50;
    }else if(Messages[msg_index].Speed=='F'){
        interval = 500;
        scroll_enable = true;
        msg_speed=25;
    }else{
        interval = 5000;
        scroll_enable = false;
        //getMessage();
    }
    
    if(Messages[msg_index].Bright=='M'){
        disp.brightness(50);
    }else if(Messages[msg_index].Bright=='H'){
        disp.brightness(100);
    }else{
        disp.brightness(5);
    }
        
}

void getTime(){
    Serial.println("Reading RTC Time");
    rtclock.breakTime(rtclock.now(), mtt);
    sprintf(time_str, "%d:%d:%d",     //%d allows to print an integer to the string
        mtt.hour,     //get hour method
        mtt.minute,   //get minute method
        mtt.second    //get second method
    );

    Serial.println(time_str);

    sprintf(date_str, "%d:%d:%d",     //%d allows to print an integer to the string
        mtt.day,      //get daymethod
        mtt.month,    //get month method
        mtt.year+1970      //get year method
    );

    Serial.println(date_str);

}

void getDate(){
    Serial.println("Reading RTC Date");
    rtclock.breakTime(rtclock.now(), mtt);
    sprintf(date_str, "%d:%d:%d",     //%d allows to print an integer to the string
        mtt.day,      //get daymethod
        mtt.month,    //get month method
        mtt.year+1970      //get year method
    );

    Serial.println(date_str);
}

void WIFIAccess(){
  if(WIFISerial.available()){
    char ch =WIFISerial.read();
    //Serial.write(ch);
    if(ch=='$'){
        Serial.println(rxBuffer);
        digitalWrite(BUZZER,HIGH);
        if(strstr(rxBuffer,"RTCUpdate")){
            updateDate(rxBuffer);
        }else{
            decodeData(rxBuffer);
            checkMessages();
            getMessage();
        }
        start=0;
        WIFISerial.flush();
        digitalWrite(BUZZER,LOW);           
        timer.refresh();
        timer.resume();
        displayon=true;
    }
    
    if(start){
        rxBuffer[rxBuffer_index++]=ch;    
    }
    
    if(ch=='#'){
        start=1;
        rxBuffer_index=0;
        displayon=false;
        timer.pause();
    }     
  }
}

void updateDate(String json){
    DeserializationError error = deserializeJson(doc, json);
    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    
    int hour = doc["RTCUpdate"]["Hour"];
    int min = doc["RTCUpdate"]["Min"];
    int sec = doc["RTCUpdate"]["Sec"];
    int day = doc["RTCUpdate"]["Day"];
    int month = doc["RTCUpdate"]["Month"];
    int year = doc["RTCUpdate"]["Year"];
    
    Log("Date Time RTC Update");
    Log(String("Time: ") + String(hour) + String(":")+ String(min) + String(":")+ String(sec));
    Log(String("Time: ") + String(day) + String("-")+ String(month) + String(":")+ String(year));
    //rtc.adjust(DateTime(year, month, day, hour, min, sec));
    mtt.month = month;
    mtt.day = day;
    mtt.year = year-1970;
    mtt.hour = hour;
    mtt.minute = min;
    mtt.second = sec;
    tt = rtclock.makeTime(mtt);
    rtclock.setTime(tt);
}


void decodeData(String json){
    DeserializationError error = deserializeJson(doc, json);
    // Test if parsing succeeds.
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    Numberofmsg = doc["entries"];
    Log(String(Numberofmsg));

    for(int i=0;i<Numberofmsg;i++){

        String Mtype = doc["messages"][i]["MType"];
        String MSpeed = doc["messages"][i]["MSpeed"];
        String MBright = doc["messages"][i]["MBright"];
        String Message = doc["messages"][i]["Message"];

        Messages[i].Type=Mtype.charAt(0);
        Messages[i].Speed=MSpeed.charAt(0);
        Messages[i].Bright=MBright.charAt(0);
        Message.toCharArray(Messages[i].Message,Message.length()+1);

        Log(Mtype + MSpeed + MBright);
        Log(Message);   
    }
    
        cleanEEPROM();
        delay(20);
        UpdateEEPROM();
        delay(20);
        ReadEEPROM();
           
        msg_index=0;
        x=0;         
}



