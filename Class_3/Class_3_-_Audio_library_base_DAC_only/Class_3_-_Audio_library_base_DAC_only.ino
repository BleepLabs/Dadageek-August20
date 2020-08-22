// The block we copied from the tool is pasted at the top
// design tool: https://www.pjrc.com/teensy/gui/

// "#include" means add another file to our code
// So far we've been copy and pasting things in but we can just tell our code to look
// in a library for more functions and data
// These are all necessary to get audio working but we don't need to do anything besides include them

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=178.33332061767578,178.00000190734863
AudioSynthWaveform       waveform2;      //xy=188.33332061767578,221.00000381469727
AudioSynthWaveform       waveform3;      //xy=198,276.0000047683716
AudioMixer4              mixer1;         //xy=344.3333053588867,227.00001525878906
AudioOutputAnalog        dac1;           //xy=557,194
AudioConnection          patchCord1(waveform1, 0, mixer1, 0);
AudioConnection          patchCord2(waveform2, 0, mixer1, 1);
AudioConnection          patchCord3(waveform3, 0, mixer1, 2);
AudioConnection          patchCord4(mixer1, dac1);
// GUItool: end automatically generated code


//You can edit the connections by hand but it's easier to do it in the audio library for now.


//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];


void setup() {
  //there's a lot we need to do in setup now but some of it is just copy paste.
  // This first group should only be done in setup

  //How much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 3.2 we can go up to about 200 but that won't leave any RAM for anyone else.
  // It's usually the delay and reverb that hog it.
  AudioMemory(10);

  //This next group can be done anywhere in the code but we want to start things with these
  // values and change some of them in the loop.
  
  //Notice we start by writing the object we want, then a period, then the function
  // begin(volume from 0.0-1.0 , frequency , shape of oscillator) 
  // See the tool for more info https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  waveform1.begin(1, 220.0, WAVEFORM_SINE);
  waveform2.begin(1, 440.0, WAVEFORM_SINE);

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

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64); 

}

void loop() {
  cm = millis();

  //We don't have to do anything in the loop since the audio library will jut keep doing what we told it in the setup

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
