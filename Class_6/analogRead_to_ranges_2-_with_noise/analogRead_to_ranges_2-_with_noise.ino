
#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=202.00568771362305,159.0056667327881
AudioSynthWaveform       waveform2;      //xy=290.0056915283203,334.0056838989258
AudioEffectEnvelope      envelope1;      //xy=339.0056838989258,254.00567245483398
AudioMixer4              mixer1;         //xy=507.00567626953125,193.00566482543945
AudioEffectDigitalCombine combine1;       //xy=511.00560760498047,313.0055913925171
AudioMixer4              mixer2;         //xy=656.0056838989258,251.0056667327881
AudioAmplifier           amp1;           //xy=780.0055923461914,352.00557041168213
AudioOutputI2S           i2s1;           //xy=806.0056381225586,200.0056495666504
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(waveform2, 0, combine1, 1);
AudioConnection          patchCord3(envelope1, 0, mixer1, 0);
AudioConnection          patchCord4(mixer1, 0, combine1, 0);
AudioConnection          patchCord5(mixer1, 0, mixer2, 0);
AudioConnection          patchCord6(combine1, 0, mixer2, 1);
AudioConnection          patchCord7(mixer2, amp1);
AudioConnection          patchCord8(amp1, 0, i2s1, 0);
AudioConnection          patchCord9(amp1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=333.0056495666504,447.005672454834
// GUItool: end automatically generated code



//You can edit the connections by hand but it's easier to do it in the audio library for now.


//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];

const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};
byte major[65] = {1, 3, 5, 6, 8, 10, 12, 13, 15, 17, 18, 20, 22, 24, 25, 27, 29, 30, 32, 34, 36, 37, 39, 41, 42, 44, 46, 48, 49, 51, 53, 54, 56, 58, 60, 61, 63, 65, 66, 68, 70, 72, 73, 75, 77, 78, 80, 82, 84, 85, 87, 89, 90, 92, 94, 96, 97, 99, 101, 102};


float vol;
float pcell;
float step_size = 1.0 / 12.0;
float step_spacing = 20.0;
float freq;
int note_sel, prev_note_sel, new_note;
unsigned long new_note_time;
float filter_freq;

void setup() {
  //there's a lot we need to do in setup now but some of it is just copy paste.
  // This first group should only be done in setup

  //How much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 3.2 we can go up to about 200 but that won't leave any RAM for anyone else.
  // It's usually the delay and reverb that hog it.
  AudioMemory(10);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13); //The volume of the input. Again we'll get to this later

  //Output volume. Goes from 0.0 to 1.0 but a fully loud signal will clip over .8 or so.
  // For headphones it's pretty loud at .4
  // There are lots of places we can change the final volume level. This one you set once and leave alone.
  sgtl5000_1.volume(0.25);

  //This next group can be done anywhere in the code but we want to start things with these
  // values and change some of them in the loop.

  //Notice we start by writing the object we want, then a period, then the function
  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  // See the tool for more info https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.begin(1, 220.0, WAVEFORM_SAWTOOTH);
  waveform2.begin(1, 10.0, WAVEFORM_SINE);

  //filter1.frequency(10000);
  //filter1.resonance(2.0);
  combine1.setCombineMode(AudioEffectDigitalCombine::XOR);
  mixer1.gain(0, 1);
  mixer1.gain(1, 0);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  mixer2.gain(0, 1);
  mixer2.gain(1, 0);
  mixer2.gain(2, 0);
  mixer1.gain(3, 0);

  amp1.gain(1); //final volume

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

  envelope1.noteOn();//otherwise we won't hear it in this example
}

float xfade;
int cuttoff=1000;

void loop() {
  cm = millis();

  vol = (smooth(0, 31, analogRead(A1)));

  if (vol < cuttoff) {
    amp1.gain(vol / cuttoff); //final volume
    mixer2.gain(0, 1);
    mixer2.gain(1, 0);
  }

  if (vol >= cuttoff) {
    amp1.gain(1); //final volume
    xfade = (vol - cuttoff) / (4095-cuttoff);
    mixer2.gain(0, 1 - xfade);
    mixer2.gain(1, xfade);
    waveform2.frequency(10 + (xfade * 100));
  }


  pcell = (smooth(1, 31, analogRead(A7))) / 4095.00;

  if (cm - prev[1] > 5) {
    prev[1] = cm;
    prev_note_sel = note_sel;
    for (float j; j < 1.0; j += step_size) {
      if (pcell > j && pcell < j + step_size) {
        note_sel = (j * step_spacing) + 10;
        freq = chromatic[major[note_sel]];
        filter_freq = freq + ((pcell - j) * 30000.0); //pcell-j gives us a value that starts at 0 and goes to step_size for each range
      }
    }
    if (note_sel != prev_note_sel) {
      new_note = 1;
      new_note_time = cm;
      // envelope1.noteOn();

    }
    waveform1.frequency(freq);
    //filter1.frequency(filter_freq);

  }

  if (new_note == 1) {
    if (cm - new_note_time > 100) {
      new_note = 0;
      //envelope1.noteOff();

    }
  }

  if (cm - prev[0] > 500) {
    prev[0] = cm;
    Serial.print(filter_freq);
    Serial.print("\t"); //tab
    Serial.println(freq);

    /*
        Serial.print("processor: ");
        Serial.print(AudioProcessorUsageMax());
        Serial.print("%    Memory: ");
        Serial.print(AudioMemoryUsageMax());
        Serial.println();
        AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
        AudioMemoryUsageMaxReset();
    */
  }
}
