// Arduino M5Stack sketch to connect to AWS and subscribe for messages in a topic 
// Created mainly as debug monitor for another IOT devices by subscription to a topic
// Plan to publish to the topic later. 
//
// Need one more .ino file with credentials (see Private data variables below) and 
// Amazon certificates in aws_iot_certficates.c (see Hornbill AWS_IOT for details and example)
// Don't forget to activate your certs/policies and link it all together with a thing 
// (Otherwise you may observe AWS errors via Serial port monitor to debug connection problems)
//
// With M5Stack support library from https://github.com/m5stack/M5Stack 
// and AWS_IOT library from https://github.com/ExploreEmbedded/Hornbill-Examples/tree/master/arduino-esp32/AWS_IOT
// JSON is parsed with https://arduinojson.org/
// NTP time with help of https://github.com/arduino-libraries/NTPClient
//===============================================================================
#include <M5Stack.h>
#include <Free_Fonts.h> 
#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
//-------------------------------------------------------------------------------
#define ESP32
#define MAX_RETRIES   128
#define BUF_SIZE      1024
// display colors
#define BKGR_COLOR    TFT_BLUE
#define OK_COLOR      TFT_GREEN
#define ERR_COLOR     TFT_RED
#define LOG_FILE_NAME "/DataLog.txt"
//===============================================================================
// Private data for connections are stored in another file 
extern char WIFI_SSID[];      // name of your WiFi network
extern char WIFI_PASSWORD[];  // password to WiFi
extern char HOST_ADDRESS[];   // URL of your AWS endpoint
extern char CLIENT_ID[];      // I put thing name here
extern char TOPIC_NAME[];     // Topic to subscribe to
extern char CMD_TOPIC_NAME[]; // Topic to publish to
//===============================================================================
// Global variables
AWS_IOT aws_client;
bool AWS_connected;   // Flag to show AWS connection state
bool AWS_msg;         // Flag to show incoming message
char SendBuffer[BUF_SIZE];
char ReceiveBuffer[BUF_SIZE];
WiFiUDP ntpUDP;
StaticJsonDocument<256> json_doc;
NTPClient ntp_client(ntpUDP, "pool.ntp.org", 10800, 60000);
//===============================================================================
void CallbackAWS(char *TopicName, int MsgLen, char *Msg)
{
  if (MsgLen > (BUF_SIZE - 1))
    DoReset("Incoming message to big");
  
  strncpy(ReceiveBuffer, Msg, MsgLen);
  ReceiveBuffer[MsgLen] = 0;
  AWS_msg = true;
}
//-------------------------------------------------------------------------------
// Show error message and reset if something went wrong
void DoReset(char *Message)
{
  M5.Lcd.clear(BKGR_COLOR);
  M5.Lcd.setTextColor(ERR_COLOR); 
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print(Message);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.print("Rebooting in 60 sec...");

  vTaskDelay(60000 / portTICK_RATE_MS);
  ESP.restart();
}
//-------------------------------------------------------------------------------
void PrintCurrentTime()
{
  M5.Lcd.fillRect(201, 201, 320, 20, BKGR_COLOR);
  M5.Lcd.setTextColor(OK_COLOR);
  M5.Lcd.setCursor(200, 220);
  M5.Lcd.print(ntp_client.getFormattedTime());
}
//-------------------------------------------------------------------------------
void ConnectWiFi()
{
  int i = 0;
  int retry_cnt = 0;
  
  M5.Lcd.clear(BKGR_COLOR);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setFreeFont(FM12);

  strcpy(SendBuffer, "WiFi SSID: ");
  strcat(SendBuffer, WIFI_SSID);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print(SendBuffer);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Wait for radio connection setup
  while (WiFi.status() != WL_CONNECTED)
  {
    retry_cnt++;
    if (retry_cnt > MAX_RETRIES)
      DoReset("WiFi failed");
    
    for (i=0; i<retry_cnt; i++)
      SendBuffer[i] = '.';
    SendBuffer[retry_cnt] = 0;    
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.print(SendBuffer);
    vTaskDelay(1000 / portTICK_RATE_MS); // sleep for 1 second
  }

  M5.Lcd.clear(BKGR_COLOR);
  M5.Lcd.setTextColor(OK_COLOR);
  M5.Lcd.setCursor(0, 20);
  sprintf(SendBuffer, "WiFi SSID: %s", WIFI_SSID);
  M5.Lcd.print(SendBuffer);

  // Wait for DHCP IP assignment
  retry_cnt = 0;
  M5.Lcd.setTextColor(TFT_WHITE);
  while (WiFi.localIP()[0] == 0)
  {
    retry_cnt++;
    if (retry_cnt > MAX_RETRIES)
      DoReset("DHCP failed");
    M5.Lcd.setCursor(0, 40);
    sprintf(SendBuffer, "IP: %s", WiFi.localIP().toString().c_str());
    M5.Lcd.print(SendBuffer); 
    vTaskDelay(1000 / portTICK_RATE_MS); // sleep for 1 second
  }
  M5.Lcd.fillRect(0, 21, 320, 20, BKGR_COLOR);
  M5.Lcd.setTextColor(OK_COLOR);
  M5.Lcd.setCursor(0, 40);
  sprintf(SendBuffer, "IP: %s", WiFi.localIP().toString().c_str());
  M5.Lcd.print(SendBuffer);  

  ntp_client.begin();
}
//-------------------------------------------------------------------------------
void ConnectAWS()
{
  int res;
  int retry_cnt = 0;

  M5.Lcd.fillRect(0, 41, 320, 40, BKGR_COLOR);
  M5.Lcd.setTextColor(OK_COLOR);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.print("AWS connecting"); 
  res = -1;
  while (res != 0)
  {
    retry_cnt++;
    PrintCurrentTime();
    if (retry_cnt > MAX_RETRIES)
      DoReset("ASW failed");
    res = aws_client.connect(HOST_ADDRESS, CLIENT_ID);

    M5.Lcd.fillRect(0, 41, 320, 20, BKGR_COLOR);
    M5.Lcd.setTextColor(ERR_COLOR);
    M5.Lcd.setCursor(0, 60);
    sprintf(SendBuffer, "AWS status: 0x%x", res);
    M5.Lcd.print(SendBuffer); 
    vTaskDelay(5000 / portTICK_RATE_MS); // sleep for 5 second
  }
  M5.Lcd.fillRect(0, 41, 320, 20, BKGR_COLOR);
  M5.Lcd.setTextColor(OK_COLOR);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.print("AWS connected"); 

  AWS_connected = true;

  res = -1;
  while (res != 0)
  {
    retry_cnt++;
    PrintCurrentTime();
    if (retry_cnt > MAX_RETRIES)
      DoReset("Subscribe failed");
    res = aws_client.subscribe(TOPIC_NAME, CallbackAWS);

    M5.Lcd.fillRect(0, 61, 320, 20, BKGR_COLOR);
    M5.Lcd.setTextColor(ERR_COLOR);
    M5.Lcd.setCursor(0, 80);
    sprintf(SendBuffer, "Subscr. status: 0x%x", res);
    M5.Lcd.print(SendBuffer); 
    vTaskDelay(5000 / portTICK_RATE_MS); // sleep for 5 second
  }
  M5.Lcd.fillRect(0, 61, 320, 20, BKGR_COLOR);
  M5.Lcd.setTextColor(OK_COLOR);
  M5.Lcd.setCursor(0, 80);
  sprintf(SendBuffer, "topic: %s", TOPIC_NAME);
  M5.Lcd.print(SendBuffer); 
}
//===============================================================================
// the setup routine runs once when M5Stack starts up
void setup()
{
  AWS_connected = false;
  AWS_msg = false;
  
  M5.begin();
}
//===============================================================================
// the loop routine runs over and over again forever
void loop() 
{  
  int res;
  File log_file;
  double data_value;
  
  if (WiFi.status() != WL_CONNECTED)
  {
    AWS_connected = false;
    ntp_client.end();
    
    ConnectWiFi();
  }
  else 
  { // WiFi Connected
    ntp_client.update();
    
    if (!AWS_connected)
      ConnectAWS();  
  }

  if (AWS_msg)
  {
    AWS_msg = false;
    M5.Lcd.fillRect(0, 81, 320, 120, BKGR_COLOR);
    M5.Lcd.setTextColor(OK_COLOR);
    M5.Lcd.setCursor(0, 100);
    sprintf(SendBuffer, "MSG @%s", ntp_client.getFormattedTime());
    M5.Lcd.print(SendBuffer);

    /////////////////////////////////////////
    //  Expected Json message have format:
    //  {
    //    "data": 48.75608
    //  }
    /////////////////////////////////////////
    DeserializationError error = deserializeJson(json_doc, ReceiveBuffer);
    if (error)
      DoReset("JSON failure");
    data_value = json_doc["data"].as<double>();
    sprintf(ReceiveBuffer, "Data: %f", data_value);
    
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.print(ReceiveBuffer); 

    log_file = SD.open(LOG_FILE_NAME, FILE_WRITE);
    if (log_file)
    {
      log_file.println(SendBuffer);
      log_file.println(ReceiveBuffer);
      log_file.close();
    }
  }

  M5.update();
  if (M5.BtnA.wasReleased())
  { // Send ON command  
    json_doc.clear();
    json_doc["light_mode"] = 1;
    serializeJson(json_doc, SendBuffer, BUF_SIZE);
    res = aws_client.publish(CMD_TOPIC_NAME, SendBuffer);
    M5.Lcd.fillRect(1, 201, 20, 20, OK_COLOR);
  }
  
  if (M5.BtnB.wasReleased())
  { // Clear status
    M5.Lcd.fillRect(1, 201, 20, 20, BKGR_COLOR);
  }
  
  if (M5.BtnC.wasReleased())
  { // Send OFF command   
    json_doc.clear();
    json_doc["light_mode"] = 0;
    serializeJson(json_doc, SendBuffer, BUF_SIZE);
    res = aws_client.publish(CMD_TOPIC_NAME, SendBuffer);  
    M5.Lcd.fillRect(1, 201, 20, 20, ERR_COLOR);
  }

  PrintCurrentTime();
  vTaskDelay(500 / portTICK_RATE_MS);
}
