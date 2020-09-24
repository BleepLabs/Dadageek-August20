// Sending out a MIDI clock at a very presise BPM
// according to this program it's a tiny bit more accurate than ableton but that's just a few tests on my system with simple code.
// http://www.users.on.net/~mcdds001/mmmmqac/midi_jitter.html

#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.
#include "MIDI.h"
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

IntervalTimer timer1; //Make a new interval timer called timer1

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform2;      //xy=167,267
AudioSynthWaveform       waveform1;      //xy=175,215
AudioMixer4              mixer1;         //xy=383,245
AudioOutputI2S           i2s1;           //xy=591,244
AudioConnection          patchCord1(waveform2, 0, mixer1, 1);
AudioConnection          patchCord2(waveform1, 0, mixer1, 0);
AudioConnection          patchCord3(mixer1, 0, i2s1, 0);
AudioConnection          patchCord4(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=484,355
// GUItool: end automatically generated code

unsigned long cm;
unsigned long prev[8];
uint32_t increment, accumulator, findex;
float freq, dds_tune, dds_rate, bpm;
int tempo_change;

void setup() {
  AudioMemory(10);
  MIDI.begin();

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13); //The volume of the input. Again we'll get to this later
  sgtl5000_1.volume(0.25);

  bpm = 120; //intended beats per minute
  // 60000 millis in a minute / 24 pulses per measure / bpm * 1000 to make it microseconds
  dds_rate = ((60000 / 24) / bpm) * 1000;
  //dds_tune = (1.00 / (dds_rate * .000001)); //used to make the oscillation at the frequency we want them to be
  timer1.begin(midi_clock, dds_rate); //begin(name of function to run, microsends to run it at)
  timer1.priority(0); //highest priority. This means it will overide the audio code but as long as you don't put too much in the interupt you'll be fine

  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);
}


void midi_clock() {
  MIDI.sendClock(); // do this first in the interupt to insure acutare timing
  // you can do plent of other MIDI commands in there too but do any analogReads in the loop
}


void loop() {
  cm = millis();

  //to change the rate do this but only run it when needed, not continuosly.
  if (tempo_change == 1) {
    bpm = 120;
    dds_rate = ((60000 / 24) / bpm) * 1000;
    timer1.update(dds_rate);
  }


  if (cm - prev[0] > 500) {
    prev[0] = cm;

    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

  }
}
