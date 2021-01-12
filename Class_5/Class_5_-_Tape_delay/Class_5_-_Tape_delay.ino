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
AudioInputI2S            i2s2;           //xy=241.00000762939453,197.99999678134918
AudioMixer4              mixer1;         //xy=468,257.00001525878906
AudioEffectTapeDelay         tapeDelay1;     //xy=472.00000190734863,403.00003814697266
AudioMixer4              mixer2;         //xy=703,261
AudioOutputI2S           i2s1;           //xy=884.9999809265137,356.00001525878906
AudioConnection          patchCord1(i2s2, 0, mixer2, 0);
AudioConnection          patchCord2(i2s2, 1, mixer1, 0);
AudioConnection          patchCord3(mixer1, tapeDelay1);
AudioConnection          patchCord4(tapeDelay1, 0, mixer1, 3);
AudioConnection          patchCord5(tapeDelay1, 0, mixer2, 1);
AudioConnection          patchCord6(mixer2, 0, i2s1, 0);
AudioConnection          patchCord7(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=655.9999618530273,104
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
float peaky;

void setup() {


  AudioMemory(20);

  //info for all of these "sgtl5000" functions are in the tool here https://www.pjrc.com/teensy/gui/?info=AudioControlSGTL5000
  sgtl5000_1.enable(); //Turn the adapter board on

  //Tell it what input we want to use. The line in is accessible on the top of the audio adapter
  // See the hookup guide here
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  //The volume of the input. 0 is low, 15 is high
  sgtl5000_1.lineInLevel(12); //5 is default for line level

sgtl5000_1.volume(0.75);

   tapeDelay1.begin(tape_delay_bank, DELAY_SIZE, DELAY_SIZE/2, 0, 1);

  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  mixer1.gain(0, .5);
  mixer1.gain(1, 0);
  mixer1.gain(2, 0);
  mixer1.gain(3, .5);

  mixer2.gain(0, .2);
  mixer2.gain(1, .8);

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

}

void loop() {
  cm = millis();
  
  pot[0] = smooth(0, 35, analogRead(A0));
  pot[1] = smooth(1, 35, analogRead(A1));
  pot[2] = smooth(2, 35, analogRead(A2));

  if (cm - prev[1] > 5) {
    prev[1] = cm;
    mixer1.gain(3, pot[2] / 4095.0); // feedback
    tapeDelay1.length((peaky*(pot[1] / 4095.0)) * DELAY_SIZE);
    
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
