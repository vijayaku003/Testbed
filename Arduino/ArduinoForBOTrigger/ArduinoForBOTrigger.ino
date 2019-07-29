/****************************************************************************
  CAN-Bus -- Ultrasonic Demo
  Daniel Ng
*************************************************************************/

#include <Canbus.h>
#include <NewPing.h>

#define TRIGGER_PIN 3
#define ECHO_PIN 4
#define MAX_DISTANCE 200
#define LED1 8
#define LED2 7
/* Define Joystick connection pins */
#define UP     A1
#define DOWN   A3
#define LEFT   A2
#define RIGHT  A5
#define CLICK  A4

static int byteCount = 0;

//********************************Setup Loop*********************************//

void setup() {

  //Initialize analog pins as inputs
  pinMode(UP, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(CLICK, INPUT);

  //Pull analog pins high to enable reading of joystick movements
  digitalWrite(UP, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(LEFT, HIGH);
  digitalWrite(RIGHT, HIGH);
  digitalWrite(CLICK, HIGH);

  Serial.begin(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);

  Serial.println("CAN-Bus Demo");

  if (Canbus.init(CANSPEED_500)) /* Initialise MCP2515 CAN controller at the specified speed */
  {
    Serial.println("CAN Init ok");
  } else
  {
    Serial.println("Can't init CAN");
  }

  delay(1000);


}



//********************************Main Loop*********************************//

void loop() {
   int up = 100;
   int down = 200;
   int left = 44;
   int right = 144;
   int clicked = 244; 
  if (digitalRead(UP) == 0) {
    Canbus.message_tx_ultrasonic(100);
  }
  
if (digitalRead(DOWN) == 0) {
      Canbus.message_tx_ultrasonic(200);

}

if (digitalRead(LEFT) == 0) {
        Canbus.message_tx_ultrasonic(44);

}

if (digitalRead(RIGHT) == 0) {
      Canbus.message_tx_ultrasonic(144);

}
if (digitalRead(CLICK) == 0) {
      Canbus.message_tx_ultrasonic(244);

}
delay(250);
}


