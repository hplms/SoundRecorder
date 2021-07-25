/* Sound Recorder for Teensy 3.6
 * Author: Händel Palmés
 * Based on Walter Zimmer's Micro Sound Recorder code, adapted and modified for my final degree project.
 */
 
#include <Audio.h>
#include <TimeLib.h>
#include "config.h"
#include "sd_time.h"
#include "hibernate.h"

AudioInputI2S            i2s;                    
AudioRecordQueue         queue;                   
AudioConnection          patchCord(i2s, 0, queue, 0); 

//FFT
AudioPlaySdWav           playSdWav;             
AudioAnalyzeFFT1024      fft1024;            
AudioConnection          patchCord1(playSdWav, 0, fft1024, 0);

//AUDIO BOARD
AudioControlSGTL5000     sgtl5000; 

const int myInput = AUDIO_INPUT_MIC;

char serial_buffer[128];
int i;

uint8_t dhs; //date and hour start
uint8_t rec;
uint8_t norec;
periodic_save_parameters PSP;

uint8_t singlerec;
uint32_t singlerect;

time_t getTeensy3Time(){  return Teensy3Clock.get();}

void setup() {

  setSyncProvider(getTeensy3Time);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } 

  // Audio connections require memory, and the record queue
  // uses this memory to buffer incoming audio.
  AudioMemory(60);

  // Enable the audio shield, select input, and enable output
  sgtl5000.enable();
  sgtl5000.inputSelect(myInput);
  sgtl5000.volume(0.5);

  uSD.init();

  Serial.begin(19200);
  while(!Serial){}
  
  Serial.println("COMMANDS:");
  Serial.println();
  Serial.println("periodic_save [date_and_start_time][date_and_end_time][duration_seconds][period_seconds]");
  Serial.println("date and hour format [dd/mm/yyyy-hh:mm:ss]");
  Serial.println("It records the amount of time indicated by the duration_seconds parameter in the time interval defined ");
  Serial.println("by date_and_start_time and date_and_end_time in periods of period_seconds.");
  Serial.println();
  Serial.println("single_save [duration_seconds]");
  Serial.println("It records the amount of seconds indicated by the duration_seconds parameter.");
  Serial.println();
  Serial.println("name_session [session]");
  Serial.println("It creates a folder in SD with the name of the session, the next records shall be located there. ");
  Serial.println();
  Serial.println("list [session]");
  Serial.println("It list files and sessions of root if nothing is typed, or files of a session.");
  Serial.println();
  Serial.println("fft [session]");
  Serial.println("It process the fft of all files in a session.");
  Serial.println();

}

