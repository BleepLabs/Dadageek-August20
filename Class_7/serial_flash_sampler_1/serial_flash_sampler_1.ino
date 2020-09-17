/*
  "simple" serial flash sampler
  Uses the s25fl127 chip soldered to the bottom of the audio adapter board
  https://www.mouser.com/ProductDetail/Cypress-Semiconductor/S25FL127SABMFI101?qs=yatApJqb7mSoDiG6v501Uw%3D%3D

  The samples are stored in banks on the flash chip, all of the same calculated size. 
  See "sfblocks" below

  A bank must be erased before data can be put in it. This takes about 500 millis per block
  Once a sample is recorded it's length and starting location are stored in eeprom,
  a special type of memory in the Teensy that persists after the power is turned off
  On startup, the Teensy checks this memory and remembers the starts and lengths of all the samples
  If you change "sfblocks" or "number_of_banks" this data will make less sens and you'll play other sections of memory

  There is a slight buzzing sound while you're recording but it shouldn't be in the samples.
  Up to 3 or 4 samples can be played at once depending on the playback speed. 1.0 is regular speed, .5 is half, 2.0 is double. The faster the more taxing

  In this example one button is dedicated to erasing, one for recording and one playing.
  The bank they are all address is controlled by the pot.
  A second sampler is started but unused

  The samplers really just play back sections of memory. You can have multiple ones looking
   at the same part of memory ie the same sample

  Since "AudioSampler" is not in the regular library use "playSdWav" as it has the same outputs
  Then just "AudioPlaySdWav" with "AudioSampler" and "playSdWavX" with "samperX"

  queue_left and queue_right are AudioRecordQueue object and are what take the audio data
   and get it into the sampler. Don't change their names

  more sampler functions:
  sample_loop(0 or 1); Loop this sampler
  sample_reverse(0 or 1); play it backwards


*/

//include these before the rest
#include "sampler.h"
#include <EEPROM.h>
#include <SerialFlash.h>
#include <Bounce2.h>

////////////////////////////////////////

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSampler           sampler1;     //xy=81,443
AudioSampler           sampler0;     //xy=82,484
AudioInputI2S            line_in;           //xy=156,246
AudioMixer4              sampler_mixer_right;         //xy=289,514
AudioMixer4              sampler_mixer_left;         //xy=291,430
AudioAmplifier           amp_in_left;           //xy=338,216
AudioAmplifier           amp_in_right;           //xy=341,262
AudioRecordQueue         queue_left;         //xy=484,188
AudioRecordQueue         queue_right;         //xy=498,241
AudioMixer4              final_mixer_right;         //xy=505,470
AudioMixer4              final_mixer_left;         //xy=518,380
AudioOutputI2S           i2s2;           //xy=682,409
AudioConnection          patchCord1(sampler1, 0, sampler_mixer_left, 0);
AudioConnection          patchCord2(sampler1, 1, sampler_mixer_right, 0);
AudioConnection          patchCord3(sampler0, 0, sampler_mixer_left, 1);
AudioConnection          patchCord4(sampler0, 1, sampler_mixer_right, 1);
AudioConnection          patchCord5(line_in, 0, amp_in_left, 0);
AudioConnection          patchCord6(line_in, 1, amp_in_right, 0);
AudioConnection          patchCord7(sampler_mixer_right, 0, final_mixer_right, 0);
AudioConnection          patchCord8(sampler_mixer_left, 0, final_mixer_left, 0);
AudioConnection          patchCord9(amp_in_left, 0, final_mixer_left, 1);
AudioConnection          patchCord10(amp_in_left, queue_left);
AudioConnection          patchCord11(amp_in_right, 0, final_mixer_right, 1);
AudioConnection          patchCord12(amp_in_right, queue_right);
AudioConnection          patchCord13(final_mixer_right, 0, i2s2, 1);
AudioConnection          patchCord14(final_mixer_left, 0, i2s2, 0);
AudioControlSGTL5000     sgtl5000_1;
// GUItool: end automatically generated code

#define FlashChipSelect 6 //

//each block is 256kilobyte
//In 44.1kHz stereo that's just under 1.5 seconds
//on the s25fl127 there are 64 blocks (fyi these arent the 8bit block from the datasheet these are 4x that to account for stero 16b audio)
#define sfblocks 8 //about 12 seconds

// You could jsut put this equation there but you can't have a remainer 
#define number_of_banks 12 // 64 / sfblocks 

int16_t rec_target, foffset, mode_timer_latch, rec_mode, mode, prev_bank_sel;
uint32_t bankstart[number_of_banks];
uint32_t samplelen[number_of_banks];
byte bank_status[number_of_banks];
uint32_t sfaddress, address;
uint32_t cm, prev[12], mode_cm, mode_timer;
int bank_sel;
float freq[4];

//change the buttons to your pins
int button1_pin = 0;
int button2_pin = 1;
int button3_pin = 2;

//info on what bounce does here https://github.com/thomasfredericks/Bounce2#alternate-debounce-algorithms-for-advanced-users-and-specific-cases
//functions: https://github.com/thomasfredericks/Bounce2/wiki#methods
#define BOUNCE_LOCK_OUT //this tells it what mode to be in. I think it's the better one for music
Bounce button1 = Bounce(); //make a bounce object called button1
Bounce button2 = Bounce();
Bounce button3 = Bounce();

