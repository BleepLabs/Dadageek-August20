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

int pot1_value;
int pot1_pin = A0;

int pot2_value;
int pot2_pin = A1;

void setup() {
  // set both LED pins as output:
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);

}

void loop()
{
  current_time = millis();

  button_state = digitalRead(button_pin); //if the button is not being pressed it will read HIGH. if it pressed it will read LOW

  pot1_value = analogRead(A0); //Read the analog voltage at pin A0. Returns 0 for 0 volts and 1023 for the max voltage (3.3V)
  interval1 = pot1_value / 2;
  interval2 = pot1_value / 5;

  pot2_value = analogRead(A1) / 4; //this value is 0-1024 but analogwrite is 0-255 so we divide by 4. We'll talk about these funny numbers later


  if (current_time - previous_time1 > interval1) {
    previous_time1 = current_time;

    if (led1_state == LOW) {
      led1_state = HIGH;
    }
    else {
      led1_state = LOW;
    }
    analogWrite(led1_pin, led1_state*pot2_value); //since the state is 1 or 0 it's off half the time, on at our new pot reading the other half
    //Only pins marked "PWM" can do analogWrite.
  }


  if (current_time - previous_time2 > interval2) {
    previous_time2 = current_time;

    if (led2_state == 0) {
      led2_state = 1;
    }
    else {
      led2_state = 0;
    }
    analogWrite(led2_pin, led2_state*pot2_value);
  }
}
