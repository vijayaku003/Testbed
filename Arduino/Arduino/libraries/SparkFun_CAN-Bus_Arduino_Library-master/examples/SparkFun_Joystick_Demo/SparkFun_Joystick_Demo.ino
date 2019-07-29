int incomingByte = 0;   // for incoming serial data

void setup() {
        Serial.begin(115200);     // opens serial port, sets data rate to 9600 bps
        pinMode(8, OUTPUT);
}

void loop() {

        // send data only when you receive data:
        if (Serial.available() > 0) {
                // read the incoming byte:
                incomingByte = Serial.read();
                digitalWrite(8, HIGH); // sets the digital pin 13 on
                // say what you got:
               // Serial.print("I received: ");
               // Serial.println(incomingByte, DEC);
               delay(100);
                digitalWrite(8, LOW);  // sets the digital pin 13 off

        }
}
