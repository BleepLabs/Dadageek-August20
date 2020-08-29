/*
  In this file several examples are shown:
  Smoothing analog readings and converting them to exponetial curves
  Reading touch sensors
  Using envelope followers

*/

int  raw_reading[4];
int  smoothed_reading[4];
float follower[4] = {1, 1, 1, 1}; //can't be 0 since we can't multiply by it
float fall_rate = .99995; //very small changes in this number make a big difference
unsigned long current_time, prev[4];
int expo[4];

void setup() {
  analogReadResolution(12);
  analogReadAveraging(32); //this should still be used. it happed at the hardware level and is a different kind of averaging.
}

void loop() {
  current_time = millis();

  //smooth needs to happed fast so read and smooth in the bottom of the loop, not in a timing "if"
  raw_reading[0] = analogRead(A0);
  smoothed_reading[0] = smooth(0, 25, raw_reading[0]);

  // exponential conversion = (in^n) / (max value of in^(n-1))

  // a standard logarithmic curve would be to the power of 2 so:
  expo[0] = pow(smoothed_reading[0], 2) / 4095; //pow(base, exponent)
  // to get a steeper curve you could do:
  //expo[0] = pow(smoothed_reading[0], 3) / (pow(4095, 2));

  //raw_reading[1] = analogRead(A1);
  raw_reading[1] = touchRead(0) / 4; //touch read can return some large numbers so it might best to divide it before smoothing
  // see the notes here on making touch sensors https://github.com/BleepLabs/Dadageek-August20/wiki/Using-sensors
  smoothed_reading[1] = smooth(1, 47, raw_reading[1]);

  if (follower[1] < smoothed_reading[1]) {
    follower[1] = smoothed_reading[1]; //the follow jumps up to meet a new reading...
  }

  if (follower[1] > smoothed_reading[1]) {
    follower[1] *= fall_rate; //..but slowly fades after the reading has dropped
  }

  //printing alwasy needs to be in a timing if. Don't go faster than 5 milliseconds
  if (current_time - prev[0] > 40) {
    prev[0] = current_time;

    byte print_sel = 1; //which info to print?

    if (print_sel == 1) { //touch
      Serial.print(" raw:");
      Serial.print(raw_reading[1]);
      Serial.print(" sm:");
      Serial.print(smoothed_reading[1]);
      Serial.print(" fol:");
      Serial.print(follower[1]);
      Serial.println();
    }

    if (print_sel == 0) { //pot and expo
      Serial.print(" raw:");
      Serial.print(raw_reading[0]);
      Serial.print(" sm:");
      Serial.print(smoothed_reading[0]);
      Serial.print(" exp:");
      Serial.print(expo[0]);
      Serial.println();
    }
  }

} //end of loop

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
