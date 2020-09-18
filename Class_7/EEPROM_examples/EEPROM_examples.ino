/*
  Saving and laoding from eeprom
  The Teensy 3.2 has 2048 bytes of EEPROM
  This type of memory retained when the device is powered off unlike RAM
  Each byte can "only" be written to 100k times. Great for most things but don't try to make a tiny audio delay with it
*/

#include <EEPROM.h> //gotta include the library

void setup() {

  while (!Serial) { // setpu won't continue untill you open the serial monitor
  }

  Serial.println("~ Fun with EEPROM! ~");
  Serial.println();

  byte save = 0;

  if (save == 1) {
    Serial.println("Saving");
    //It's very easy to store bytes in EEPROM
    byte sb = 210;
    //write(location from 0-2047,data 0-255 )
    EEPROM.write(0, sb);
    Serial.print("saved byte: ");
    Serial.println(sb);

    //EEPROM is only bytes so is we want to store larger numbers
    // we need to chop them up to bytes to save and rebuild them to read
    //slot and number to save
    long s1 = 156987435;
    save_long_to_ee(1, s1); //second bank as we alreay wrote to first byte
    Serial.print("saved long: ");
    Serial.println(s1);
    //but what about floats?
    // automatic converison to bytes is really tricky but you can easily do it manually
    // say you have 12.34567
    // we care about 6 digits so lets multiply it by 100000 to save and divide by 100000 to read
    // You'll either have to remeber on your own the factor ot usr eeprom to do it
    float f1 = 12.34567;
    int factor = 100000;
    int save_float = f1 * factor;
    save_long_to_ee(2, save_float);
    save_long_to_ee(3, factor);
    Serial.print("saved float: ");
    Serial.println(f1, 6); //print 6 digits

  }
  if (save == 0) {
    Serial.println("Loading");
    byte read_byte = EEPROM.read(0);
    Serial.print(" read byte: ");
    Serial.println(read_byte);
    //read a slot back
    long read1 = read_long_from_ee(1);
    Serial.print(" read long: ");
    Serial.println(read1);
    //read a float back
    float read2 = read_long_from_ee(2);//mave them floats so they divide propperly
    float read3 = read_long_from_ee(3);
    float f_read = read2 / read3;
    Serial.print(" read float: ");
    Serial.println(f_read, 6); //print 6 digits

  }
}

void loop() {
  // put your main code here, to run repeatedly:

}




//each slot is 4 bytes so we have 512 slots
void save_long_to_ee(uint16_t slot, int32_t data) {
  int16_t eestart = slot * 4;
  EEPROM.write(eestart, data >> 24);
  EEPROM.write(eestart + 1, data >> 16);
  EEPROM.write(eestart + 2, data >> 8);
  EEPROM.write(eestart + 3, data & 0xFF);
}

uint32_t read_long_from_ee(uint16_t slot) {
  int16_t eestart = slot * 4;
  byte b4 = EEPROM.read(eestart);
  byte b3 = EEPROM.read(eestart + 1);
  byte b2 = EEPROM.read(eestart + 2);
  byte b1 = EEPROM.read(eestart + 3);
  int32_t lt = (b4 << 24) | (b3 << 16) | (b2 << 8) | (b1 );
  return lt;
}
