// The block we copied from the tool is pasted at the top
// design tool: https://www.pjrc.com/teensy/gui/

// "#include" means add another file to our code
// So far we've been copy and pasting things in but we can just tell our code to look
// in a library for more functions and data
// These are all necessary to get audio working but we don't need to do anything besides include them

#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated waveformMod2;   //xy=236.0000114440918,140.88889503479004
AudioSynthWaveformModulated waveformMod1;   //xy=247,70.00000286102295
AudioSynthWaveformModulated waveformMod3;   //xy=273.00000762939453,314.88889503479004
AudioSynthWaveformModulated waveformMod4;   //xy=306,362.8888988494873
AudioEffectEnvelope      envelope2;      //xy=418.82384490966797,196.00000476837158
AudioEffectEnvelope      envelope1;      //xy=428.82387924194336,41
AudioFilterStateVariable filter1;        //xy=578,86.88888549804688
AudioFilterStateVariable filter2;        //xy=585.0000801086426,207.88890647888184
AudioFilterStateVariable filter3;        //xy=739.4000358581543,224.00000286102295
AudioMixer4              mixer1;         //xy=870.82386302948,88
AudioEffectChorus        chorus1;        //xy=942.8240127563477,150.00001430511475
AudioOutputI2S           i2s1;           //xy=1090.824062347412,253.00000381469727
AudioAmplifier           amp1;           //xy=1099.8239631652832,198.00002765655518
AudioMixer4              mixer2;         //xy=1100.8238677978516,111
AudioConnection          patchCord1(waveformMod2, envelope2);
AudioConnection          patchCord2(waveformMod1, envelope1);
AudioConnection          patchCord3(waveformMod3, 0, filter1, 1);
AudioConnection          patchCord4(waveformMod3, 0, filter3, 1);
AudioConnection          patchCord5(waveformMod4, 0, filter2, 1);
AudioConnection          patchCord6(envelope2, 0, filter2, 0);
AudioConnection          patchCord7(envelope1, 0, filter1, 0);
AudioConnection          patchCord8(filter1, 0, mixer1, 0);
AudioConnection          patchCord9(filter2, 0, filter3, 0);
AudioConnection          patchCord10(filter3, 2, mixer1, 1);
AudioConnection          patchCord11(mixer1, chorus1);
AudioConnection          patchCord12(mixer1, 0, mixer2, 0);
AudioConnection          patchCord13(chorus1, 0, mixer2, 1);
AudioConnection          patchCord14(amp1, 0, i2s1, 0);
AudioConnection          patchCord15(mixer2, amp1);
AudioControlSGTL5000     sgtl5000_1;     //xy=1072.8239631652832,323.0000525712967
// GUItool: end automatically generated code

//You can edit the connections by hand but it's easier to do it in the audio library for now.

#include <MIDI.h>

//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];
int new_note[12], old_note[12];

#define CHORUS_LEN 10000  // At 44.1 kHz sample rate every 1000 integers is 22.6 milliseconds
// Chorus doesn't use the audioMemory() function. Instead you make an array for it
short chorus_bank[CHORUS_LEN]; //short is a special type similar to int and must be used for some audio functions for no good reason
int type, note_reading, velocity, channel, d1, d2, cc_num, cc_val, cc_send_val;

const float midi_frequencies[108] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861};
int midi_print = 1;


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
  waveformMod1.begin(1, 220.0, WAVEFORM_SAWTOOTH);
  waveformMod2.begin(1, 220.05, WAVEFORM_SAWTOOTH);
  waveformMod3.begin(1, .1, WAVEFORM_SINE);
  waveformMod4.begin(1, .3, WAVEFORM_TRIANGLE);

  chorus1.begin(chorus_bank, CHORUS_LEN, 3);

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

  envelope1.noteOn();
  envelope2.noteOn();

  filter1.resonance(1.5);
  filter2.resonance(1.5);

  filter1.frequency(500);
  filter2.frequency(2000);
  filter1.octaveControl(4);
  filter2.octaveControl(1);

  filter3.frequency(400);
  filter3.octaveControl(1);
  
}

void loop() {
  cm = millis();

  //We don't have to do anything in the loop since the audio library will jut keep doing what we told it in the setup

 old_note[channel] = new_note[channel];
  //Always do this in the bottom of the loop, not in a timing if
  if (usbMIDI.read() == 1) { // Is there a MIDI message incoming ?
    type = usbMIDI.getType();
    switch (type) {  //switch is very similar to if and is used when there are several specific valuse to compare the input to
      case midi::NoteOn: //if type == noteon
        note_reading = usbMIDI.getData1(); //note number
        velocity = usbMIDI.getData2(); //amplitude of note
        channel = usbMIDI.getChannel(); //MIDI channel of note
        if (velocity > 0) {  //Some systems send note off as note on 0 Velocity
          new_note[channel] = note_reading;
          if (midi_print == 1) Serial.println(String("Note On:  ch=") + channel + ", note=" + note_reading + ", velocity=" + velocity); //fancy way of printing lots of info
        } else {
          new_note[channel] = 0;
          if (midi_print == 1) Serial.println(String("Note Off VEL: ch=") + channel + ", note=" + note_reading);
        }
        break; //this case is done
      case midi::NoteOff:
        note_reading = usbMIDI.getData1();
        velocity = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        new_note[channel] = 0;
        if (midi_print == 1) Serial.println(String("Note Off: ch=") + channel + ", note=" + note_reading + ", velocity=" + velocity);
        break;
      case  midi::ControlChange:
        cc_num = usbMIDI.getData1();
        cc_val = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        if (midi_print == 1) Serial.println(String("CC, num=") + cc_num + ", val= " + cc_val + " ch=" + channel);
        break;
      default: //none of the others happened so do this one
        d1 = usbMIDI.getData1();
        d2 = usbMIDI.getData2();
        //There are lots of MIDI sync messages that you might not want to see
        // uncomment this to get the firehouse
        //Serial.println(String("Message, type=") + type + ", data = " + d1 + " " + d2);
    }
  }

  if (old_note[1] != new_note[1]) {
    if (new_note[1] > 0) {
      envelope1.noteOn();
      waveformMod1.frequency(midi_frequencies[new_note[1]]);
    }
    if (new_note[1] == 0) {
      envelope1.noteOff();
    }
  }

  if (old_note[2] != new_note[2]) {
    if (new_note[2] > 0) {
      envelope2.noteOn();
      waveformMod2.frequency(midi_frequencies[new_note[2]]);
    }
    if (new_note[2] == 0) {
      envelope2.noteOff();
    }
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
