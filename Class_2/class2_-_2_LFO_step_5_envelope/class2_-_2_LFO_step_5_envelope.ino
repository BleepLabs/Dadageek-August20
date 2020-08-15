/*
  Modulating the oscillator with LFOs
*/

IntervalTimer timer1; //Make a new interval timer called timer1

int led1_pin =  10;
int led1_state = LOW;
int led2_pin =  9;
int led2_state = 0;
unsigned long current_time;

//Instead of having the variables named like this:
/*
  unsigned long  previous_time1 = 0;
  unsigned long  previous_time2 = 0;
  unsigned long  previous_time3 = 0;
  unsigned long interval1 = 50;
  unsigned long interval2 = 50;
  unsigned long interval3 = 50;

*/

//lets do it like this:
unsigned long  previous_time[8];
unsigned long  time_interval[8] = {50, 50, 200};
//interval[0] now contains 50, interval[2] is 200, interval[3] is 0

int button_pin = 0;
int prev_button_state;
int button_state;
int pot1_pin = A0;
int pot2_pin = A1;
int pot_reading[8];
float fold_amount; //floats can hold decimals
float freq1;
float freq2, freq3;
int out1, out2, out3;// You can define variables like this too. All of them will be integers equal to 0
uint32_t dds_tune;
uint32_t dds_rate;
int lfo_latch[8];
float lfo[8]; //an array of 8 integers named "lfo"
float mod[8];
unsigned long cu, du, looptime;


void setup() {
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
  //On the Teensy the resolutions of the in and out can be set to 12 bits
  // by default it is 8 bit out, 10 bit in
  // when looking at other examples it's important to know what range these values are
  analogReadResolution(12);
  analogWriteResolution(12);
  analogReadAveraging(64); //instead of reading once, do it 64 times and average

  //set the rate of our interrupt timer
  dds_rate = 40; //20 microseconds = 50KHz
  dds_tune = (1.00 / (dds_rate * .000001)); //used to make the oscillation at the frequency we want them to be
  timer1.begin(osc1, dds_rate);
}

void osc1() { // code that is run whenever the timer goes off, every 20 micros

  out1 = oscillator(0, freq1, mod[0], .5); //oscillator(voice select, frequency, amplitude, shape)
  out2 = oscillator(1, .0002, 1, .5); //oscillator(voice select, frequency, amplitude, shape)

  mod[2] = (out2 + 2048) / 4095.0;
  out3 = fold(out1 * fold_amount); //folds the input instead of clipping it based on the level of the volume pot.


  if (cu - previous_time[1] > time_interval[0]) {
    previous_time[1] = cu;
    if (lfo_latch[0] == 0) {
      lfo[0] *= .97;
    }
    if (lfo[0] < 1 ) { //4095 is the highest number we can analogWrite since we have it set to 12 bits
      lfo[0] = 0;
      lfo_latch[0] = 1;
    }
    mod[0] = lfo[0] / 4095.0;
    analogWrite(led1_pin, lfo[0]);
  }


  analogWrite(A14, out3 + 2048); // The oscillators produce numbers between -2048 and 2048 but the DAC can't output negatove numbers so we add the offset back in
}

void loop()
{
  cu = micros();
  current_time = millis();
  prev_button_state = button_state;
  button_state = digitalRead(button_pin);

  if (button_state == 0 && prev_button_state == 1) {
    lfo_latch[0] = 0;
    lfo[0] = 4095.0;
    Serial.println("!");
  }

  pot_reading[0] = analogRead(A0); //Read the analog voltage at pin A0. Returns 0 for 0 volts and 4095(12 bits) for the max voltage (3.3V)
  time_interval[0] = pot_reading[0];

  pot_reading[1] = analogRead(A1);
  time_interval[1] = pot_reading[1];

  pot_reading[2] = analogRead(A2);
  freq1 = pot_reading[2] / 4.0; //divide by a float to make sure we get a float out
  freq2 = freq1 * .501; //.5 would be a perfect octave down. Detuning it a tiny bit gives it an interesting sound

  pot_reading[3] = analogRead(A3);
  freq3 = pot_reading[3] / 2.0;

  fold_amount = 1.0;



  if (cu - previous_time[2] > time_interval[1]) {
    previous_time[2] = cu;

    lfo[1] -= 10;
    if (lfo[1] < 0) { //4095 is the highest number we can analogWrite since we have it set to 12 bits
      lfo[1] = 4095;
    }
    mod[1] = (lfo[1] / 4095.0);
    analogWrite(led2_pin, lfo[1]);
  }

  if (current_time - previous_time[3] > 5) {
    previous_time[3] = current_time;

  }

  du = micros() - cu;
} //end of loop



//you can declare "global" variable wherever but the code above it can't see them.
// So you should always do your declaring at the very top
int16_t wavelength = 4095;
uint32_t accumulator[8] = {}; //these are arrays here where are 8 separate variables called accumulator. accumulator[0],accumulator[1],etc
uint32_t increment[8] = {};
uint32_t waveindex[8] = {};


//Here's a function I made to generate a waveform that can be adjusted from saw to ramp

//(voice select, frequency, amplitude, shape)
// Each separate oscillator needs a unique voice select number
// Frequency is a floating point to allow for finer control
// Amplitude is a float 0.0 to 1.0, off to full amplitude
// 0.0 shape is a saw, 0.5 is triangle and 1.0 is ramp

int16_t oscillator(byte sel, float freq, float amp, float tri_shape) {
  int16_t tout;  //"local" variables can only be seen in the {} they are in and anything in sub {}
  int waveamp = amp * 4095.00;

  int knee = tri_shape * 4095.00;

  if (knee < 0)
  {
    knee = 0;
  }

  if (knee > wavelength - 1)
  {
    knee = wavelength - 1;
  }

  increment[sel] = (4294967296.00 * (freq)) / (dds_tune);

  accumulator[sel] += increment[sel];
  waveindex[sel] = ((accumulator[sel]) >> (32 - 12)); //wavelength is 12 bits

  if (waveindex[sel] < knee) {
    tout = ((waveindex[sel]) * waveamp) / knee;
  }

  if (waveindex[sel] >= knee) {
    tout = ((((waveindex[sel] - knee) * waveamp) / (wavelength - knee)) * -1) + waveamp;
  }

  return tout - (waveamp / 2);

}

//This is meant to "fold" a waveform, creating new harmonics
// just give it an input and it returns a folded output
int16_t fold(int16_t input) {
  int16_t foldv = input;

  static int h_res = 2040;
  static int ih_res = h_res * -1;

  for (int i; i < 8; i++) {
    if (foldv > h_res) {
      foldv -= ((foldv - h_res) * 2);
    }
    if (foldv < ih_res) {
      foldv += ((foldv * -1) - (h_res)) * 2;
    }
  }
  return foldv;
}
