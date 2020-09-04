// Using the tape delay effect with an audio input
// More info https://github.com/BleepLabs/Dadageek-August20/wiki/Class-5-~-Audio-input

#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.

#include "effect_tape_delay.h" // this needs to be before the other audio code 


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioAmplifier           amp1;           //xy=124,393
AudioInputI2S            i2s2;           //xy=183,309
AudioSynthWaveformSineModulated sine_fm1;       //xy=207,473
AudioEffectMultiply      multiply1;      //xy=328,389
AudioEffectTapeDelay      tapeDelay1;      //xy=521,472
AudioMixer4              mixer1;         //xy=523,395
AudioAmplifier           amp2;           //xy=697,466
AudioOutputI2S           i2s1;           //xy=841,468
AudioConnection          patchCord1(amp1, sine_fm1);
AudioConnection          patchCord2(i2s2, 0, multiply1, 0);
AudioConnection          patchCord3(i2s2, 0, mixer1, 2);
AudioConnection          patchCord4(i2s2, 0, amp1, 0);
AudioConnection          patchCord5(sine_fm1, 0, multiply1, 1);
AudioConnection          patchCord6(sine_fm1, 0, mixer1, 1);
AudioConnection          patchCord7(multiply1, 0, mixer1, 0);
AudioConnection          patchCord8(tapeDelay1, amp2);
AudioConnection          patchCord9(tapeDelay1, 0, mixer1, 3);
AudioConnection          patchCord10(mixer1, tapeDelay1);
AudioConnection          patchCord11(amp2, 0, i2s1, 0);
AudioConnection          patchCord12(amp2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=510,293
// GUItool: end automatically generated code



//Some effects require memory to be allocated outside of the AudioMemory() function/
#define DELAY_SIZE 10000  // At 44.1 kHz sample rate every 1000 integers is 22.6 milliseconds
int16_t tape_delay_bank[DELAY_SIZE];

int pot[5];
unsigned long cm;
unsigned long prev[8];
#define button1_pin 8
#define button2_pin 12
int button1, button2, prev_button1, prev_button2;

void setup() {

  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);

  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 3.2 we can go up to about 200 but that won't leave any RAM for anyone else.
  // It's usually the delay and reverb that hog it.
  AudioMemory(10);

  //info for all of these "sgtl5000" functions are in the tool here https://www.pjrc.com/teensy/gui/?info=AudioControlSGTL5000
  sgtl5000_1.enable(); //Turn the adapter board on

  //Tell it what input we want to use. The line in is accessible on the top of the audio adapter
  // See the hookup guide here
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  //The volume of the input. 0 is low, 15 is high
  sgtl5000_1.lineInLevel(12); //5 is default for line level


  //If you want to use a microphone instead use these line. You can't use both at once
  //sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  //sgtl5000_1.micGain(20); //number is from 0-63 dB of gain. Usually this gets set pretty high

  //Headphone output volume.
  // For headphones it's pretty loud at .4
  sgtl5000_1.volume(0.25);

  //begin(bank to use, max size of bank, length of delay,rate reduction,interpolation)
  // delay time is 0 for longest delay, DELAY_SIZE-1 for shortest
  // redux is a way to make the delay longer while sacrificing sample rate. takes whole numbers
  // 0 is no reduction. 1 doubles the length while halving the sample rate, so 22kHz, 2 is 11kHz
  // interpolation takes whole numbers and is how fast the tape will get to the desired location. 0 is as fast as possible, 1 is a little slower
  tapeDelay1.begin(tape_delay_bank, DELAY_SIZE, 0, 1, 4);

  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  // See the tool for more info https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  sine_fm1.amplitude(1);
  amp1.gain(.5);
  amp2.gain(1);

  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  mixer1.gain(0, 0);
  mixer1.gain(1, 0);
  mixer1.gain(2, .7);
  mixer1.gain(3, 0);

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

}

void loop() {
  cm = millis();

  prev_button1 = button1;
  button1 = digitalRead(button1_pin);
  prev_button2 = button2;
  button2 = digitalRead(button2_pin);

  if (prev_button1 == 1 && button1 == 0) {
  }
  
  if (prev_button1 == 0 && button1 == 1) {
  }


  //smooth(channel, number of readings to average, input)
  // if we do this in the bottom of the loop, as in not in a timing if, it will respond much more quickly
  pot[0] = smooth(0, 35, analogRead(A0));
  pot[1] = smooth(1, 35, analogRead(A1));
  pot[2] = smooth(2, 35, analogRead(A2));
  pot[3] = smooth(3, 35, analogRead(A3));
  pot[4] = smooth(4, 35, analogRead(A6));

  if (cm - prev[1] > 5) {
    prev[1] = cm;
    sine_fm1.frequency(pot[0] / 4.0);
    //amp2.gain(pot[2] / 4095.0); //output volume
    //amp1.gain(pot[1] / 1000.0); //amount the incoming audio is FMing the oscillator
    mixer1.gain(3, pot[2] / 4095.0); // feedback
    tapeDelay1.length((pot[1] / 4095.0) * DELAY_SIZE);
  }

  if (cm - prev[0] > 500) {
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
