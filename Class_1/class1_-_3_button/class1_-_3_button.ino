int led1_pin =  10;  // The pin for LED1
int led1_state = LOW;  // The state of LED1

int led2_pin =  9;  // The LED2 pin
int led2_state = 0;  // The LED2 state LOW and 0 are synonymous

unsigned long  previous_time1 = 0;  // will store the last time LED1 was updated
unsigned long  previous_time2 = 0;  // will store the last time LED2 was updated

unsigned long current_time; 

unsigned long interval1 = 50; //the interval that LED1 will change
unsigned long interval2 = 50; //the interval that LED2 will change

int button_pin = 0; //the pin one side of the button is connected to. The other side connects to ground.
int button_state; //where we will store if the reading on the pin is high or low.

void setup() {
  // set both LED pins as output:
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);  //When using a button, always make sure to set the mode to INPUT_PULLUP

}

void loop()
{
  current_time = millis();

  button_state = digitalRead(button_pin); //if the button is not being pressed it will read HIGH. if it pressed it will read LOW

  if (current_time - previous_time1 > interval1) {
    previous_time1 = current_time;

    if (led1_state == LOW) {
      led1_state = HIGH;
    }
    else {
      led1_state = LOW;
    }
    if (button_state == 0) { // if the button is pressed
      digitalWrite(led1_pin, led1_state); //do the same thing as before
    }
    if (button_state == 1) {
      digitalWrite(led1_pin, 0); //don't light up
    }
  }


  if (current_time - previous_time2 > interval2) { //has "interval2" amount of time past since we last executed this code?
    previous_time2 = current_time;

    // if  LED2 is off turn it on and vice versa:
    if (led2_state == 0) {
      led2_state = 1;
    }
    else {
      led2_state = 0;
    }

    if (button_state == 1) { // we can do the opposite for this LED
      digitalWrite(led2_pin, led2_state);
    }
    else { //there are only two options so else works too
      digitalWrite(led2_pin, 0);
    }
  }

}
