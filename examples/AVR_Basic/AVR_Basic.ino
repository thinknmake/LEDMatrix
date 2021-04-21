#include "TimerOne.h"
#include "LEDMatrix.h"
#include "FreeMonoBold12pt7b.h"
#include "KRDEV_ALL_08.h"

//LEDMatrix disp(14,2,0,4,5,16);
LEDMatrix disp;
char msg1[32]="EMBEDDED MAKERS";
char msg3[32]="HELLO AND WELCOME";
char msg2[32]="gkfnZd vfHkuanu";

char display_msg[128]={0};

int x=0,last_x=0;

long previousMillis = 0;
long interval = 1000;

uint8_t messgess=0;

void callback()
{
    disp.update_disp();
}

void timerinit()
{
  Timer1.initialize(2000);           // initialize timer1, and set a 1/2 second period
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
}

void setup() {
      Serial.begin(115200);
      disp.init();
      Serial.println("Stated Code");
      disp.setFont(&FreeMonoBold12pt7b);
      //disp.print_str((char*)"Nilesh");
      //disp.setFont(&Kruti_Dev_040__Bold12pt7b);
      strcpy(display_msg,msg1);
      timerinit();
}

void loop() {
      unsigned long currentMillis = millis();
      if(currentMillis - previousMillis > interval) {
          previousMillis = currentMillis;

          if(x==0){
              if(messgess==0)
              {
                disp.setFont(&FreeMonoBold12pt7b);
                strcpy(display_msg,msg1);
                disp.DisplayClear();
                disp.print_line(display_msg);
                messgess=1;
              }
              else if(messgess==1)
              {
                disp.setFont(&KRDEV_ALL_08);
                strcpy(display_msg,msg2);
                disp.DisplayClear();
                disp.print_line(display_msg);
                messgess=2;
              }
              else if(messgess==2)
              {
                  disp.setFont(&FreeMonoBold12pt7b);
                  strcpy(display_msg,msg3);
                  disp.DisplayClear();
                  disp.print_line(display_msg);
                  messgess=0;
              }
              last_x=disp.cursor_x;
              interval=1000;

              //delay(1000);
          }
          else{
              interval=40;
              //delay(100);
          }

          disp.setCursor(x--,14);
          disp.DisplayClear();
          disp.print_line(display_msg);

          if(-x > (last_x)){
              x=0;
          }
      }
}