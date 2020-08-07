
uint32_t cm, cu, prev[10]; //large numbers to store micro and milliseconds timers in
// "prev" is an array of 10 separate variables. We'll just use two of them

byte LED_latch;

void setup() {
  pinMode(13, OUTPUT); //Pin 13 is has an LED attached to it.
}

void loop() {
  cu = micros(); //how many microseconds have elapsed since restarting
  cm = millis(); //how many milliseconds have elapsed since restarting

  //if 1 seconds has passed, do this code
  if (cm - prev[0] > 1000) {
    prev[0] = cm; //remember the time so we can measure how long it's been since we last ran this code
    LED_latch = 1;
    prev[2] = cu; //set the current time in microseconds to this variable
    digitalWrite(13, 1);  //turn the LED on
    Serial.print("Howdy! "); //print "howdy" on the serial monitor for some reason.
    Serial.println(cm); //Print the time in milliseconds and creates a new line
  }

  //If 100 microseconds have passed since prev[2] was set to the time in micros
  // and LED_latch[0] is 1 then run this code

  if (cu - prev[2] > 100 && LED_latch == 1) {
    LED_latch = 0;  //turn this back to 0
    digitalWrite(13, 0);  //turn the led off
  }




}
