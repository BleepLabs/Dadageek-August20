int led1_pin =  10;                // The pin for LED1
int led1_state = LOW;             // The state of LED1

int led2_pin =  9;                // The LED2 pin
int led2_state = 0;             // The LED2 state LOW and 0 are synonomous

unsigned long  previous_time1 = 0;  // will store the last time LED1 was updated
unsigned long  previous_time2 = 0;  // will store the last time LED2 was updated

unsigned long current_time;

unsigned long interval1 = 50;              //the interval that LED1 will change
unsigned long interval2 = 50;              //the interval that LED2 will change

int button_pin; //pin one side of the button is connected to
int button_state; //where we will store if the reading on the pin is high or low.

int pot1_value; //store the reading from the potentiomeer aka knob
int pot_pin = A0; //pins that can do analog readings start with A

void setup() {
  // set both LED pins as output:
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  //you don't need to setup analogRead pins
}

void loop()
{
  current_time = millis();

  button_state = digitalRead(button_pin); //if the button is not being pressed it will read HIGH. if it pressed it will read LOW

  pot1_value = analogRead(A0); //Read the analog voltage at pin A0. Returns 0 for 0 volts and 1023 for the max voltage (3.3V)
  interval1 = pot1_value / 2;  //we can do any math on variubles you can imagine. 
  interval2 = pot1_value * .2; // "interval2" doesnt need to be a "float" to multiply by one

  if (current_time - previous_time1 > interval1) { //we're comapring to milliseconds so a value of 1000 would make it turn on for 1 seconds then off for one
    previous_time1 = current_time;

    if (led1_state == LOW) {
      led1_state = HIGH;
    }
    else {
      led1_state = LOW;
    }
    digitalWrite(led1_pin, led1_state); 
  }


  if (current_time - previous_time2 > interval2) { 
    previous_time2 = current_time;

    if (led2_state == 0) {
      led2_state = 1;
    }
    else {
      led2_state = 0;
    }
    digitalWrite(led2_pin, led2_state);
  }
}
