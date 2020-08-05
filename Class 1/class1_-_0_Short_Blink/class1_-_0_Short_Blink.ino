
uint32_t cm, cu, prev[10];
byte latch[3];

void setup() {
  pinMode(13, OUTPUT);

}

void loop() {
  cu = micros();
  cm = millis();

  if (cu - prev[2] > 100 && latch[0] == 1) {
    latch[0] = 0;
    digitalWrite(13, 0);
  }


  if (cm - prev[0] > 1000) {
    prev[0] = cm;
    latch[0] = 1;
    prev[2] = cu;
    digitalWrite(13, 1);
    Serial.print("Howdy! ");
    Serial.println(cm);

  }



}
