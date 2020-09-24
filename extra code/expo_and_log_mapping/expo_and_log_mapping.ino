//log and expo scaling with

int j;
float scaledResult, curve_adjust;
unsigned long cm, prev[4];

void setup() {
  Serial.begin(9600);
}

void loop() {
  cm = millis();


  if (cm - prev[0] > 5) {
    prev[0] = cm;
    j++;
    if (j > 200) {
      j = 0;
    }
    //fscale(input,input low,input high,output low,output high, curve)
    //curve respons to -1.0 to 1.0
    //necative curve gives exponential output, positve logarhytmic
    //curve of 0 is linear
    curve_adjust = fscale(analogRead(A0), 0, 1023, -1.0, 1.0, 0);
    scaledResult = fscale( j, 0, 200, 0, 200, curve_adjust);

    //view in serail polotter to easily see the slope change
    Serial.print(curve_adjust);
    Serial.print("  ");
    Serial.print(j);
    Serial.print("  ");
    Serial.println(scaledResult);

  }
}


// floating point scaling
// woreks like map but allows for float resutls and applying a log or expo curve
//if curve is 0, it retuns a linear scaling
//less tah 0 and it's logarhyminc
//greater than 0 and it's exopnetial
///https://playground.arduino.cc/Main/Fscale/
// there are more effeint ways of doing this but it works just fine for getting pots to other ranges.


float fscale(float inputValue,  float originalMin, float originalMax, float newBegin, float
             newEnd, float curve) {

  float OriginalRange = 0;
  float NewRange = 0;
  float zeroRefCurVal = 0;
  float normalizedCurVal = 0;
  float rangedValue = 0;
  boolean invFlag = 0;


  // condition curve parameter
  // limit range

  if (curve > 1) curve = 1;
  if (curve < -1) curve = -1;

  float curve_amount = 1.0; // increase this number to get steeper curves 
  curve = (curve * curve_amount * -1.0) ; // - invert and scale - this seems more intuitive - postive numbers give more weight to high end on output
  curve = pow(10, curve); // convert linear scale into lograthimic exponent for other pow function


  // Check for out of range inputValues
  if (inputValue < originalMin) {
    inputValue = originalMin;
  }
  if (inputValue > originalMax) {
    inputValue = originalMax;
  }

  // Zero Refference the values
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
