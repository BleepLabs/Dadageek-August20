#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform2;      //xy=78.33332061767578,209.99999904632568
AudioSynthWaveform       waveform3;      //xy=84.99999999999999,264.99999999999994
AudioSynthWaveform       waveform1;      //xy=86.33332061767578,157.99999904632568
AudioSynthWaveform       waveform4;      //xy=99.16666666666666,326.66666666666663
AudioMixer4              mixer1;         //xy=280.3333044052124,187.0000114440918
AudioOutputI2S           i2s1;           //xy=463.33331298828125,197.66666221618652
AudioConnection          patchCord1(waveform2, 0, mixer1, 1);
AudioConnection          patchCord2(waveform3, 0, mixer1, 2);
AudioConnection          patchCord3(waveform1, 0, mixer1, 0);
AudioConnection          patchCord4(waveform4, 0, mixer1, 3);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=287.3333206176758,89.99999904632568
// GUItool: end automatically generated code

const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};

//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];
float freq[8];
int melody1[8] = {15, 22, 12, 14, 22, 15, 17, 30};
int melody2[8] = {15, 22, 12, 14, 22, 15, 17, 30};

int melody1_pos;
int melody_rate;
int button1, prev_button1;
int button_pin = 0;

void setup() {

  pinMode(button_pin, INPUT_PULLUP);

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
  sgtl5000_1.volume(0.8);

  //This next group can be done anywhere in the code but we want to start things with these
  // values and change some of them in the loop.

  //Notice we start by writing the object we want, then a period, then the function
  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  // See the tool for more info https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.begin(1, 220.0, WAVEFORM_SINE);
  waveform2.begin(1, 440.0, WAVEFORM_SINE);
  waveform3.begin(1, 220.0, WAVEFORM_SINE);
  waveform4.begin(1, 440.0, WAVEFORM_SINE);

  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  // A gain of 1 means the output is the same as the input.
  // .5 would be half volume and 2 would be double
  // Since we have two oscillators coming in that are already "1" We should take them down by half so we don't clip.
  // If you go over "1" The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, 0);
  mixer1.gain(1, 0);
  mixer1.gain(2, .5);
  mixer1.gain(3, .5);

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

}

void loop() {
  cm = millis();

  //We don't have to do anything in the loop since the audio library will jut keep doing what we told it in the setup

  prev_button1 = button1;
  button1 = digitalRead(button_pin);

  if (prev_button1 == 1 && button1 == 0) {
    for (int j; j < 8; j += 2) {
      melody2[j] = random(10, 30);
    }
    //melody2[0] = random(10, 30);  or just do sepdfthfdhecific steps
    //melody2[4] = random(10, 30);
  }


  if (cm - prev[2] > melody_rate) {
    prev[2] = cm;
    melody1_pos = melody1_pos + 1;
    if (melody1_pos > 7) {
      melody1_pos = 0;
    }

    int piano_loc = melody1[melody1_pos];
    if (piano_loc > 88) {
      piano_loc = 88;
    }
    freq[0] = chromatic[piano_loc];
    freq[1] = freq[0] * 2.0;

    waveform1.frequency(freq[0]);
    waveform2.frequency(freq[1]);

    freq[2] = chromatic[melody2[melody1_pos]];
    freq[3] = freq[2] * 2.0;
    waveform3.frequency(freq[2]);
    waveform4.frequency(freq[3]);
  }

  if (cm - prev[1] > 20) {
    prev[1] = cm;
    melody_rate = analogRead(A0) / 4.0;
    //freq[2] = ((analogRead(A1) / 4095.0) * 3.5) + .5;
    freq[2] = (analogRead(A1) / 4095.0) * 88.0 ;
  }


  if (cm - prev[0] > 700) {
    prev[0] = cm;

    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // This pretty much all a copy paste
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

  }
}
