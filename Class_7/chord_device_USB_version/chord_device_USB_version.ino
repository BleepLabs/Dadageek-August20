

// arrays from https://github.com/b38tn1k/chordtoy/blob/master/chordtoy/chordtoy.ino

#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.


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

#include <MIDI.h>
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

byte midi_out_channel = 1;

int chord_select, note_enable_sel;
int note_in, note_state, new_note, inversion_sel, type_select;
int note_enable = 1;
int type, note, velocity, channel, d1, d2;
int notes_out[4];

#define num_chord_types 3
#define num_inversions_types 6
#define num_chord_note_selections 15

const byte chord_type[num_chord_types][6] = {
  {3, 7, 10, 14, 17, 21}, //minor
  {3, 6, 12, 15, 18, 24}, //dim
  {4, 7, 10, 12, 14, 17}, //major
};

const int inversions[num_inversions_types][3] = {
  {0, 0, 0},
  {0, 0, -12},
  { -12, 0, 0},
  { -12, -12, 0},
  {0, -12, -12},
  { -12, -12, -12}
};

const int chord_notes[15][6] = {
  {0, 0, 0, 0, 0, 0},
  {0, 1, 0, 0, 0, 0},
  {1, 1, 0, 0, 0, 0},
  {1, 1, 1, 0, 0, 0},
  {1, 1, 0, 1, 0, 0},
  {1, 1, 0, 0, 1, 0},
  {0, 1, 1, 1, 0, 0},
  {0, 1, 1, 0, 1, 0},
  {1, 0, 1, 0, 1, 0},
  {0, 0, 1, 1, 1, 0},
  {0, 1, 1, 0, 1, 0},
  {0, 0, 0, 1, 1, 1},
  {0, 1, 1, 0, 0, 1},
  {1, 1, 0, 0, 0, 1},
  {0, 1, 0, 1, 0, 1}
};

const float chromatic[88] = {55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192};

unsigned long cm;
unsigned long prev[8];

void setup() {
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff(); //otherwise it might send the root twice

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

  waveform1.begin(1, 220.0, WAVEFORM_SINE);
  waveform2.begin(1, 440.0, WAVEFORM_SINE);

  mixer1.gain(0, .5);
  mixer1.gain(1, .5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);


  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

}

void loop() {
  cm = millis();

  if (usbMIDI.read()) {
    byte type = usbMIDI.getType();
    switch (type) {
      case midi::NoteOn:
        note = usbMIDI.getData1();
        velocity = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        if (velocity > 0) {
          if (note_enable == 1) {
            note_enable = 0;
            note_in = note;
            note_state = 1; //theres a new note and it's on
          }
          //Serial.println(String("Note On:  ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        } else {
          note_state = -1; //note is now off
          //Serial.println(String("Note Off: ch=") + channel + ", note=" + note);
        }
        break;
      case midi::NoteOff:
        note = usbMIDI.getData1();
        velocity = usbMIDI.getData2();
        channel = usbMIDI.getChannel();
        note_state = -1;
        //note_in = note;
        // Serial.println(String("Note Off: ch=") + channel + ", note=" + note + ", velocity=" + velocity);
        break;
      default:
        d1 = usbMIDI.getData1();
        d2 = usbMIDI.getData2();
        //Serial.println(String("Message, type=") + type + ", data = " + d1 + " " + d2);
    }
  }

  //only do this when a new note is coming in and we have closed the previous notes
  // this is the easy way to avoid polypyony stuff and the complexity of keeping track of waht notes are on and off.
  // we'll impove it after you see if this fits your needs

  if (note_state == 1) {

    //random doesn't include the top numer so this is 0-2 as num_chord_types==3 BUT that's cool as the locations of the arracy are 0,1,2 ya know
    type_select = random(0, num_chord_types);
    inversion_sel = random(0, num_inversions_types);
    chord_select = random(0, num_chord_note_selections);

    Serial.print(type_select);
    Serial.print(" ");
    Serial.print(inversion_sel);
    Serial.print(" ");
    Serial.print(chord_select);
    Serial.println();


    note_state = 0; //and only do it once
    note_enable = 0;
    byte j = 0;
    Serial.print("j ");

    for (byte i = 0; i < 6; i++) {
      if (chord_notes[chord_select][i] == 1) {
        notes_out[j] = chord_type[type_select][i] + note_in;
        j++; //j will only be 0-2 max
        Serial.print(j);
        Serial.print(" ");
      }
    }
    Serial.println();
    Serial.print("notes ON: ");
    for (byte k = 0; k < 3; k++) {
      if (notes_out[k] > 0) { //we won't have 3 notes every time
        notes_out[k] += inversions[inversion_sel][k];
      }
      Serial.print(notes_out[k]);
      Serial.print(" ");
      usbMIDI.sendNoteOn(notes_out[k], 127, midi_out_channel);
    }
    Serial.println();
  }

  if (note_state == -1) { //now note is off
    note_state = 0; //only do this once
    Serial.print("notes OFF: ");
    for (byte k = 0; k < 3; k++) {
      Serial.print(notes_out[k]);
      Serial.print(" ");
      usbMIDI.sendNoteOff(notes_out[k], 0, midi_out_channel);
      note_enable = 1; //we've turned these notes off so allow a new root
      notes_out[k] = 0;// clean it up

    }
    Serial.println();
  }

  byte print_stats = 0;
  if (cm - prev[0] > 500 && print_stats == 1) {
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
