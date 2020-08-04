int pots[8]; // hold 8 integers in the array "pots"
unsigned long current_time;
unsigned long prev_time[4]; //we can make however many postions in the array as our RAM can hold.
// it's not an issue if we don't use them either

void setup() {
  //Serial.begin(); //this is not necessary for the teeensy's USB serial but you'll see it in other code

}

void loop() {
  current_time = millis();
  pots[0] = analogRead(A0);
  pots[1] = analogRead(A1);
  pots[2] = analogRead(A2); //you don't need to have anythig hooked up to read the pin, it'll jsut give you some noise which is fine.

  if (current_time - prev_time[0] > 250) {  //[0] is the first location in the array
    prev_time[0] = current_time;


    // start at 0. when this piece of code ends, increment j by one as long as its less than 8
    // once its at 8 the for is done and we carry along with the rest of the code.
    for (int j = 0; j < 8; j += 1) {
      Serial.print(j);
      Serial.print("-"); // quotations mean print these characters
      Serial.print(pots[j]); //print the j postion of pots
      Serial.print("  ");
    }
    Serial.println(); //print a new line
  }


}
