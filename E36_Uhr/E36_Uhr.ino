/*
 * Skript for a real time clock within an old speedometer
 */

//*---------------------------------------------------------------*/
/*include libraries----------------------------------------------*/
/*---------------------------------------------------------------*/
#include "Time.h"
#include "Timezone.h"

/*---------------------------------------------------------------*/
/*initialize variables-------------------------------------------*/
/*---------------------------------------------------------------*/
#define DCF_PIN 2           // Connection pin to DCF 77 device

time_t time;

bool Up = false;
bool flagError = 0;
bool flagNewMinute = 0;

char time_s[9];
char date_s[11];

int i = 0;
int j = 0;
int intMin = 0;
int intHour = 0;
int intDay = 0;
int intWeekDay = 0;
int intMonth = 0;
int intYear = 0;
int codeDCF[] = {1,2,4,8,10,20,40,80}; /*DCF77 decoding template*/
int arrayLength = 59;
int code[59];
int flankUp = 0;          /* 0: LOW     1: HIGH*/
int flankDown = 0;        /* 0: LOW     1: HIGH*/
int PreviousflankUp;      /* 0: LOW     1: HIGH*/
int typePulse = 0;        /* 0: LOW     1: HIGH*/
int durationPulse = 0;    /* duration of sensed signal pulse*/
int durationCycle = 0;    /* duration of cycle between two rising flanks*/
int constHigh = 200;      /* default value for HIGH signal*/
int constLow = 100;       /* default value for LOW signal*/
int constNew = 1500;      /* default value for detection of new minute*/

/*---------------------------------------------------------------*/
/*setup script---------------------------------------------------*/
/*---------------------------------------------------------------*/
void setup() {
  Serial.begin(9600); 
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Rebooting clock. This may take several minutes.");
}

/*---------------------------------------------------------------*/
/*loop script---------------------------------------------------*/
/*---------------------------------------------------------------*/
void loop() {

  int sensorValue = digitalRead(DCF_PIN);
    if (sensorValue) {
      if (!Up) {
        flankUp=millis();
        Up = true;
        digitalWrite(LED_BUILTIN, HIGH);
      }
    } else {
      if (Up) {
        flankDown=millis();
        durationCycle = flankUp-PreviousflankUp;
        durationPulse = flankDown - flankUp;
        PreviousflankUp = flankUp;
        if (durationPulse > (constHigh+constLow)/2)
          {typePulse = 1;
          }
        else if (durationPulse < 50)
        {
          typePulse = 99;
          flagNewMinute = false;
          for(int j = 0; j < 59; j++) /*Reset Code*/
          {
            code[j]=0;
          }
          i=0;
        }
        else
        {
          typePulse = 0;
         };
      
        Up = false;
      if (durationCycle > constNew)
      {
        if(flagNewMinute)
        {
          intMin = decodeDCF(code,21,7);
          intHour = decodeDCF(code,29,6);
          intDay = decodeDCF(code,36,6);
          intWeekDay = decodeDCF(code,42,3);
          intMonth = decodeDCF(code,45,5);
          intYear = 2000+decodeDCF(code,50,8);

          setTime(intHour,intMin,0,intDay,intMonth,intYear);
          Serial.print("Current time is : ");
          Serial.print(sprintTime()); 
          Serial.print("  "); 
          Serial.println(sprintDate());  
          
          for(int j = 0; j < 59; j++) /*Code zurücksetzen*/
          {
            code[j]=0;
          }
          }
        flagNewMinute = true;
        i = 0;
        }

    if (flagNewMinute)
    {
      code[i] = typePulse;
      i++;
      };
    digitalWrite(LED_BUILTIN, LOW);
 
      }              
    }
  
}

char* sprintTime() {
  snprintf(time_s,sizeof(time_s),"%.2d:%.2d:%.2d" , hour(), minute(), second());
  time_s[strlen(time_s)] = '\0';
  return time_s;
}

char* sprintDate() {
  snprintf(date_s,sizeof(date_s),"%.2d.%.2d.%.4d" , day(), month(), year());
  date_s[strlen(date_s)] = '\0';
  return date_s;
}

int decodeDCF(int code[59],int startBit,int nBit){
  int res = 0;
  for (int j=0;j<nBit;j++){
    res=res+code[startBit+j]*codeDCF[j];
    }
  return res;
}