void serialRead(){
  serial_buffer[i++] = Serial.read();
  if(serial_buffer[i-1] == '\n'){ 
    serial_buffer[i-1] = '\0'; 
    i=0;
    String serial_bufferString = serial_buffer; serial_bufferString.trim(); serial_bufferString.toLowerCase();
    String firstPart = serial_bufferString.substring(0,serial_bufferString.indexOf(' '));
    String secondPart;
    if(firstPart == "periodic_save"){
      String aux = serial_bufferString.substring(serial_bufferString.indexOf(' ')+1);
        secondPart = aux.substring(0,aux.indexOf(' '));
        //SECOND PART TIME AND DATE FROM
        Serial.println();
        Serial.println("Date and hour start");
        String day_from_string =  secondPart.substring(0,secondPart.indexOf('/'));
        uint8_t day_from = day_from_string.toInt();
        Serial.print(day_from_string);
        Serial.print("/");
        String aux1 = secondPart.substring(secondPart.indexOf('/')+1);
        String month_from_string =  aux1.substring(0,aux1.indexOf('/'));
        uint8_t month_from = month_from_string.toInt();
        Serial.print(month_from_string);
        Serial.print("/");
        aux1 = aux1.substring(aux1.indexOf('/')+1);
        String year_from_string =  aux1.substring(0,aux1.indexOf('-'));
        uint16_t year_from = year_from_string.toInt();
        Serial.print(year_from_string);
        Serial.print(" ");
          
        aux1 = aux1.substring(aux1.indexOf('-')+1);
        String hour_from_string =  aux1.substring(0,aux1.indexOf(':'));
        uint8_t hour_from = hour_from_string.toInt();
        Serial.print(hour_from_string);
        Serial.print(":");
        aux1 = aux1.substring(aux1.indexOf(':')+1);
        String minute_from_string =  aux1.substring(0,aux1.indexOf(':'));
        uint8_t minute_from = minute_from_string.toInt();
        Serial.print(minute_from_string);
        Serial.print(":");
        aux1 = aux1.substring(aux1.indexOf(':')+1);
        uint8_t second_from = aux1.toInt();
        Serial.print(aux1);
        Serial.println();

        Serial.println("Date and hour end");
        //THIRD PART TIME AND DATE TO
        aux = aux.substring(aux.indexOf(' ')+1);
        String thirdPart = aux.substring(0,aux.indexOf(' '));
        String day_to_string = thirdPart.substring(0,thirdPart.indexOf('/'));
        uint8_t day_to = day_to_string.toInt();
        Serial.print(day_to_string);
        Serial.print("/");
        String aux2 = thirdPart.substring(thirdPart.indexOf('/')+1);
        String month_to_string = aux2.substring(0,aux2.indexOf('/'));
        uint8_t month_to = month_to_string.toInt();
        Serial.print(month_to_string);
        Serial.print("/");
        aux2 = aux2.substring(aux2.indexOf('/')+1);
        String year_to_string = aux2.substring(0,aux2.indexOf('-'));
        uint16_t year_to = year_to_string.toInt();
        Serial.print(year_to_string);
        Serial.print(" ");
          
        aux2 = aux2.substring(aux2.indexOf('-')+1);
        String hour_to_string = aux2.substring(0,aux2.indexOf(':'));
        uint8_t hour_to = hour_to_string.toInt();
        Serial.print(hour_to_string);
        Serial.print(":");
        aux2 = aux2.substring(aux2.indexOf(':')+1);
        String minute_to_string = aux2.substring(0,aux2.indexOf(':'));
        uint8_t minute_to = minute_to_string.toInt();
        Serial.print(minute_to_string);
        Serial.print(":");
        aux2 = aux2.substring(aux2.indexOf(':')+1);
        uint8_t second_to = aux2.toInt();
        Serial.print(aux2);
        Serial.println();

        aux = aux.substring(aux.indexOf(' ')+1);
        String fourthPart = aux.substring(0,aux.indexOf(' '));
        uint16_t duration_seconds = fourthPart.toInt();
        Serial.print("Duration in seconds:");
        Serial.println(duration_seconds);
        
        aux = aux.substring(aux.indexOf(' ')+1);
        String fifthPart = aux.substring(0,aux.indexOf(' '));
        uint16_t period_seconds = fifthPart.toInt();
        Serial.print("Period in seconds:");
        Serial.println(period_seconds);
        Serial.println();
        
        rec=1;

        PSP = {day_from,month_from,CalendarYrToTm(year_from),hour_from,minute_from,
        second_from,day_to,month_to,CalendarYrToTm(year_to),hour_to,minute_to,second_to,duration_seconds,
        period_seconds};
    }

    if(firstPart == "name_session"){
      String aux = serial_bufferString.substring(serial_bufferString.indexOf(' ')+1);
      String folderName = aux.substring(0,aux.indexOf(' '));
  
      if (!uSD.createSession(folderName)) Serial.println("Create folder failed...");
      else Serial.println("Session created!"); 
      Serial.println(); 
      
    }

    if(firstPart == "list"){
      String aux = serial_bufferString.substring(serial_bufferString.indexOf(' ')+1);
      String parameter = aux.substring(0,aux.indexOf(' '));
      String filename;
      
      if(parameter == "list") uSD.openRoot();
      else uSD.openSession(parameter);
      
      while(uSD.nextFile()){
        filename = uSD.getFileName();
        if(filename != "SYSTEM~1") Serial.println(filename);
      }
      Serial.println(); 
      
    }

    if(firstPart == "single_save"){
      String aux = serial_bufferString.substring(serial_bufferString.indexOf(' ')+1);
      secondPart = aux.substring(0,aux.indexOf(' '));
      singlerect = secondPart.toInt();

      singlerec=1;
    }
      
    if(firstPart == "fft"){
      String aux = serial_bufferString.substring(serial_bufferString.indexOf(' ')+1);
      String sessionName = aux.substring(0,aux.indexOf(' '));
      
      if(!uSD.openSession(sessionName)) Serial.println("Open session failed...");
      else {
        String fileName;
        String fftfolder;
        uint8_t data [1024];
        int16_t fftvalue;
        size_t nb = 1024;
        bool fftflag = false;
        fft1024.windowFunction(AudioWindowHamming1024);

        if(!uSD.createFFTSession(sessionName)) Serial.println("Create FFT folder failed...");
        
        while(uSD.nextFile()){
          Serial.println("Calculating FFT of file:");
          fileName = uSD.getFileName();
          Serial.println(fileName);
          fileName = sessionName+"/"+fileName;
          if(!uSD.openFFTbin()) Serial.println("Open bin file failed...");
          else Serial.println("Bin file opened!");
          if(!playSdWav.play(fileName.c_str())) Serial.println("Can't play wav file...");
          while(1){
            if (fft1024.available()) { 
              fftflag=true;
              for (int i=0; i<512; i++) {
                fftvalue = fft1024.read(i);
                data[i*2] = fftvalue & 0xFF;
                data[i*2+1] = fftvalue >> 8;
              }
              uSD.writeFFTbin(data,nb);
            }
            if(!playSdWav.isPlaying() && fftflag){
              uSD.closeFFTbin();
              Serial.println("File closed!");
              fftflag=false;
              break;
            }
          }
        }
      }
    }
    
  }
}

