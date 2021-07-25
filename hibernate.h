#ifndef _HIBERNATE_H
#define _HIBERNATE_H

#include <Snooze.h>

SnoozeTouch touch;
SnoozeDigital digital;
SnoozeAlarm  alarm;
SnoozeBlock config_teensy36(touch, digital, alarm);

uint16_t duration_seconds;
uint16_t period_seconds;

void setWakeupCallandSleep(uint32_t nsec)
{
   
   Serial.print("Go to sleep ");
   Serial.print(nsec);
   Serial.println(" seconds");
   delay(500);
   
   if(nsec<60) alarm.setRtcTimer(0, 0, nsec);
   else{
    tmElements_t tx = seconds2tm(nsec);
    alarm.setRtcTimer (tx.Hour,tx.Minute,tx.Second);
   }

   Snooze.deepSleep( config_teensy36 );

}

void setWakeupCallandSleep(periodic_save_parameters *PSP)
{  

   tmElements_t timefromtm;

   timefromtm.Year = PSP->year_from;
   timefromtm.Month = PSP->month_from;
   timefromtm.Day = PSP->day_from;
   timefromtm.Hour = PSP->hour_from;
   timefromtm.Minute = PSP->minute_from;
   timefromtm.Second = PSP->second_from;


   uint32_t timefrom = tm2seconds(&timefromtm);
   
   uint32_t nsec = timefrom - now();
   Serial.println(nsec);
   delay(500);
  
   if(nsec<60) {
    alarm.setRtcTimer(0, 0, nsec);
   }
   else{
    tmElements_t tx = seconds2tm(nsec);
    alarm.setRtcTimer (tx.Hour,tx.Minute,tx.Second);
   }

   Snooze.deepSleep( config_teensy36 );

}

//record end?, compare rtc with DHE (date and hour end)
uint8_t checkDHE(periodic_save_parameters *PSP){
  
  uint32_t tt = now();
  tmElements_t tx = seconds2tm(tt);
  
  if(tx.Year == PSP->year_to && tx.Month == PSP->month_to && tx.Day == PSP->day_to
  && tx.Hour == PSP->hour_to && tx.Minute == PSP->minute_to && tx.Second >= PSP->second_to)
    return 1;
  else 
    return 0;
    
}

uint32_t checkDutyCycle(periodic_save_parameters *PSP, uint8_t state){
  
  static uint8_t dhe;
  static uint8_t gosleep;
  static uint8_t startrec;
  static uint8_t endrec;
  
  uint32_t tt = now();
  static uint32_t t_start = tt;

  duration_seconds = PSP->duration_seconds;
  period_seconds = PSP->period_seconds;

  if(startrec) {t_start=tt; startrec=0;}

  if (!dhe) dhe=checkDHE(PSP);
  if(dhe && !endrec) {endrec=1; return -2;}
  
  if(tt > t_start + duration_seconds && state>0){
    Serial.println("close acquisition");
    t_start = tt+period_seconds;
    gosleep=1;
    return -1;
  }
  if(gosleep==1 && state==0){
    gosleep=0;
    if(endrec) {startrec=1; endrec=0; dhe=0;}
    return period_seconds;
  }
  return 0;
  
}

uint8_t checkDutyCycle (uint32_t record_time){
  
  uint32_t tt = now();
  static uint32_t t_start = tt;
  static uint8_t endrec;
  
  if(endrec) {t_start=tt; endrec=0;}

  if(tt-t_start > record_time) {endrec=1; return 1;}
  else return 0;
  
}

#endif
