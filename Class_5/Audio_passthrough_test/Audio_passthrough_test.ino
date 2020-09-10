// this jsut takes the incoming signal and directly send it to the output so you can test your hardware
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=251.00000381469727,180.00000190734863
AudioOutputI2S           i2s2;           //xy=432,176
AudioConnection          patchCord1(i2s1, 0, i2s2, 0);
AudioConnection          patchCord2(i2s1, 1, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=340,122
// GUItool: end automatically generated code

unsigned long cm;
unsigned long prev[8];

void setup() {
  AudioMemory(10);

  sgtl5000_1.enable(); //Turn the adapter board on
  
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN);
  //The volume of the input. 0 is low, 15 is high
  sgtl5000_1.lineInLevel(12); //5 is default for line level
  
  //If you want to use a microphone instead use these line. You can't use both at once
  //sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  //sgtl5000_1.micGain(20); //number is from 0-63 dB of gain. Usually this gets set pretty high
  

  sgtl5000_1.volume(0.25);

  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64); 

}

void loop() {
  cm = millis();

 
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
