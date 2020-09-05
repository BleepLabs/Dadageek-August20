/*
  USB Midi control
  The Teensy must be set to MIDI in Tools>USB type>Serial+MIDI
  Using regular DIN MIDI is very similar but you need some additional hardware https://www.pjrc.com/teensy/td_libs_MIDI.html
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform2;      //xy=73.88890075683594,229.00002479553223
AudioSynthWaveform       waveform1;      //xy=84.8888931274414,172.00001525878906
AudioEffectEnvelope      envelope2;      //xy=233.88890838623047,254.8888816833496
AudioEffectEnvelope      envelope1;      //xy=240.888916015625,197.8888816833496
AudioMixer4              mixer1;         //xy=462.8888702392578,261.00002098083496
AudioMixer4              mixer2;         //xy=574.8888854980469,341.8888854980469
AudioEffectChorus        chorus1;        //xy=597.888843536377,260.88892364501953
AudioOutputI2S           i2s1;           //xy=690.8887557983398,435.0000367164612
AudioAmplifier           amp1;           //xy=700.8888854980469,340.8888854980469
AudioConnection          patchCord1(waveform2, envelope2);
AudioConnection          patchCord2(waveform1, envelope1);
AudioConnection          patchCord3(envelope2, 0, mixer1, 1);
AudioConnection          patchCord4(envelope1, 0, mixer1, 0);
AudioConnection          patchCord5(mixer1, chorus1);
AudioConnection          patchCord6(mixer1, 0, mixer2, 0);
AudioConnection          patchCord7(mixer2, amp1);
AudioConnection          patchCord8(chorus1, 0, mixer2, 1);
AudioConnection          patchCord9(chorus1, 0, mixer1, 2);
AudioConnection          patchCord10(amp1, 0, i2s1, 0);
AudioConnection          patchCord11(amp1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=395.88891220092773,430.00001668930054
// GUItool: end automatically generated code


const float midi_frequencies[108] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861};

#include <MIDI.h>

#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.


#define CHORUS_LEN 20000  // At 44.1 kHz sample rate every 1000 integers is 22.6 milliseconds
// Chorus doesn't use the audioMemory() function. Instead you make an array for it
short chorus_bank[CHORUS_LEN]; //short is a special type similar to int and must be used for some audio functions for no good reason

//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];

int type, note_reading, velocity, channel, d1, d2, cc_num, cc_val, cc_send_val;
int new_note[8], old_note[8];
int midi_print = 0;
float ch_wet, ch_dry;
int button1_pin = 10;
int button1, prev_button1;
int pot[6], prev_pot[6];

void setup() {

  pinMode(button1_pin, INPUT_PULLUP);

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
  waveform1.begin(1, 220.0, WAVEFORM_TRIANGLE);
  waveform2.begin(1, 440.0, WAVEFORM_TRIANGLE);

  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  // A gain of 1 means the output is the same as the input.
  // .5 would be half volume and 2 would be double
  // Since we have two oscillators coming in that are already "1" We should take them down by half so we don't clip.
  // If you go over "1" The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, .4);
  mixer1.gain(1, .4);
  mixer1.gain(2, .3); //chorus feedback
  mixer1.gain(3, 0);

  mixer2.gain(0, .2); //dry
  mixer2.gain(1, .8); //wet from chorus
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  amp1.gain(1); ///final volume

  //Chorus is the kind of effect you just setup then control the wet/dry of
  // You can change the number of voices, though.
  //begin(array to use, size of array, number of voices)
  chorus1.begin(chorus_bank, CHORUS_LEN, 3);

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

}

void loop() {
  cm = millis();
  old_note[channel] = new_note[channel];
  //Always do this in the bottom of the loop, not in a timing if
  if (usbMIDI.read()==1) { // Is there a MIDI message incoming ?
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
      waveform1.frequency(midi_frequencies[new_note[1]]);
    }
    if (new_note[1] == 0) {
      envelope1.noteOff();
    }
  }

    if (old_note[2] != new_note[2]) {
    if (new_note[2] > 0) {
      envelope2.noteOn();
      waveform2.frequency(midi_frequencies[new_note[2]]);
    }
    if (new_note[2] == 0) {
      envelope2.noteOff();
    }
  }

  if (cc_num == 24) {
    ch_wet = (cc_val / 127.0); //MIDI values are 7 bit 0-127
    ch_dry = 1.0 - ch_wet; //flip it around the easy way
    mixer2.gain(0, ch_dry); //dry
    mixer2.gain(1, ch_wet); //wet from chorus
  }


  prev_button1 = button1;
  button1 = digitalRead(button1_pin);

  if (prev_button1 == 1 && button1 == 0) {
    //(note number, velocity, channel)
    usbMIDI.sendNoteOn(60, 127, 1);  // 60 = C4
  }

  if (prev_button1 == 0 && button1 == 1) {
    //(note number, velocity, channel)
    usbMIDI.sendNoteOff(60, 0, 1); //its best to send velocity 0 with note off
  }

//we slow this down which makes the smooth laggy but we take less smooth measurements
// this keeps it from sending jittery CC data
  if (cm - prev[1] > 2) { 
    prev[1] = cm;
    prev_pot[0] = pot[0];
    pot[0] = smooth(0, 15, analogRead(A1) / 32) ; //midi is only 0-127
    if (prev_pot[0] != pot[0] ) {
      cc_send_val = pot[0];
      //(control number, value, channel
      usbMIDI.sendControlChange(30, cc_send_val, channel);
    }
  }

  if (cm - prev[0] > 500 && 1 == 1) { //change it to 1==1 for it to print
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
