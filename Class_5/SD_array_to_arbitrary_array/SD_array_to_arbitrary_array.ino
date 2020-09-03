/*
  Read arditrary arry from sd
  press the button to load the sample
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformModulated waveformMod1;   //xy=355,370
AudioMixer4              mixer1;         //xy=539,376
AudioOutputI2S           i2s1;           //xy=684,401
AudioConnection          patchCord1(waveformMod1, 0, mixer1, 0);
AudioConnection          patchCord2(mixer1, 0, i2s1, 1);
AudioConnection          patchCord3(mixer1, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=531,544
// GUItool: end automatically generated code


#include <SPI.h>
#include <SD.h>
#define CS_PIN 10

File file;
char file_name[] = "B.TXT"; //name for the file

uint32_t  cu, cm, du, prev[8], SDloc, array_begin, array_len, array_jump;
int jump, interpolate;
short arb_array[257]; //short is jsut another type audio uses. Same as int really

void setup() {

  pinMode(8, INPUT_PULLUP);
  AudioMemory(10);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13);
  sgtl5000_1.volume(0.25);

  waveformMod1.begin(1, 220.0, WAVEFORM_ARBITRARY);
  waveformMod1.arbitraryWaveform(arb_array, 20000); //max freq hasn't been implemented so the second value is irrelivant
  mixer1.gain(0, 1);

  pinMode(CS_PIN, OUTPUT);
  SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
  SPI.setSCK(14);  // Audio shield has SCK on pin 14
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    delay(3000);
  }
  // Create or open the file.
  file = SD.open(file_name);
  if (!file) {
    Serial.println("open failed");
    delay(3000);
  }

  //if you want to laod the file with random data to test change this to 1
  byte test = 0;
  if (test == 1) {
    file.close();
    delay(10);
    file = SD.open(file_name, FILE_WRITE);

    for (int v; v < 20000; v++) {
      int newnum = random(-32000, 32000);
      SDaw(v, newnum);
    }
    Serial.print(file_name);
    Serial.println(" filled with random");

  }

}

int button1, prev_button1;

void loop() {
  cm = millis();
  prev_button1 = button1;
  button1 = digitalRead(8);
  //asuming there's alreay a pile of data on the card

  if (prev_button1 == 1 && button1 == 0) {

    //if you're not writing and reading to the card you don't need to open and close
    // if you are doing both you need to have this for each section.
    //file = SD.open(file_name, FILE_READ);

    array_begin = random(1000); //where to start in the SD array this is measures in the 10 cahracter blocs we used to write to it.
    array_len = 255; //how many readings to take. If we want a bigger range of readings we cahnge the jump
    array_jump = 2; //1 would be every reading, 5 would be every 5th reading
    int counter = 0;
    int lessthan = (array_len * array_jump) + array_begin;
    for (int j = array_begin; j < lessthan; j += array_jump) {
      arb_array[counter] = SDar(j);
      //arb_array[counter] = smooth(0, 8, SDar(j)); // you could do this but really smooth is a low pass filter so just that instead?
      // If you want to interpolate between readings that's a whole other thing and could be done with a follower and jsut a hand full of readings from the SD
      counter++;
    }
    //file.close(); //necessary
  }

  if (cm - prev[1] > 500) {
    prev[1] = cm;

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


//based on https://playground.arduino.cc/Main/DigitalSmooth/
// This function continuously samples an input and puts it in an array that is "samples" in length.
// This array has a new "raw_in" value added to it each time "smooth" is called and an old value is removed
// It throws out the top and bottom 15% of readings and averages the rest

#define maxarrays 3 //max number of different variables to smooth
#define maxsamples 99 //max number of points to sample and 
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
//SD array write(location, number to write);
void SDaw(uint32_t loc, int32_t val) {
  file.seek(loc * 10);
  char buf[10];
  String toCard = "";
  toCard = String(val);
  toCard.toCharArray(buf, 10);
  file.write(buf, 10);
}

//SD array read
// retuns the value from (location)

int32_t SDar(uint32_t loc) {
  file.seek(loc * 10);
  char rc[10];
  file.read(rc, 10);
  String inString = "";
  //Serial.print("read ");
  for (int i = 0; i < 10; i++) {
    inString += rc[i];
  }
  return inString.toInt();
}
