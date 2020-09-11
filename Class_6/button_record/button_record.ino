//recording a button and pot


#include "smooth.h" //smooth is located in a tab at the top instead of in this file. We just have to include it
// if the file to be included is in the same directory as the main .ino, use " ". Above we see <> this is for files in other directories.


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=113,166.00000190734863
AudioEffectEnvelope      envelope1;      //xy=303.00000381469727,167.00000190734863
AudioMixer4              mixer1;         //xy=457.00000762939453,183.00000190734863
AudioAmplifier           amp1;           //xy=619.0000076293945,232.00000286102295
AudioOutputI2S           i2s1;           //xy=659.0000076293945,171.00000190734863
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(envelope1, 0, mixer1, 0);
AudioConnection          patchCord3(mixer1, amp1);
AudioConnection          patchCord4(amp1, 0, i2s1, 0);
AudioConnection          patchCord5(amp1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=454,286
// GUItool: end automatically generated code


// these could be define or variables no big difference
int button1_pin = 0;
int button2_pin = 1;
int button3_pin = 2;

//info on what bounce does here https://github.com/thomasfredericks/Bounce2#alternate-debounce-algorithms-for-advanced-users-and-specific-cases
//fuctions https://github.com/thomasfredericks/Bounce2/wiki#methods
#include <Bounce2.h> //use bounce2, not bounce
#define BOUNCE_LOCK_OUT //this tells it what mode to be in. I think it's the better one for music
Bounce button1 = Bounce(); //make a bounce object called button1
Bounce button2 = Bounce(); //make a bounce object called button2
Bounce button3 = Bounce();




// this does need to be a define as we'll use it up here in the declaration section
#define array_length 32
byte button_rec_array[array_length]; //byte to save space
int pot_rec_array[array_length];

unsigned long cm;
unsigned long prev[8];
int button_rec, prev_button_reading, pot_rec, array_loc, button_reading, recording;
float vol;

void setup() {

  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  pinMode(button3_pin, INPUT_PULLUP);

  button1.attach(button1_pin); //what pin will it read
  button1.interval(10); //how many milliseconds of debounce time

  button2.attach(button2_pin); //what pin will it read
  button2.interval(10); //how many milliseconds of debounce time

  button3.attach(button3_pin); //what pin will it read
  button3.interval(10); //how many milliseconds of debounce time

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
  waveform1.begin(1, 220.0, WAVEFORM_SINE);

  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  // A gain of 1 means the output is the same as the input.
  // .5 would be half volume and 2 would be double
  // Since we have two oscillators coming in that are already "1" We should take them down by half so we don't clip.
  // If you go over "1" The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, 1);
  mixer1.gain(1, 0);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  amp1.gain(1);//final volume

  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

  randomSeed(analogRead(A9)); //if we don't do this our "random" called later on will always provide the same results;

  //fill the array with data
  for (int i; i < array_length; i++) {
    pot_rec_array[i] = (i * random(20, 75)) + 200; //use i to increment the potion and change the data put in the array
  }

  envelope1.noteOn(); //start with the envelope on
  envelope1.release(20);

}

void loop() {
  cm = millis();

  vol = (smooth(1, 31, analogRead(A2))) / 4095.00;
  amp1.gain(vol);//final volume

  button1.update(); //call these before doing anything with the buttons.
  button2.update();
  button3.update();


  if (button1.read() == 0) { //is button 1 being pressed?
    pot_rec = smooth(0, 31, analogRead(A1) / 4.0);
    pot_rec_array[array_loc] = pot_rec;
  }

  if (button2.read() == 0) {
    button_rec_array[array_loc] = button3.read();
  }

  if (button3.fell()) { //new way of saying was it high now it's low
    envelope1.noteOn();
  }

  if (button3.rose()) { //was it low now its high
    envelope1.noteOff();
  }

  if (cm - prev[1] > 100) {
    prev[1] = cm;
    array_loc++;
    if (array_loc >= array_length) {
      array_loc = 0;
    }
    //still need to compare previous values so we know when to do note on and off
    prev_button_reading = button_reading;
    button_reading = button_rec_array[array_loc];

    if (prev_button_reading == 0 && button_reading == 1) {
      envelope1.noteOff();
      analogWrite(5, 0); //(pin, value 0-4095)
    }
    if (prev_button_reading == 1 && button_reading == 0) {
      envelope1.noteOn();
      analogWrite(5, 3000);//(pin, value 0-4095)

    }
    waveform1.frequency(pot_rec_array[array_loc]);
    analogWrite(6, pot_rec_array[array_loc] * 4);
    byte printer = 1; //change this to anything else to turn off printing
    if (printer == 1) {
      Serial.print("step: ");
      Serial.print(array_loc);
      Serial.print("  button:");
      Serial.print(button_reading);
      Serial.print("  pot:");
      Serial.println(pot_rec_array[array_loc]);
    }
  }


  if (cm - prev[0] > 500 && 0 == 1) { //turn this on by changing it to 1==1
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
