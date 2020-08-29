/*
  Using envelopes and the delay effect
*/


#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated waveformMod1;   //xy=177,232
AudioSynthWaveformModulated waveformMod2;   //xy=183,279
AudioEffectEnvelope      envelope1;      //xy=417,229
AudioEffectEnvelope      envelope2;      //xy=422,278
AudioEffectDelay         delay1;         //xy=437,597
AudioMixer4              mixer1;         //xy=450,354
AudioMixer4              mixer2;         //xy=457,456
AudioMixer4              mixer3;         //xy=646.0000038146973,423.99997901916504
AudioMixer4              mixer4;         //xy=647.0056762695312,321.0056743621826
AudioOutputI2S           i2s1;           //xy=650.9999809265137,224.99995517730713
AudioConnection          patchCord1(waveformMod1, envelope1);
AudioConnection          patchCord2(waveformMod2, envelope2);
AudioConnection          patchCord3(envelope1, 0, mixer1, 0);
AudioConnection          patchCord4(envelope2, 0, mixer1, 1);
AudioConnection          patchCord5(delay1, 0, mixer2, 1);
AudioConnection          patchCord6(mixer1, 0, mixer2, 0);
AudioConnection          patchCord7(mixer1, 0, mixer3, 0);
AudioConnection          patchCord8(mixer2, delay1);
AudioConnection          patchCord9(mixer2, 0, mixer3, 1);
AudioConnection          patchCord10(mixer3, 0, mixer4, 0);
AudioConnection          patchCord11(mixer4, 0, i2s1, 0);
AudioConnection          patchCord12(mixer4, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=211,429
// GUItool: end automatically generated code




//Then we have our variable declarations like before
unsigned long cm;
unsigned long prev[8];
#define num_of_buttons 2
int button_pin[num_of_buttons] = {10, 12}; //lets put the button pins in an array
int button_read[num_of_buttons];
int prev_button_read[num_of_buttons];
float wet_level, dry_level, fb_level, delay_time, vol;
int  smoothed_reading[4];

void setup() {

  pinMode(button_pin[0], INPUT_PULLUP);
  pinMode(button_pin[1], INPUT_PULLUP);

  //How much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 3.2 we can go up to about 200 but that won't leave any RAM for anyone else.
  // It's usually the delay and reverb that hog it.
  AudioMemory(100);

  //Start the delay effect
  // delay(output channel, milliseconds of delay time)
  // every 10 milliseconds needs 3 blocks of memory in AudioMemory
  delay1.delay(0, 250); //needs 84 blocks on its own.

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13); //The volume of the input. Again we'll get to this later

  //Output volume. Goes from 0.0 to 1.0 but a fully loud signal will clip over .8 or so.
  // For headphones it's pretty loud at .4
  // There are lots of places we can change the final volume level. This one you set once and leave alone.
  sgtl5000_1.volume(0.25);


  //This next group can be done anywhere in the code but we want to start things with these
  // values and change some of them in the loop.

  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  waveformMod1.begin(1, 220.0, WAVEFORM_SINE);
  waveformMod2.begin(1, 440.0, WAVEFORM_SINE);


  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)

  mixer1.gain(0, .5); //combine both oscillators
  mixer1.gain(1, .5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  //delay mixer
  mixer2.gain(0, .75); //input
  mixer2.gain(1, 0); //feedback
  mixer2.gain(2, 0);
  mixer2.gain(3, 0);

  //wet / dry  mixer
  mixer3.gain(0, .5); //dry
  mixer3.gain(1, .5); //wet
  mixer3.gain(2, 0);
  mixer3.gain(3, 0);
  //final output level
  mixer4.gain(0, 0);
  mixer4.gain(1, 0);
  mixer4.gain(2, 0);
  mixer4.gain(3, 0);


  //Then we do the stuff we've done before.
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

  envelope1.attack(1);
  envelope1.decay(50);
  envelope1.sustain(.2);
  envelope1.release(500); //how long will it take for the notes to fade out in milliseconds

  envelope2.attack(1000);
  envelope2.release(100);

}

void loop() {
  cm = millis();

  //This one "for" updates both the buttons.
  // If we had more buttons all we'd need to do is change j < 2 and the array sizes
  // it's best to do button reads in the loop outside of a timing "if"
  for (int j = 0; j < 2; j++) {
    prev_button_read[j] = button_read[j];
    button_read[j] = digitalRead(button_pin[j]);
  }

  //we could do some fancy this to simplify this and include it in the "for" above
  // but lets keep it simple for now
  if (prev_button_read[0] == 1 &&  button_read[0] == 0) {
    envelope1.noteOn(); //the note on is a single event. Need to only do it one, not continuously
  }
  if (prev_button_read[0] == 0 &&  button_read[0] == 1) {
    envelope1.noteOff(); //same for note off
  }

  if (prev_button_read[1] == 1 &&  button_read[1] == 0) {
    envelope2.noteOn();
  }
  if (prev_button_read[1] == 0 &&  button_read[1] == 1) {
    envelope2.noteOff();
  }


  //reading and changing values should usually be slowed down a little
  if (cm - prev[0] > 5) {
    prev[0] = cm;

    wet_level = (analogRead(A0) / 4095.0); //0.0 - 1.0
    dry_level = (wet_level - 1.0) * -1.0; //1.0-0.0
    mixer3.gain(0, dry_level);
    mixer3.gain(1, wet_level);

    fb_level = (analogRead(A1) / 4095.0) * 1.2;
    mixer2.gain(1, fb_level);

    delay_time = analogRead(A2);
    smoothed_reading[0] = ((smooth(0, 49, delay_time)) / 4095.0) * 240.0;
    delay1.delay(0, smoothed_reading[0]);

    vol = (analogRead(A3) / 4095.0);
    mixer4.gain(0, vol);

  }


  if (cm - prev[1] > 500) {
    prev[1] = cm;
    Serial.println(delay_time);
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();

  }
}


//based on https://playground.arduino.cc/Main/DigitalSmooth/
// This function continuously samples an input and puts it in an array that is "samples" in length.
// This array has a new "raw_in" value added to it each time "smooth" is called and an old value is removed
// It throws out the top and bottom 15% of readings and averages the rest

#define maxarrays 1 //max number of different variables to smooth
#define maxsamples 51 //max number of points to sample and 
//reduce these numbers to save RAM

unsigned int smoothArray[maxarrays][maxsamples];

// sel should be a unique number for each occurrence
// samples should be an odd number greater that 7. It's the length of the array. The larger the more smooth but less responsive
// raw_in is the input. positive numbers in and out only.

unsigned int smooth(byte sel, unsigned int samples, unsigned int raw_in) {
  int j, k, temp, top, bottom;
  long total;
  static int i[maxarrays];
  static int sorted[maxarrays][maxsamples];
  boolean done;

  i[sel] = (i[sel] + 1) % samples;    // increment counter and roll over if necessary. -  % (modulo operator) rolls over variable
  smoothArray[sel][i[sel]] = raw_in;                 // input new data into the oldest slot

  for (j = 0; j < samples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[sel][j] = smoothArray[sel][j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (samples - 1); j++) {
      if (sorted[sel][j] > sorted[sel][j + 1]) {    // numbers are out of order - swap
        temp = sorted[sel][j + 1];
        sorted[sel] [j + 1] =  sorted[sel][j] ;
        sorted[sel] [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((samples * 15)  / 100), 1);
  top = min((((samples * 85) / 100) + 1  ), (samples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[sel][j];  // total remaining indices
    k++;
  }
  return total / k;    // divide by number of samples
}
