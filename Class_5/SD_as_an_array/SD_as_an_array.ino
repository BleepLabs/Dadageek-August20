/*
  Treat an SD card liek a big ol' array
  Instead of it storing ints or bytes it just stores numbers that are a max of 9 digits long
  so it can be -999999999 to 999999999
  It's kinda hacky and wasteful but it's an sd card with GIGS of space it's fine
*/

#include <SPI.h>
#include <SD.h>
#define CS_PIN 10

File file;
char file_name[] = "B.TXT"; //name for the file
uint32_t  cu, cm, du, prev[8], SDloc = 1;

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
  file = SD.open(file_name, FILE_WRITE);
  if (!file) {
    Serial.println("open failed");
    delay(3000);
  }
  file.close();
}

void loop() {
  cm = millis();

  if (cm - prev[0] > 50) {
    prev[0] = cm;
    SDloc++; //increment the pace we're writing too
    int newnum = random(-32000, 32000); //make a new number
    cu = micros(); // remember what time it is in microseconds

    //This is all you really need to know
    // just treat it like an array except the numbers can be 9 digits long,
    // rather than being an int or long.
    file = SD.open(file_name, FILE_WRITE); //first you have to open your file
    SDaw(SDloc, newnum); //SD array write(location to put it at, number)
    int val = SDar(SDloc); //val will be equal to the number at that location
    file.close(); //close it when you're done

    du = micros() - cu; //subtract the previous micros reading from this one to determine how ling the SD read write took
    Serial.print("read:");  Serial.println(val);
    if (newnum != val) {
      Serial.print("!! match error !! "); //if the input doesn't equal the reading
      Serial.println(newnum);
    }
    Serial.print("mircos take: ");  Serial.println(du);
    Serial.println();
  }
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
// returns the value from (location)
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
