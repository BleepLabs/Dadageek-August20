/*

  It's a big leap from making sound we way we did before to something that sounds like a "real" synth
  You can only ever do one thing at a time on any computer and you can only do it at one speed.
  Teensy3.2 can do a simple opperation 96 million times a seconds. Things take a lot of little operations to do but still it's fast
  but what if you want a light blinking at something faster than 1 millisecond but slower than 96MHz?

  You can interip the loop after a set amount of micro or even nano seconds have passed. We can kind of thinking this like multitasking
  the loop is still looping but ever X nano seconds it stops what it's doing and does the things that are in the interval

*/

IntervalTimer timer1;

int led1_pin =  10;
int led1_state = LOW;
int led2_pin =  9;
int led2_state = 0;
unsigned long  previous_time1 = 0;
unsigned long  previous_time2 = 0;
unsigned long  previous_time3 = 0;
unsigned long current_time;
unsigned long interval1 = 50;
unsigned long interval2 = 50;
unsigned long interval3 = 100;
int button_pin;
int button_state;
int pot1_value;
int pot1_pin = A0;
int pot2_value;
int pot2_pin = A1;
float amp1;
float freq1; //floats can hold decimals
float freq2;
float freq3;

//int out1, out2, out3;// You can define varbiles like this too
int out[8];
int pots[8];
const float chromatic[108] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861};

uint32_t dds_tune;
uint32_t dds_rate;

void setup() {
  pinMode(led1_pin, OUTPUT);
  pinMode(led2_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);

  analogWriteResolution(12);
  //set the rate of our interupt timer
  dds_rate = 50; //20 micoseconds = 50KHz
  dds_tune = (1.00 / (dds_rate * .000001)); //used to make the oscialltoisn at the freqency we want them to be
  
  timer1.begin(osc1, dds_rate);
}

void osc1() { // code that is run whenv er the timer goes off, every 20 micros
  int32_t final_out;
  int scale;

  for (int m = 0; m < 5; m++) {
    out[m] = vtri(m, (chromatic[m+32+scale])*amp1, 4000, 128) ;
    scale += 5;
    final_out += out[m];
  }
  analogWrite(A14, (final_out/6) + 2048 );
}

uint32_t dt;
void loop()
{
  //everything happens like normal in the loop it jsut takes a little loger to finish. But like milliseconds longer.
  current_time = millis();

  button_state = digitalRead(button_pin); //if the button is not being pressed it will read HIGH. if it pressed it will read LOW

  pots[0] = analogRead(A0); //Read the analog voltage at pin A0. Returns 0 for 0 volts and 1023 for the max voltage (3.3V)
  interval1 = pots[0] / 2;
  interval2 = pots[0] / 5;
  freq1 = pots[0] / 4.0; //divide by a float to make sure we get a float out
  freq2 = freq1 * 2.00;
  freq3 = freq1 / 2.00;

  pots[1] = analogRead(A1) / 4; //this value is 0-1024 but analogwrite is 0-255 so we divide by 4. We'll talk about these funny numbers later
  amp1 = pots[1] * (4.0 / 255.0);

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

  if (current_time - previous_time3 > interval3) {  //dont go faster than 10 milliseconds
    previous_time3 = current_time;

    for (int j = 0; j < 8; j += 1) {
      Serial.print(j);
      Serial.print("-");
      Serial.print(pots[j]);
      Serial.print("  ");
    }
    Serial.println();

  }

}

//you can declare "global" varible wherever
int16_t wavelength = 255;
uint32_t accumulator[8] = {}; //these are arrays here where are 8 seperate varibels calles accumulator. accumulator[0],accumulator[1],etc
uint32_t increment[8] = {};
uint32_t waveindex[8] = {};

//here's a function I made to generate a waform that can be adjusted from saw to square

int16_t vtri(byte sel, float freq, uint16_t waveamp, uint16_t inknee) {
  int16_t tout;  //local varibles can only be seen in the {} they are in and anyhing in sub {}
  int16_t knee;

  knee = inknee ;

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
  waveindex[sel] = ((accumulator[sel]) >> 24);

  if (waveindex[sel] < knee) {
    tout = ((waveindex[sel]) * waveamp) / knee;
  }

  if (waveindex[sel] >= knee) {
    tout = ((((waveindex[sel] - knee) * waveamp) / (wavelength - knee)) * -1) + waveamp;
  }

  return tout - (waveamp / 2);

}

// This is meant to "fold" a waveform, creating new harmonics

int16_t fold(int16_t input) {
  int16_t foldv = input;

  static int h_res = 2040;
  static int ih_res = -2040;

  for (int i; i < 5; i++) {
    if (foldv > h_res) {
      foldv -= ((foldv - h_res) * 2);
    }
    if (foldv < ih_res) {
      foldv += ((foldv * -1) - (h_res)) * 2;
    }
  }
  return foldv;
}
