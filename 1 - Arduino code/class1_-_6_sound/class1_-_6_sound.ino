int led1_pin =  10;
int led1_state = LOW;

int led2_pin =  9;
int led2_state = 0;

int audio_pin =  A14;
int audio_state = 0;


unsigned long  previous_time1 = 0;SFSFSDFSDFSFSFS
unsigned long  previous_time2 = 0;
unsigned long  previous_time3 = 0;

unsigned long current_time;

unsigned long interval1 = 50;
unsigned long interval2 = 50;
unsigned long interval3 = 50;

int button_pin; 
int button_state; 

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
  interval3 = pot1_value / 10;

  pot2_value = analogRead(A1) / 4; //this value is 0-1024 but analogwrite is 0-255 so we divide by 4. We'll talk about these funny numbers later


  if (current_time - previous_time1 > interval1) {
    previous_time1 = current_time;

    if (led1_state == LOW) {
      led1_state = HIGH;
    }
    else {
      led1_state = LOW;
    }
    analogWrite(led1_pin, led1_state * pot2_value); //since the state is 1 or 0 it's off half the time, on at our new pot reading the other half
    //not all pins can do this.
  }


  if (current_time - previous_time2 > interval2) {
    previous_time2 = current_time;

    if (led2_state == 0) {
      led2_state = 1;
    }
    else {
      led2_state = 0;
    }
    analogWrite(led2_pin, led2_state * pot2_value);
  }

  if (current_time - previous_time3 > interval3) {
    previous_time3 = current_time;

    if (audio_state == 0) {
      audio_state = 1;
    }
    else {
      audio_state = 0;
    }
    analogWrite(A14, audio_state * 255);
    //A14 is a 12b analog DAC, not just a digital PWM pin
    //That doesn't mean everything comeing out of it will sounds great though...
  }

}
