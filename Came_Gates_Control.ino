#include <Keypad.h>
//=================================================================================================
#define TX_PIN  11
#define LED_PIN 13
#define BIT_LEN         320
#define BLINK_INTERVAL  500
//=================================================================================================
// Setup Keypad
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = 
{
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = {8, 3, 4, 6}; 
byte colPins[COLS] = {7, 9, 5}; 

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//=================================================================================================
// Defind gate codes
#define GATE_MAIN_IN       0xAAAAAA
#define GATE_MAIN_OUT      0xBBBBBB
#define GATE_PARK_NEAR_IN  0xCCCCCC
#define GATE_PARK_NEAR_OUT 0xDDDDDD
#define GATE_PARK_FAR_IN   0xEEEEEE
#define GATE_PARK_FAR_OUT  0xFFFFFF
//=================================================================================================
// Global variables
unsigned long previousMillis = 0; 
int LED_state = LOW;
//=================================================================================================
// Send code vith help of 433 MHz transmitter connected to TX_PIN
void SendCame(unsigned long code)
{
  int repeat_count;
  repeat_count = 24;

  for (int j=0; j<repeat_count; j++)
  {
    digitalWrite(TX_PIN, LOW);
    delayMicroseconds(BIT_LEN * 63);
    digitalWrite(TX_PIN, HIGH);
    delayMicroseconds(BIT_LEN);
    
    //Sending bit by bit
    for (int i=23; i>=0; i--)
    {  // 0 -> 011, 1 -> 001
      byte b = bitRead(code, i);

      digitalWrite(TX_PIN,LOW);
      delayMicroseconds(BIT_LEN);
      
      if (!b)
        digitalWrite(TX_PIN,HIGH);
      delayMicroseconds(BIT_LEN);
      
      digitalWrite(TX_PIN,HIGH);
      delayMicroseconds(BIT_LEN);
    }
    digitalWrite(TX_PIN,LOW);
    delay(16);
  }
}
//-------------------------------------------------------------------------------------------------
void setup()
{
  delay(200);
  
  // Setup outpus for transmitter and LED pins
  pinMode(TX_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);   // Kept for keypad diagnostics
}
//-------------------------------------------------------------------------------------------------  
void loop()
{
  unsigned long currentMillis = millis();
  
  char key = keypad.getKey();
  
  if (key)
  {
    Serial.println(key);   // Just for diagnostics

    switch (key)
    {
      case '3':
        SendCame(GATE_MAIN_IN);
        break;
      case '6':
        SendCame(GATE_MAIN_OUT);
        break;
      case '9':
        SendCame(GATE_PARK_NEAR_IN);
        break;
      case '#':
        SendCame(GATE_PARK_NEAR_OUT);
        break;
      case '8':
        SendCame(GATE_PARK_FAR_IN);
        break;
      case '0':
        SendCame(GATE_PARK_FAR_OUT);
        break;  
    }
  }

  // Simply blink LED to show that board is alive
  if (currentMillis - previousMillis >= BLINK_INTERVAL) 
  {
    previousMillis = currentMillis;
    if (LED_state == LOW) 
      LED_state = HIGH;
    else
      LED_state = LOW;
    digitalWrite(LED_PIN, LED_state);
  }
}
//-------------------------------------------------------------------------------------------------
