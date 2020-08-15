/*
  Serial allow us to send information from the Teensy to your computer
  To view it, click the magnifying glass icon in the top right of the Arduino IDE
  The serial plotter in the tools menu is useful to visualize changing values
*/

int pots[8]; // hold 8 integers in the array "pots"
//arrays are useful to keep things organized and easily do work on lots of values at once
unsigned long current_time;
unsigned long prev_time[12]; //we can make however many positions in the array as our RAM can hold.
// it's not an issue if we don't use them all
int prev_pots[8];
int pot_time[100];
int pot_write_head;
int pot_read_head;
void setup() {
  analogReadAveraging(64);
  //Serial.begin(); //this is not necessary for the Teensy's USB serial but you'll see it in other code
}

int gap = 1;
void loop() {
  current_time = millis();
  // read the pots and put them in the pot array
  pots[1] = analogRead(A1);
  pots[2] = analogRead(A2);
  pots[3] = analogRead(A3);

  if (current_time - prev_time[1] > 10) { //[0] is the first location in the array
    prev_time[1] = current_time;

    pot_write_head += 1;
    if (pot_write_head > 99) {
      pot_write_head = 0;
    }
    pot_read_head = pot_write_head - 50;
    if (pot_read_head < 0) {
      pot_read_head += 99;
    }
    pot_time[pot_write_head] = analogRead(A0);

    if (pot_time[pot_read_head] != pot_time[pot_write_head]) {
      Serial.println(pot_time[pot_write_head]);
    }
  }

  if (current_time - prev_time[0] > 250  && 1 == 0) { //[0] is the first location in the array
    prev_time[0] = current_time;

    // start at 0. when this piece of code ends, increment j by one as long as its less than 8
    // once its at 8 the for is done and we carry along with the rest of the code.

    for (int j = 0; j < 4; j += 1) {
      Serial.print(j);
      Serial.print("~"); // quotations mean print these characters
      Serial.print(pots[j]); //print the j potion of pot
      Serial.print("\t"); //prints a tab to keep everything in nice columns
    }

    Serial.println(current_time); //print a new line

  }

}
