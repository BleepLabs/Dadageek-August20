//https://www.arduino.cc/reference/en

//The first thing we do is initialize variables.
//Here we name the variables we want to store data in and decide on the data type

int led1_pin =  10;  // the number of the pin that the LED is attached to. 
                     //"int" describes a data type which allows the varible to be a whole number from -32,767 to 32,767 (16 bits)
int led1_state = 0;  // remembers what we want the state of the LED to be. Well start with 0 aka off or LOW
unsigned long previous_time = 0;   // will store last time the LED was updated. "unsigned long" is another data type that's 32 bytes instead of 16/
unsigned long current_time;         // stores the current number of milliseconds since the device was reset. We havent set it equal to anything so it will defualt to 0
int interval1 = 1000;        ///the interval at which LED1 will change in milliseconds. We'll start with 500 = .5 seconds

//End of initialization

void setup() //This part of the code runs just once after the teensy resets.
{ //The setup starts here....
  pinMode(led1_pin, OUTPUT); // set the LED pin as output.

} //..and ends here.

void loop() //This part of the code will run over and over again as fast as the Teensy can go.
{
  current_time = millis();
  // Call the function "millis()".
  // It returns the time in milliseconds that have ellapsed since the device restarted which we store in the varible "current_time"
  // That means for the rest or this loop "current_time" will be equal to that number of milliseconds

  if (current_time - previous_time > interval1) {
    //has "interval1" amount of time past since we last executed this code contained in the curly brackets?
    //If the current time minus the last time is bigger that the interval then this part of the code is run
    //If not then it dosen't and we do the next thing in the code

    previous_time = current_time;  //remember what time it is now so we can check how long it's been next loop

    // if the LED is off turn it on and vice versa:
    if (led1_state == LOW) { // == means check if these are te same
      led1_state = HIGH; // = mean set the first varible equal to the second thing
    }
    else { //if the previous comarion is false do this
      led1_state = LOW;
    }

    digitalWrite(led1_pin, led1_state);
    // Ouptut the value "led1_state" to "led1_pin.
    //if "led1_state" is low aka "0" the LED will be off
    //if "led1_state" is high aka "1" the LED will be on
    //it's digital so it can only be on or off, 1 or 0, high or low.

  } // the "if" statement is now over.


} // Loop is over so it starts again with no wait.