void loop() {

  if(Serial.available()>0) while(Serial.available()>0) serialRead();

  if(rec > 0){
    SIM_SCGC6 &= ~SIM_SCGC6_I2S;
    setWakeupCallandSleep(&PSP);
    SIM_SCGC6 |= SIM_SCGC6_I2S;
    setSyncProvider(getTeensy3Time);
    dhs=1;
    rec=0;
  }

  if(dhs>0){
    int32_t nsec;
    static uint8_t state=0;
    
    nsec=checkDutyCycle(&PSP,state);
    
    if(nsec==-1) state=2; // this will be last record in file
    if(nsec>0) 
    { 
      SIM_SCGC6 &= ~SIM_SCGC6_I2S;
      setWakeupCallandSleep(nsec); // file closed sleep now
      SIM_SCGC6 |= SIM_SCGC6_I2S;
      setSyncProvider(getTeensy3Time);
    }
    if(nsec==-2) norec=1;
    
    if(state==0){
      if(!uSD.open()) Serial.println("File open failed...");
      else Serial.println("File open succeed!");
      queue.begin();  
      recByteSaved = 0L;
      state=1;
    }

    if(state==1){
      if (queue.available() >= 2) {
        byte buffer[512];
        memcpy(buffer, queue.readBuffer(), 256);
        queue.freeBuffer();
        memcpy(buffer+256, queue.readBuffer(), 256);
        queue.freeBuffer();
        //WRITE
        if(!uSD.write(buffer, 512)) Serial.println("File write failed...");
        recByteSaved += 512;
      }
    }

    if(state==2){
      queue.end();
  
      while (queue.available() > 0) {
        uSD.write((byte*)queue.readBuffer(), 256);
        queue.freeBuffer();
        recByteSaved += 256;
      }
      uSD.writeOutHeader();
      uSD.close();
      Serial.println("File closed!");
      state=0;
      if(norec){ 
        dhs=0; 
        norec=0;
        if(uSD.isSession()){
          Serial.println("Session closed!");
          uSD.closeSession();
        }
        Serial.println("The end of record!");
        Serial.println();
      }
    }
      
  }

  if(singlerec){
    Serial.println("Record start!");
      
    if(!uSD.open()) Serial.println("File open failed...");
    else Serial.println("File open succeed!");
    
    queue.begin();  
    recByteSaved = 0L;

    while(!checkDutyCycle(singlerect)){
      if (queue.available() >= 2) {
        byte buffer[512];
        memcpy(buffer, queue.readBuffer(), 256);
        queue.freeBuffer();
        memcpy(buffer+256, queue.readBuffer(), 256);
        queue.freeBuffer();
        //WRITE
        if(!uSD.write(buffer, 512)) Serial.println("File write failed...");
        recByteSaved += 512;
      }
    }

    queue.end();
  
    while (queue.available() > 0) {
      uSD.write((byte*)queue.readBuffer(), 256);
      queue.freeBuffer();
      recByteSaved += 256;
    }
    uSD.writeOutHeader();
    uSD.close();
    
    Serial.println("File closed!");  
    Serial.println("The end of record!");
    Serial.println();

    singlerec = 0;
  }
  
}
