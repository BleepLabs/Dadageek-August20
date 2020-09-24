//If we want a second LED we need to have separate variables for some things

//Variables will not have anything inherently in common because of their names. We could name them anything.
// The variable names are only there to help us. The computer gets rid of the names when we compile and upload

int led1_pin =  10;  // The pin for LED1
int led1_state = LOW;  // The state of LED1 LOW and 0 are synonymous

int led2_pin =  9;   // The LED2 pin
int led2_state = 0;  // The LED2 state LOW and 0 are synonymous

unsigned long  previous_time1 = 0;  // will store the last time LED1 was updated
unsigned long  previous_time2 = 0;  // will store the last time LED2 was updated


unsigned long current_time;  // We haven't set it equal to anything so it will default to 0

unsigned long interval1 = 1000;  //the interval that LED1 will change
unsigned long interval2 = 333;  //the interval that LED2 will change


void setup() {
  // set both LED pins as outputs
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);

}

void loop()
{
  current_time = millis(); //call the function millis() to get the time in milliseconds that have elapsed since the device started

  if (current_time - previous_time1 > interval1) { //has "interval1" amount of time past since we last executed this code?
    previous_time1 = current_time; //if the statement is true remember the current time

    // if the LED is off turn it on and vice versa:
    if (led1_state == LOW) {
      led1_state = HIGH;
    }
    else {
      led1_state = LOW;
    }
    /*
        //  This wouldn't work
        if (led1_state == LOW) {
          led1_state = HIGH;
        }
        if (led1_state == HIGH) {
          led1_state = LOW;
        }
    */
    digitalWrite(led1_pin, led1_state);   // Write the value "led1_state" to the pin "led1_pin".
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

    digitalWrite(led2_pin, led2_state);   // Write the value "led2_state" to "led2_pin.

  }

}
