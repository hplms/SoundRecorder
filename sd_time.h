#ifndef _SD_TIME_H
#define _SD_TIME_H

#include <SD.h>
#include <SPI.h>

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// Use these with the Teensy 3.5 & 3.6 SD card
//#define SDCARD_CS_PIN    BUILTIN_SDCARD
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

// Use these for the SD+Wiz820 or other adaptors
//#define SDCARD_CS_PIN    4
//#define SDCARD_MOSI_PIN  11
//#define SDCARD_SCK_PIN   13

//write wav
unsigned long ChunkSize = 0L;
unsigned long Subchunk1Size = 16;
unsigned int AudioFormat = 1;
unsigned int numChannels = 1;
unsigned long sampleRate = F_SAMP;
unsigned int bitsPerSample = 16;
unsigned long byteRate = sampleRate*numChannels*(bitsPerSample/8);// samplerate x channels x (bitspersample / 8)
unsigned int blockAlign = numChannels*bitsPerSample/8;
unsigned long Subchunk2Size = 0L;
unsigned long recByteSaved = 0L;
unsigned long NumSamples = 0L;
byte byte1, byte2, byte3, byte4;

class c_uSD
{
  private:
    File file;
    File dir;
    File fftfile;
  public:
    void init();
    bool open(void);
    bool write(byte * data, size_t ndat);
    void close(void);
    bool createSession(String sessionName);
    bool openSession(String sessionName);
    bool openRoot(void);
    char * getFileName(void);
    bool openFFTbin(void);
    void closeFFTbin(void);
    bool writeFFTbin(byte* data, size_t ndat);
    bool nextFile(void);
    bool createFFTSession(String sessionName);
    bool isSession(void);
    void closeSession(void);
    
    void writeOutHeader(void);

    void setPrefix(char *prefix);
    
  private:
    int16_t session;
    String path = "/";
    String fftpath = "/";
    char * fileName;

};
c_uSD uSD;

/*
 *  Logging interface support / implementation functions 
 */
//_______________________________ For File Time settings _______________________

#define EPOCH_YEAR 1970 //T3 RTC
#define LEAP_YEAR(Y) (((EPOCH_YEAR+Y)>0) && !((EPOCH_YEAR+Y)%4) && ( ((EPOCH_YEAR+Y)%100) || !((EPOCH_YEAR+Y)%400) ) )
static  const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; 

tmElements_t seconds2tm(uint32_t tt)
{ 
  tmElements_t tx;
  tx.Second   = tt % 60;    tt /= 60; // now it is minutes
  tx.Minute   = tt % 60;    tt /= 60; // now it is hours
  tx.Hour     = tt % 24;    tt /= 24; // now it is days
  tx.Wday     = ((tt + 4) % 7) + 1;   // Sunday is day 1 (tbv)

  // tt is now days since EPOCH_Year (1970)
  uint32_t year = 0;  
  uint32_t days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= tt) year++;

  tx.Year=CalendarYrToTm(1970+year);
  
  // correct for last (actual) year
  days -= (LEAP_YEAR(year) ? 366 : 365);
  tt  -= days; // now tt is days in this year, starting at 0
  
  uint32_t mm=0;
  uint32_t monthLength=0;
  for (mm=0; mm<12; mm++) 
  { monthLength = monthDays[mm];
    if ((mm==1) & LEAP_YEAR(year)) monthLength++; 
    if (tt<monthLength) break;
    tt -= monthLength;
  }
  tx.Month = mm + 1;   // jan is month 1  
  tx.Day = tt + 1;     // day of month
  return tx;
}

uint32_t tm2seconds (tmElements_t *tx) 
{
  uint32_t tt; 
  tt=tx->Second + tx->Minute*60 + tx->Hour*3600;

  // count days size epoch until previous midnight
  uint32_t days = tx->Day-1;

  uint32_t mm=0;
  //uint32_t monthLength=0;
  for (mm=0; mm<(uint32_t)(tx->Month-1); mm++) days+=monthDays[mm]; 
  if(tx->Month>2 && LEAP_YEAR(tmYearToCalendar(tx->Year)-1970)) days++;

  uint32_t years=0;
  while(years++ < (uint32_t)(tmYearToCalendar(tx->Year)-1970)) days += (LEAP_YEAR(years) ? 366 : 365);
  //  
  tt+=(days*24*3600);
  return tt;
}

char *makeFilename()
{ static char filename[40];

  uint32_t tt = now();

  tmElements_t tx = seconds2tm(tt);
  sprintf(filename, "%02d%02d%02d%02d%s", 
                    tx.Day, tx.Month, tx.Hour, tx.Minute,".WAV");
                    
  return filename;  
}

//____________________________ FS Interface implementation______________________
void c_uSD::init()
{
  
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  while (!(SD.begin(SDCARD_CS_PIN))) {
    Serial.println("Unable to access the SD card");
  }

}

