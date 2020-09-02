/*
  Treat an SD card liek a big ol' array
  Instead of it storing ints or bytes it jsut stores numbers that are a max of 9 digits long
  so it can be -999999999 to 999999999
  It's kinda hacky and wasteful but it's an sd card with GIGS of space it's fine
*/

#include <SPI.h>
#include <SD.h>
#define CS_PIN 10

File file;
char file_name[] = "B.TXT"; //name for the file
uint32_t  cu, cm, du, prev[8], SDloc=1;

void setup() {
  Serial.begin(9600);
  pinMode(CS_PIN, OUTPUT);
  SPI.setMOSI(7);  // Audio shield has MOSI on pin 7
  SPI.setSCK(14);  // Audio shield has SCK on pin 14
  // Initialize the SD.
  if (!SD.begin(CS_PIN)) {
    Serial.println("begin failed");
    delay(3000);
  }
  // Create or open the file.
  //file = SD.open(file_name, FILE_WRITE);
  file = SD.open(file_name, FILE_READ);
  if (!file) {
    Serial.println("open failed");
    delay(3000);
  }

  file.seek(0);
  // Write dummy data to create the file
  file.write(111111111111111111111111111111111111111);
  file.close();
}

void loop() {
  cm = millis();

  if (cm - prev[0] > 50) {
    prev[0] = cm;
    SDloc++; //increment the pace we're writing too
    int newnum = random(-10000, 10000); //make a new number
    cu = micros(); // remeber what time it is in microseconds

    //This is all you really need to know
    // just treat it liks an array except the numbers can be 9 digits long,
    // rather than being an int or long.
    //SDaw(SDloc, newnum); //SD array write(location to put it at, number)
    int val = SDar(SDloc); //val will be equat to the number at that location


    du = micros() - cu; //subtrace the previous micros reading from this one to determine how ling the SD read write took
    Serial.print("read:");  Serial.println(val);
    if (newnum != val) {
      Serial.println("!! match error !!"); //if the input doesn't equal the reading
    }
    Serial.print("mircos take: ");  Serial.println(du); //on my very cheap SD it's usally 75 micros but some times its almost 8000;
    Serial.println();
  }
}

//SD array write(location, number to write);
void SDaw(uint32_t loc, int32_t val) {
  file = SD.open(file_name, FILE_WRITE);
  file.seek(loc * 10);
  static char buf[10];
  String toCard = "";
  if (val < 0) {
    buf[9] = "-";
  }
  toCard = String(val);
  toCard.toCharArray(buf, 10);
  file.write(buf, 10);
    file.close();
}

//SD array read
// retuns the value from (location)
int32_t SDar(uint32_t loc) {
   file = SD.open(file_name, FILE_READ);
  file.seek(loc * 10);
  char rc[10];
  file.read(rc, 10);
  String inString = "";
  //Serial.print("read ");
  for (int i = 0; i < 10; i++) {
    inString += rc[i];
  }
   file.close();
  return inString.toInt();

}
