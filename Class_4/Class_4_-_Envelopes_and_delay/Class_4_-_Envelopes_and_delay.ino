/*
  Note how the envelope audio objects were renamed

*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated waveformMod2;   //xy=313,113.00000286102295
AudioSynthWaveformModulated waveformMod1;   //xy=330,65.00000190734863
AudioEffectDelay         delay1;         //xy=477.00000762939453,448.00000762939453
AudioMixer4              mixer1;         //xy=490.00000381469727,205.00000190734863
AudioMixer4              mixer2;         //xy=497.00000762939453,307.00000381469727
AudioEffectEnvelope      envelope[2];      //xy=501.00000762939453,109.00000190734863
AudioOutputI2S           i2s1;           //xy=691.0000152587891,185.00000190734863
AudioMixer4              mixer3;         //xy=696.0000114440918,266.00000381469727
AudioConnection          patchCord1(waveformMod2, envelope[1]);
AudioConnection          patchCord2(waveformMod1, envelope[0]);
AudioConnection          patchCord3(delay1, 0, mixer2, 1);
AudioConnection          patchCord4(mixer1, 0, mixer2, 0);
AudioConnection          patchCord5(mixer1, 0, mixer3, 0);
AudioConnection          patchCord6(mixer2, delay1);
AudioConnection          patchCord7(mixer2, 0, mixer3, 1);
AudioConnection          patchCord8(envelope[1], 0, mixer1, 1);
AudioConnection          patchCord9(envelope[0], 0, mixer1, 0);
AudioConnection          patchCord10(mixer3, 0, i2s1, 0);
AudioConnection          patchCord11(mixer3, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=287.00000762939453,249.00000381469727
// GUItool: end automatically generated code


//You can edit the connections by hand but it's easier to do it in the audio library for now.


//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];
int button_pin[2] = {10, 12}; //lets put the button pins in an array
int button_read[2];
int prev_button_read[2];

void setup() {

  pinMode(button_pin[0], INPUT_PULLUP);
  pinMode(button_pin[1], INPUT_PULLUP);

  //How much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 3.2 we can go up to about 200 but that won't leave any RAM for anyone else.
  // It's usually the delay and reverb that hog it.
  AudioMemory(100);
  // start the delay delay(output channel, milliseconds of delay time)
  // every 10 milliseconds needs 3 blocks of memory in AudioMemory

  delay1.delay(0, 250); //needs 84 blocks on its own.


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
  waveformMod1.begin(1, 220.0, WAVEFORM_SINE);
  waveformMod2.begin(1, 440.0, WAVEFORM_SINE);

  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  // A gain of 1 means the output is the same as the input.
  // .5 would be half volume and 2 would be double
  // Since we have two oscillators coming in that are already "1" We should take them down by half so we don't clip.
  // If you go over "1" The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, .5);
  mixer1.gain(1, .5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  mixer2.gain(0, .75);
  mixer2.gain(1, 0);
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  mixer2.gain(0, .5);
  mixer2.gain(1, .5);
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

  envelope[0].release(1000); //how long will it take for the notes to fade out
  envelope[1].release(100);

}

void loop() {
  cm = millis();

  //This one "for" upades both the buttons.
  // If we had more buttons all we'd need to do is change j < 2 and the array sizes
  for (int j = 0; j < 2; j++) {
    prev_button_read[j] = button_read[j];
    button_read[j] = digitalRead(button_pin[j]);
    if (prev_button_read[j] == 1 &&  button_read[j] == 0) {
      envelope[j].noteOn();
    }
    if (prev_button_read[j] == 0 &&  button_read[j] == 1) {
      envelope[j].noteOff();
    }
  }


  //reading and changing values should usually be slowed down a little
  if (cm - prev[0] > 5) {
    prev[0] = cm;

    float wet = (analogRead(A0) / 4095.0);
    float dry = (wet - 1.0) * -1.0;
    float fb = (analogRead(A1) / 4095.0) * 1.2;
    float dt = (analogRead(A2) / 4095.0) * 240.0;
    mixer2.gain(1, fb);
    delay1.delay(0, dt);
    mixer3.gain(0, dry);
    mixer3.gain(1, wet);
  }

  if (cm - prev[1] > 100) {
    prev[1] = cm;
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

  }
}
