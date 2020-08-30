#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformDc     dc1;            //xy=123.33806991577148,44.17140197753906
AudioEffectEnvelope      envelope3;      //xy=190.00472259521484,85.83806610107422
AudioSynthWaveformModulated waveformMod1;   //xy=247.00470733642578,135.27746391296387
AudioSynthNoisePink      pink1;          //xy=265.83805084228516,192.27746200561523
AudioEffectDelay         delay1;         //xy=393.17139053344727,518.2774658203125
AudioMixer4              mixer1;         //xy=406.17139053344727,275.2774658203125
AudioEffectEnvelope      envelope2;      //xy=413.8380699157715,193.44412565231323
AudioMixer4              mixer2;         //xy=413.17139053344727,377.2774658203125
AudioEffectEnvelope      envelope1;      //xy=423.17139053344727,136.2774658203125
AudioMixer4              mixer4;         //xy=590.004731496175,208.94413503011066
AudioMixer4              mixer3;         //xy=612.1713905334473,336.2774658203125
AudioOutputI2S           i2s1;           //xy=740.5046768188477,193.6108112335205
AudioConnection          patchCord1(dc1, envelope3);
AudioConnection          patchCord2(envelope3, 0, waveformMod1, 0);
AudioConnection          patchCord3(waveformMod1, envelope1);
AudioConnection          patchCord4(pink1, envelope2);
AudioConnection          patchCord5(delay1, 0, mixer2, 1);
AudioConnection          patchCord6(mixer1, 0, mixer2, 0);
AudioConnection          patchCord7(mixer1, 0, mixer3, 0);
AudioConnection          patchCord8(envelope2, 0, mixer1, 1);
AudioConnection          patchCord9(mixer2, delay1);
AudioConnection          patchCord10(mixer2, 0, mixer3, 1);
AudioConnection          patchCord11(envelope1, 0, mixer1, 0);
AudioConnection          patchCord12(mixer4, 0, i2s1, 0);
AudioConnection          patchCord13(mixer4, 0, i2s1, 1);
AudioConnection          patchCord14(mixer3, 0, mixer4, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=203.17139053344727,319.2774658203125
// GUItool: end automatically generated code

//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];
#define num_of_buttons 2
int button_pin[num_of_buttons] = {10, 12}; //lets put the button pins in an array
int button_read[num_of_buttons];
int prev_button_read[num_of_buttons];
float wet_level, dry_level, fb_level, delay_time, vol;
int  smoothed_reading[4];
float freq1;
int seq_index, prev_seq_index, led_tick, seq_rate;



#define steps_in_seq 64

// I put a simple beat in to start
// heres 2 dinamsion array. the first will be for one drum and the secodns for another.
// instaed of jsut 1 or 0 it could be used for anything like velocity. Of have other dimaentions for other variables

byte sequence[2][steps_in_seq] = {
  {
    1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0,
  },

  {
    1, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 0, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0,  1, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0,  1, 0, 1, 0, 1, 0, 1, 0,
  },

};

void setup() {

  pinMode(button_pin[0], INPUT_PULLUP);
  pinMode(button_pin[1], INPUT_PULLUP);
  pinMode(5, OUTPUT);

  AudioMemory(100);
  pink1.amplitude(1);

  // every 10 milliseconds needs 3 blocks of memory in AudioMemory
  delay1.delay(0, 10); //needs 84 blocks on its own.

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13); //The volume of the input. Again we'll get to this later

  sgtl5000_1.volume(0.25);
  waveformMod1.begin(1, 70.0, WAVEFORM_SINE);
  dc1.amplitude(1.0);

  mixer1.gain(0, .5); //combine both oscillators
  mixer1.gain(1, .5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  //delay mixer
  mixer2.gain(0, .75); //input
  mixer2.gain(1, 0); //feedback
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  //wet / dry  mixer
  mixer3.gain(0, 1); //dry
  mixer3.gain(1, 0); //wet
  mixer3.gain(2, 0);
  mixer3.gain(3, 0);
  //final output level
  mixer4.gain(0, 1);
  mixer4.gain(1, 0);
  mixer4.gain(2, 0);
  mixer4.gain(3, 0);


  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

  envelope2.attack(1);
  envelope2.decay(25 );
  envelope2.sustain(.2);
  envelope2.release(500); //how long will it take for the notes to fade out in milliseconds

  envelope1.attack(0);
  envelope1.release(50);


  envelope3.attack(0);
  envelope3.decay(50);
  envelope2.sustain(0);

}


void loop() {
  cm = millis();

  for (int j = 0; j < 2; j++) {
    prev_button_read[j] = button_read[j];
    button_read[j] = digitalRead(button_pin[j]);
  }

  if (prev_button_read[1] == 1 &&  button_read[1] == 0) {
    envelope1.noteOn();
    envelope3.noteOn();
    if (button_read[0] == 0) {  //record a note only if the other button is down
      sequence[0][seq_index] = 1;
    }
  }
  if (prev_button_read[1] == 0 &&  button_read[1] == 1) {
    envelope1.noteOff();
    envelope3.noteOff();
  }



  if (cm - prev[2] > seq_rate) {
    prev[2] = cm;
    prev_seq_index = seq_index;
    seq_index++;

    if (seq_index % 8 == 0) { //blink every 8 steps
      analogWrite(5, 64);
    }
    else {
      analogWrite(5, 0);
    }

    if (seq_index > steps_in_seq - 1) {
      seq_index = 0;
    }

    if (sequence[0][seq_index] > 0) {
      envelope1.noteOn();
      envelope3.noteOn();

    }

    //only do a note off if there was a note last time
    // could be inpvoved to only have a noteOn if there was a 0 last time so you could hold notes
    if (sequence[0][prev_seq_index] > 0 && sequence[0][seq_index] == 0) {
      envelope1.noteOff();
      envelope3.noteOff();
    }

    if (sequence[1][seq_index] > 0) {
      envelope2.noteOn();
    }
    if (sequence[1][prev_seq_index] > 0 && sequence[1][seq_index] == 0) {
      envelope2.noteOff();
    }


  }

  if (cm - prev[0] > 5) {
    prev[0] = cm;
    seq_rate = analogRead(A0) / 40;

    vol = (analogRead(A3) / 4095.0);
    mixer4.gain(0, vol);

    freq1 = (analogRead(A6));
    waveformMod1.frequency(freq1 / 100.0);

    /*
        wet_level = (analogRead(A0) / 4095.0); //0.0 - 1.0
        dry_level = (wet_level - 1.0) * -1.0; //1.0-0.0
        mixer3.gain(0, dry_level);
        mixer3.gain(1, wet_level);

        fb_level = (analogRead(A1) / 4095.0) * 1.2;
        mixer2.gain(1, fb_level);

        delay_time = analogRead(A2);
        smoothed_reading[0] = ((smooth(0, 49, delay_time)) / 4095.0) * 240.0;
        delay1.delay(0, smoothed_reading[0]);


    */
  }


  if (cm - prev[1] > 100) {
    prev[1] = cm;
    Serial.println(seq_rate);
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

  }
}


//based on https://playground.arduino.cc/Main/DigitalSmooth/
// This function continuously samples an input and puts it in an array that is "samples" in length.
// This array has a new "raw_in" value added to it each time "smooth" is called and an old value is removed
// It throws out the top and bottom 15% of readings and averages the rest

#define maxarrays 1 //max number of different variables to smooth
#define maxsamples 51 //max number of points to sample and 
//reduce these numbers to save RAM

unsigned int smoothArray[maxarrays][maxsamples];

// sel should be a unique number for each occurrence
// samples should be an odd number greater that 7. It's the length of the array. The larger the more smooth but less responsive
// raw_in is the input. positive numbers in and out only.

unsigned int smooth(byte sel, unsigned int samples, unsigned int raw_in) {
  int j, k, temp, top, bottom;
  long total;
  static int i[maxarrays];
  static int sorted[maxarrays][maxsamples];
  boolean done;

  i[sel] = (i[sel] + 1) % samples;    // increment counter and roll over if necessary. -  % (modulo operator) rolls over variable
  smoothArray[sel][i[sel]] = raw_in;                 // input new data into the oldest slot

  for (j = 0; j < samples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[sel][j] = smoothArray[sel][j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (samples - 1); j++) {
      if (sorted[sel][j] > sorted[sel][j + 1]) {    // numbers are out of order - swap
        temp = sorted[sel][j + 1];
        sorted[sel] [j + 1] =  sorted[sel][j] ;
        sorted[sel] [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((samples * 15)  / 100), 1);
  top = min((((samples * 85) / 100) + 1  ), (samples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[sel][j];  // total remaining indices
    k++;
  }
  return total / k;    // divide by number of samples
}
