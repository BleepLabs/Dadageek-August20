//4051 mux example using this diagram

unsigned long cm;
unsigned long prev[8];
byte mux_select;
int pot_readings1[8];
int pot_readings2[8];
int digital_readings1[8];

#define ctrl_pin1 1
#define ctrl_pin2 2
#define ctrl_pin3 3
#define digital_read_pin 10
#define analog_pin1 A1
#define analog_pin2 A2


void setup() {
  pinMode(ctrl_pin1, OUTPUT);
  pinMode(ctrl_pin2, OUTPUT);
  pinMode(ctrl_pin3, OUTPUT);
  pinMode(digital_read_pin, INPUT_PULLUP);

  analogReadResolution(12);
  analogReadAveraging(64);

}

void loop() {
  cm = millis();

  // Each time this code happens a new input is selected and read
  //you could put this in the loop directly for even more speed or do it as a "for" instead.
  //I like this way as doing analog reads too fast can cause noise

  if (cm - prev[0] > 2) {
    prev[0] = cm;
    mux_select++;
    if (mux_select > 7) {
      mux_select = 0;
    }
    //mux_sel is a regualt number but we need the bits insode it to select which pins
    // are outputting high or low to sell the 4051s which pin to select;
    digitalWrite(ctrl_pin1, bitRead(mux_select, 0));
    digitalWrite(ctrl_pin2, bitRead(mux_select, 1));
    digitalWrite(ctrl_pin3, bitRead(mux_select, 2));

    //mux_select is also used to put the reading in the arrays
    //if youre not using one of these just comment it out
    pot_readings1[mux_select] = analogRead(analog_pin1);
    pot_readings2[mux_select] = analogRead(analog_pin2);
    digital_readings1[mux_select] = digitalRead(digital_read_pin);
  }

}
