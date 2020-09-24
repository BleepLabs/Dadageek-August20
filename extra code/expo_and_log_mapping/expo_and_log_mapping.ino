//logarithmic and exponential scaling with "fscale"

int j;
float scaledResult, curve_adjust;
unsigned long cm, prev[4];

void setup() {
  analogWriteResolution(12); //PWM and A14 DAC output will be 0-4095. This has no effect on the 16 bit in/out of the audio adapter
  analogReadResolution(12); //AnalogReads will return 0-4095
  analogReadAveraging(64);

}

void loop() {
  cm = millis();


  if (cm - prev[0] > 5) {
    prev[0] = cm;
    j++;
    if (j > 100) {
      j = 0;
    }
    //fscale(input, input low, input high, output low, output high, curve)
    //curve responds to -1.0 to 1.0
    //negative curve gives exponential output, positive logarithmic
    //curve = 0 is linear
    curve_adjust = fscale(analogRead(A0), 0, 4095, -1.0, 1.0, 0); //pot is now -1.0 to 1.0
    scaledResult = fscale( j, 0, 100, 0, 100, curve_adjust); //j steps through 0 to 100

    //view in serial plotter to easily see the slope change
    Serial.print("curve_adjust"); //no pace here means it just prints the value at the top of the plotter instead of plotting it
    Serial.print(curve_adjust);
    Serial.print(" "); // a space means seperate these valuse to get thier own lines on the graph
    Serial.print(j);
    Serial.print(" ");
    Serial.println(scaledResult);

  }
}


//floating point scaling
// works like map but allows for float results and applying a log or expo curve
// If curve is 0, it returns a linear scaling
// less than 0 and it's logarithmic
// greater than 0 and it's exponential

// based on
///https://playground.arduino.cc/Main/Fscale/
// there are more efficient ways of doing this but it works just fine for getting pots to other ranges.

float fscale(float inputValue,  float originalMin, float originalMax, float newBegin, float
             newEnd, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  if (curve > 1) curve = 1;
  if (curve < -1) curve = -1;

  float curve_amount = 1.0; // increase this number to get steeper curves
  curve = (curve * curve_amount * -1.0) ; // - invert and scale - this seems more intuitive - positive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into logarithmic exponent for other pow function


  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero reference the values
  OriginalRange = originalMax - originalMin;

  if (newEnd > newBegin) {
    NewRange = newEnd - newBegin;
  }
  else
  {
    NewRange = newBegin - newEnd;
    invFlag = 1;
  }

  zeroRefCurVal = inputValue - originalMin;
  normalizedCurVal  =  zeroRefCurVal / OriginalRange;   // normalize to 0 - 1 float

  // Check for originalMin > originalMax  - the math for all other cases i.e. negative numbers seems to work out fine
  if (originalMin > originalMax ) {
    return 0;
  }

  if (invFlag == 0) {
    rangedValue =  (pow(normalizedCurVal, curve) * NewRange) + newBegin;

  }
  else     // invert the ranges
  {
    rangedValue =  newBegin - (pow(normalizedCurVal, curve) * NewRange);
  }

  return rangedValue;
}
