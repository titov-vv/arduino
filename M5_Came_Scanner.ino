#include <M5Stack.h>
#include "Free_Fonts.h"

// Below lines include and use RCSwitch library with 8th protocol for Came gates
// You need to add below protocol definition into RCSwitch.h as 8th protocol by yourself:
// { 320, { 64,  1 }, {  1,  2 }, {  2,  1 }, true  }
#define CAME_PROTO  8
#include <RCSwitch.h>

#define ESP32

RCSwitch Radio = RCSwitch();

unsigned long Data[12] = { 0xAAAAAA, 0xBBBBBB, 0xCCCCCC, 0xDDDDDD, 0, 0, 0, 0, 0, 0, 0, 0 };
int CodesCount = 4;
unsigned long LastUpd;
int res;
//===============================================================================
// Check if code is presend in Data array - if yes simply return it's index
// If code was not present in array then beep, add it to array and return
// index of newly added element
int StoreCode(unsigned long code)
{
  int i, last;
  last = -1;
  
  for (i = 0; i < 12; i++)
  {
    if (Data[i] == code)
      return i;
    if (Data[i] == 0)
    {
      last = i;
      break;
    }
  }
  if (last >= 0)
  {
    Data[last] = code;
    M5.Speaker.tone(900, 200);
    M5.Lcd.clear(BLUE);
  }

  return last;
}
//===============================================================================
// Put all 12 elements of Data array on a screen in 2 columns of HEX values
// If 0<=highlight<12 - mark this code with Red color
void ShowCodes(int highlight)
{
  int i;
  M5.Lcd.setFreeFont(FM18);  
  
  for (i=0; i<12; i++)
  {
    if (i<6)
      M5.Lcd.setCursor(0, 30*(i+1));
    else
      M5.Lcd.setCursor(160, 30*(i-5));

    if (i == highlight)
      M5.Lcd.setTextColor(TFT_RED);
    else
      M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.print(Data[i], HEX);
  }  
}
//===============================================================================
// the setup routine runs once when M5Stack starts up
void setup()
{
  M5.begin();
  M5.Lcd.clear(BLUE);
    
  Radio.enableReceive(2);

  ShowCodes(-1);
  res = -1;
  LastUpd = millis();
}
//===============================================================================
// the loop routine runs over and over again forever
void loop() 
{
  int i;
  unsigned long Code;

  if (Radio.available())
  {
    if (Radio.getReceivedProtocol() == CAME_PROTO)
    {
      Code = Radio.getReceivedValue();
      res = StoreCode(Code);
      ShowCodes(res);
      LastUpd = millis();
    }
        
    Radio.resetAvailable();  
  }

  if ((res >= 0) && ((millis()-LastUpd) > 3000))
  {
    ShowCodes(-1);
    res = -1;
  }

  // DEBUG
  M5.update();
  if (M5.BtnA.wasReleased())
  {   
    File file = SD.open("/Codes.txt", FILE_WRITE);
    if (file)
    {
      for (i=0; i<12; i++)
        file.println(Data[i], HEX);
      file.close();
    }
  }
  if (M5.BtnB.wasReleased())
  {   
  }
  if (M5.BtnC.wasReleased())
  {   
  }
}
