/*
 * DCF77_SerialTimeOutput
 * Ralf Bohnen, 2013
 * This example code is in the public domain.
 */

/*---------------------------------------------------------------*/
/*include libraries----------------------------------------------*/
/*---------------------------------------------------------------*/
#include "DCF77.h"
#include "Time.h"
#include "Timezone.h"

/*---------------------------------------------------------------*/
/*initialize variables-------------------------------------------*/
/*---------------------------------------------------------------*/
#define DCF_PIN 8           // NEEDS TO BE SET TO PIN 2  Connection pin to DCF 77 device
#define DCF_INTERRUPT 0    // Interrupt number associated with pin

time_t time;
DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT);

char time_s[9];
char date_s[11];

int i = 0;
int j = 0;
int intMin = 0;
int intHour = 0;
int codeDCF[] = {1,2,4,8,10,20,40};
int arrayLength = 59;
int code[59];
int flankUp = 0;
int flankDown = 0;
int PreviousflankUp;
int typePulse = 0;       /* 0: LOW     1: HIGH*/
int durationPulse = 0;   /* duration of sensed signal pulse*/
int durationCycle = 0;    /* duration of cycle between two rising flanks*/
int constHigh = 200;   /* default value for HIGH signal*/
int constLow = 100;    /* default value for LOW signal*/
int constNew = 1500;   /* default value for detection of new minute*/
bool Up = false;
bool flagError = 0;
bool flagNewMinute = 0;

/*---------------------------------------------------------------*/
/*setup script---------------------------------------------------*/
/*---------------------------------------------------------------*/
void setup() {
  Serial.begin(9600); 
  DCF.Start();
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DCF_PIN, INPUT);
  Serial.println("Rebooting clock...");
}

/*---------------------------------------------------------------*/
/*iloop script---------------------------------------------------*/
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
        Serial.print("Sek: ");
        Serial.print(i);
        Serial.print("\t\tCycle: ");
        Serial.print(durationCycle);
        durationPulse = flankDown - flankUp;
        Serial.print("\tPulse :");
        Serial.print(durationPulse);
        PreviousflankUp = flankUp;
        if (durationPulse > (constHigh+constLow)/2)
          {typePulse = 1;
          }
        else if (durationPulse < 50)
        {
          typePulse = 99;  
        }
        else
        {
          typePulse = 0;
          };
      
        Up = false;
      Serial.print("\tSignal: ");
      Serial.print(typePulse);
      time_t DCFtime = DCF.getTime(); // Nachschauen ob eine neue DCF77 Zeit vorhanden ist
      if (typePulse == 99)
      {
        flagNewMinute = false;
          for(int j = 0; j < 59; j++)
          {
            code[j]=0;
          }
          i=0;
        Serial.println("\n--------------------- ERROR DETECTED! ---------------------");
        };
      if (durationCycle > constNew)
      {
        if(flagNewMinute)
        {
          Serial.println("\n----------------- Sucessful run -----------------");
          
          intMin = decodeMin(code);
          intHour = decodeHour(code);

          Serial.print("Es ist : ");
          Serial.print(intHour);
          Serial.print(":");
          Serial.print(intMin);
          Serial.println(" Uhr :-)");
          
          for(int j = 0; j < 59; j++) /*Code zurücksetzen*/
          {
            code[j]=0;
          }
          }
        flagNewMinute = true;
        i = 0;
        Serial.print("\n------------------- New minute detected -------------------\t");
        }

    Serial.print("\t\tCode: ");
    if (flagNewMinute)
    {
      code[i] = typePulse;
      i++;
      for(int j = 0; j < arrayLength; j++)
        {
          Serial.print(code[j]);
        }
      };
    Serial.println();
    digitalWrite(LED_BUILTIN, LOW);
            
  if (DCFtime!=0)
  {
    setTime(DCFtime); //Neue Systemzeit setzen
    Serial.print("Neue Zeit erhalten : "); //Ausgabe an seriell
    Serial.print(sprintTime()); 
    Serial.print("  "); 
    Serial.println(sprintDate());   
  } 
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

int decodeMin(int code[59]){
  int m = 0;
  for (int j=0;j<7;j++){
     m = m + code[j+21]*codeDCF[j];
  }
  return m;
}

int decodeHour(int code[59]){
  int h = 0;
  for (int j=0;j<6;j++){
    h = h+code[j+29]*codeDCF[j];
  }
  return h;
}