bool c_uSD::open(void){
  
  char *filename = makeFilename();
  Serial.println(filename);
  
  //add session path
  if(session){
    String aux = filename;
    aux = path+aux;
    filename=aux.c_str();
  }

  if (SD.exists(filename)) SD.remove(filename);
  file = SD.open(filename,FILE_WRITE);
  return file;
  
}

bool c_uSD::write(byte *data, size_t ndat){
  
  return ndat == file.write(data, ndat);
  
}

void c_uSD::close(void){  
   
    //file.truncate(0); //ERROR
    file.close();
}

bool c_uSD::createSession(String sessionName){
  if(sessionName.length()>5){
    Serial.println("Name session must be less than 5 characters...");
    return false;
  }
  session=1;
  path = sessionName + "/";
  return SD.mkdir(sessionName.c_str());
}

bool c_uSD::openSession(String sessionName){
  path = sessionName + "/";
  dir = SD.open(sessionName.c_str());
  return dir;
}

bool c_uSD::openRoot(){
  dir = SD.open("/");
  return dir;
}

bool c_uSD::createFFTSession(String sessionName){
  fftpath = fftpath + sessionName + "fft";
  return SD.mkdir(fftpath.c_str());
}

bool c_uSD::nextFile(){
  file=dir.openNextFile();
  return file;
}

char* c_uSD::getFileName(){
  return file.name();
}

bool c_uSD::openFFTbin(){
  String binFileName = file.name();
  binFileName.replace(".WAV", ".BIN");
  binFileName = fftpath +"/"+ binFileName;
  if (SD.exists(binFileName.c_str())) SD.remove(binFileName.c_str());
  fftfile = SD.open(binFileName.c_str(),FILE_WRITE);
  return fftfile;
}

bool c_uSD::writeFFTbin(byte* data, size_t ndat){
  return ndat == fftfile.write(data,ndat);
}

void c_uSD::closeFFTbin(){
  fftfile.close();
}

bool c_uSD::isSession(){
  return session==1;
}

void c_uSD::closeSession(){
  session=0;
}

void c_uSD::writeOutHeader() { // update WAV header with final filesize/datasize

//  NumSamples = (recByteSaved*8)/bitsPerSample/numChannels;
//  Subchunk2Size = NumSamples*numChannels*bitsPerSample/8; // number of samples x number of channels x number of bytes per sample
  Subchunk2Size = recByteSaved;
  ChunkSize = Subchunk2Size + 36;
  file.seek(0);
  file.write("RIFF");
  byte1 = ChunkSize & 0xff;
  byte2 = (ChunkSize >> 8) & 0xff;
  byte3 = (ChunkSize >> 16) & 0xff;
  byte4 = (ChunkSize >> 24) & 0xff;  
  file.write(byte1);  file.write(byte2);  file.write(byte3);  file.write(byte4);
  file.write("WAVE");
  file.write("fmt ");
  byte1 = Subchunk1Size & 0xff;
  byte2 = (Subchunk1Size >> 8) & 0xff;
  byte3 = (Subchunk1Size >> 16) & 0xff;
  byte4 = (Subchunk1Size >> 24) & 0xff;  
  file.write(byte1);  file.write(byte2);  file.write(byte3);  file.write(byte4);
  byte1 = AudioFormat & 0xff;
  byte2 = (AudioFormat >> 8) & 0xff;
  file.write(byte1);  file.write(byte2); 
  byte1 = numChannels & 0xff;
  byte2 = (numChannels >> 8) & 0xff;
  file.write(byte1);  file.write(byte2); 
  byte1 = sampleRate & 0xff;
  byte2 = (sampleRate >> 8) & 0xff;
  byte3 = (sampleRate >> 16) & 0xff;
  byte4 = (sampleRate >> 24) & 0xff;  
  file.write(byte1);  file.write(byte2);  file.write(byte3);  file.write(byte4);
  byte1 = byteRate & 0xff;
  byte2 = (byteRate >> 8) & 0xff;
  byte3 = (byteRate >> 16) & 0xff;
  byte4 = (byteRate >> 24) & 0xff;  
  file.write(byte1);  file.write(byte2);  file.write(byte3);  file.write(byte4);
  byte1 = blockAlign & 0xff;
  byte2 = (blockAlign >> 8) & 0xff;
  file.write(byte1);  file.write(byte2); 
  byte1 = bitsPerSample & 0xff;
  byte2 = (bitsPerSample >> 8) & 0xff;
  file.write(byte1);  file.write(byte2); 
  file.write("data");
  byte1 = Subchunk2Size & 0xff;
  byte2 = (Subchunk2Size >> 8) & 0xff;
  byte3 = (Subchunk2Size >> 16) & 0xff;
  byte4 = (Subchunk2Size >> 24) & 0xff;  
  file.write(byte1);  file.write(byte2);  file.write(byte3);  file.write(byte4);
  Serial.println("header written"); 
  /*Serial.print("Subchunk2: "); 
  Serial.println(Subchunk2Size);*/
}

#endif