//this must be included at the end of declarations I know its annoying
#include "sampler_helpers.h"


void setup() {

  delay(200);
  //must be done for the flash chip
  SPI.setSCK(14);  // Audio shield has SCK on pin 14
  SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
  SPI.setCS(FlashChipSelect);

  delay(200);
  //uncomment these if you have other SPI chips connected
  //to keep them disabled while using only SerialFlash
  //pinMode(4, INPUT_PULLUP);
  //pinMode(10, INPUT_PULLUP);

  //sometimes you'll get this error even though the chip is attached just fine. Just reset the device if this happens
  //the delay() above is to try and mitigate this
  if (!SerialFlash.begin(FlashChipSelect)) {
    while (1) { //don't continue if the chip isn't there
      Serial.println("Unable to access SPI Flash chip");
      delay(1000);
    }
  }

  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  pinMode(button3_pin, INPUT_PULLUP);

  button1.attach(button1_pin); //what pin will it read
  button1.interval(10); //how many milliseconds of debounce time

  button2.attach(button2_pin); //what pin will it read
  button2.interval(10); //how many milliseconds of debounce time

  button3.attach(button3_pin); //what pin will it read
  button3.interval(10); //how many milliseconds of debounce time


  AudioMemory(15);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13); //The volume of the input. Again we'll get to this later
  sgtl5000_1.volume(0.25);

  load_sample_locations(); //must be done before other sampler stuff. it gets the bankstart[n] samplelen[n] from eeprom
  //amplitude,frequency
  sampler0.begin(1, 1.0);
  sampler1.begin(1, 1.0);

  amp_in_left.gain(1);//can be used to amplify the incoming signal before the sampler
  amp_in_right.gain(1);

  sampler_mixer_left.gain(0, 1);
  sampler_mixer_left.gain(1, 1);
  sampler_mixer_left.gain(2, 0);
  sampler_mixer_left.gain(3, 0);

  sampler_mixer_right.gain(0, 1);
  sampler_mixer_right.gain(1, 1);
  sampler_mixer_right.gain(2, 0);
  sampler_mixer_right.gain(3, 0);

  final_mixer_left.gain(0, 1);
  final_mixer_left.gain(1, 0);
  final_mixer_left.gain(2, 0);
  final_mixer_left.gain(3, 0);

  final_mixer_right.gain(0, 1);
  final_mixer_right.gain(1, 0);
  final_mixer_right.gain(2, 0);
  final_mixer_right.gain(3, 0);

  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

  Serial.println("Ready");

}

void loop() {
  cm = millis();

  button1.update(); //call these before doing anything with the buttons.
  button2.update();
  button3.update();

  if (button1.fell()) {
    erase_bank(bank_sel); //will print out the erase progress
  }

  if (button2.fell()) {
    startRecording(bank_sel);  //record must be done in these 3 steps. first start jsut once..
  }

  if (button2.read() == 0) {
    continueRecording(); ///...then continue while the button is down...
    final_mixer_left.gain(1, 1); //listen to the incoming audio
    final_mixer_right.gain(1, 1);
  }
  if (button2.rose()) {
    stopRecording();//...then stop
    final_mixer_left.gain(1, 0); //mute incoming audio
    final_mixer_right.gain(1, 0);
  }

  if (button3.fell()) {
    // only do this once like noteOn
    //start location in memory in bytes, length in bytes
    //you can modify these to get some granular type stuff going on.
    sampler0.sample_play_loc(bankstart[bank_sel], samplelen[bank_sel]);
  }

  if (button3.rose()) {
    //if you want the sample to only play when the button is down uncomment this
    //sampler0.sample_stop();
  }

  if (cm - prev[0] > 10) {
    prev[0] = cm;
    prev_bank_sel = bank_sel;
    //this will not give you a lot of the pot <1 ie playing back slower so the pot range "if" is used
    //float freq = (analogRead(A1) / 4095.0) * 4.0;

    int raw_freg_pot = analogRead(A1);
    if (raw_freg_pot <= 2048) { //if the pot is on the left side
      freq[0] = raw_freg_pot / 2048.0;
    }
    if (raw_freg_pot > 2048) { //if the pot is on the right side
      freq[0] = (((raw_freg_pot - 2048.0) / 2048.0) * 3.0) + 1.0; //0 - 4.0
    }


    //it's easier to just use map() but this shows how you have to keep everything a float if you wanna do accurate division
    //bank_sel = int((analogRead(A2) / 4095.0) * float(number_of_banks-1));
    bank_sel = map(analogRead(A2), 0, 4095, 0, number_of_banks - 1);
    sampler0.frequency(freq[0]);
    if (prev_bank_sel != bank_sel) {
      Serial.println(bank_sel);
    }
  }

  if (cm - prev[1] > 500) {
    prev[1] = cm;
    //Serial.println(freq[0]);
    
    byte print_stats = 0;
    if (print_stats == 1) {
      Serial.print("proc: ");
      Serial.print(AudioProcessorUsageMax());
      Serial.print("%    Mem: ");
      Serial.print(AudioMemoryUsageMax());
      Serial.println();
      AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
      AudioMemoryUsageMaxReset();
    }
  }

}
