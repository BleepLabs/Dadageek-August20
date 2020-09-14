#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum1;          //xy=101.00569152832031,188.00568389892578
AudioSynthSimpleDrum     drum2;          //xy=115.00569152832031,234.00568389892578
AudioMixer4              mixer1;         //xy=318,217
AudioOutputI2S           i2s1;           //xy=526,216
AudioConnection          patchCord1(drum1, 0, mixer1, 0);
AudioConnection          patchCord2(drum2, 0, mixer1, 1);
AudioConnection          patchCord3(mixer1, 0, i2s1, 0);
AudioConnection          patchCord4(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=419,327
// GUItool: end automatically generated code

IntervalTimer timer1;

unsigned long cm;
unsigned long prev[8];
unsigned long interupt_start_time, interupt_len;
byte tick, trig1, trig2, prev_trig1, new_note, led_trig;
uint32_t led_cm;
uint32_t seq_accumulator, seq_increment, seq_index, timer_rate, timer_tune, prev_seq_index;
float seq_rate, seq_bpm;

//sequence settings
#define steps_in_seq 128   //must be a power of 2  if we want odd time signatures we can do that in other ways
#define num_steps_factor  7   //2^num_steps_factor=steps_in_seq  doing it manually isnt hard and the equantion is crazy so yeah
#define steps_per_quarter_note 16 //how many divisions make up a 1/4 

int measures_in_sequence = steps_in_seq / (steps_per_quarter_note * 4); //calculted jsut fyi. Not using this yet

byte sequence1[steps_in_seq] = { //put a simple beat in one to start
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 1, 0, 0, 0
};

byte sequence2[steps_in_seq]; //empty sequence


#include <Bounce2.h>
#define BOUNCE_LOCK_OUT //the method for debouncing. Basically jsut ignores other inpunts after the first chage for the duration of debounce time

#define button1_pin 8
#define button2_pin 10
#define led_pin 5

//(button pin, milliseconds of debounce time)
Bounce record_button = Bounce(button1_pin, 4);
Bounce trig_button = Bounce(button2_pin, 4);

void setup() {

  AudioNoInterrupts();//turn audio off while we set things up. Not necessary but can remove some pops

  pinMode(led_pin, OUTPUT);
  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  
  AudioMemory(10);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary yet but good to have
  sgtl5000_1.lineInLevel(13); //The volume of the input. Again we'll get to this later

  //Output volume. Goes from 0.0 to 1.0 but a fully loud signal will clip over .8 or so.
  // For headphones it's pretty loud at .4
  // There are lots of places we can change the final volume level. This one you set once and leave alone.
  sgtl5000_1.volume(0.25);

  mixer1.gain(0, .5);
  mixer1.gain(1, .5);
  mixer1.gain(2, 0);
  mixer1.gain(3, 0);

  analogWriteResolution(12);
  analogReadResolution(12);
  analogReadAveraging(64);


  //setting up the interupt timer. All of this is stays the same.
  // the BPM of the sequecne is controlled in the loop
  timer_rate = 100; //time in microseconds. 100HZ = 10,000 microsecodns
  timer_tune = (1.00 / (timer_rate * .000001)); //So we can descrive the seq rate in Hz accurately
  timer1.priority(1);
  timer1.begin(sequence_timer, timer_rate);

  drum1.frequency(80);
  drum1.length(25);
  drum2.frequency(220);
  drum2.length(100);

  AudioInterrupts(); //turn audio back on
}


void sequence_timer() {
  //to keep the sequeance timing solid we need to very briefly turn off the audio stuff.
  //this is fine as long as interupt_length is < 12 or so
  interupt_start_time = micros();
  AudioNoInterrupts();
  //this is pretty much how the "triangle_osc" code from the early part of the calss worked but going much more slowly
  seq_increment = (4294967296.00 * (seq_rate)) / (timer_tune);
  seq_accumulator += seq_increment;
  prev_seq_index = seq_index;
  seq_index = ((seq_accumulator) >> (32 - num_steps_factor));

  //we don't want to keep triggering the sound while the sequence is in a step that contains a "1"
  // so we only trigger it while new note is "1"
  new_note = 0;
  if (prev_seq_index != seq_index) {
    new_note = 1;
    if (seq_index == 0) {
      led_trig = 1; //blink the LED when the measure starts
      led_cm = cm;
      digitalWrite(led_pin, 1);
    }
  }
  
  //step through a sequence for each drum sound
  trig1 = sequence1[seq_index];
  trig2 = sequence2[seq_index];
  if (new_note == 1) {
    if (trig1 == 1) {
      drum1.noteOn();
    }
    if (trig2 == 1) {
      drum2.noteOn();
    }
  }
//read the buttons with bounce
  record_button.update(); 
  trig_button.update();

  if (trig_button.fell()) {
    drum2.noteOn();
    // if the record button is down while you trigger the sound it is recorded to the sequence
    if (record_button.read() == 0) {
      sequence2[seq_index] = 1;
    }
  }

  AudioInterrupts(); //turn audio back on
  interupt_len = micros() - interupt_start_time; //how long did this all take in microseconds. Need to keep it well under 12 or so
}


void loop() {
  cm = millis();

  if (cm - prev[1] > 10) {
    prev[1] = cm;
    //the pot is used to control bpm so we move it to a resonable range
    int raw_pot = (((analogRead(A1) / 4095.00) * 200.0) + 60.0); 
    seq_bpm = raw_pot;
    
    //the rate that is passed on to the timing code is based on what we delacred for the 
    // sequence total steps and divisions per 1/4 note which also determine how many measures are in the se
    seq_rate = ((seq_bpm * ((float)steps_per_quarter_note / (float)steps_in_seq)) / 60.0);

    if (led_trig == 1) {
      if (cm - led_cm > 15) { //keep the led on for 15 millisconds when triggerd by the interupt timer
        led_trig = 0;
        digitalWrite(led_pin, 0);
      }
    }
  }

  if (cm - prev[0] > 500) {
    prev[0] = cm;

    Serial.print("interupt_len ");
    Serial.print(interupt_len);
    Serial.print("  seq_bpm ");
    Serial.print(seq_bpm);
    Serial.print("  processor ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%  memory ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();

    AudioProcessorUsageMaxReset(); //We need to reset these valuse so we get a real idea of wh the audio code is doing rather than jsut peaking in every half a second
    AudioMemoryUsageMaxReset();

  }
}
