//Important libraries for Accelerometer
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

// Defining pins for accelerometer
#define LIS3DH_CLK A4 //SCL
#define LIS3DH_MISO 8 //SDO
#define LIS3DH_MOSI A3 //SDA
#define LIS3DH_CS 9 //CS

Adafruit_LIS3DH lis = Adafruit_LIS3DH(LIS3DH_CS, LIS3DH_MOSI, LIS3DH_MISO, LIS3DH_CLK);

#if defined(ARDUINO_ARCH_SAMD)
   #define Serial SerialUSB
#endif

#include <Servo.h>

unsigned long time; // for high decimal precision timing

Servo myservo;

const int buttonPin = 5; // pushbutton pin number
int buttonState = 0; // initialize button to not be pressed

float new_value = 0;
float old_value = 10;

int unlocktime = 120; // length of time (in seconds) for the egg to unlock

int counter = 0;
int error = 0;

int locked = 110; // servo locked position (110deg)
int unlocked = 90; // servo unlocked position (90deg)

void setup(void) {
#ifndef ESP8266
  while (!Serial);
#endif

  Serial.begin(9600); // Start Serial monitor
  Serial.println("LIS3DH test!");
  
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldn't start");
    while (1);
  }
  Serial.println("LIS3DH found!");
  
  lis.setRange(LIS3DH_RANGE_4_G);
  
  Serial.print("Range = "); Serial.print(2 << lis.getRange());  
  Serial.println("G");
  
  myservo.attach(7); //setting servo pin

  pinMode(buttonPin, INPUT);

  myservo.write(locked); //set servo to locked position
}

void loop() {

  buttonState = digitalRead(buttonPin);

  lis.read();

  sensors_event_t event; 
  lis.getEvent(&event);
  
  new_value = (event.acceleration.x + event.acceleration.y + event.acceleration.z)/3; // new_value is the average of xyz accelerations
  
  Serial.println(counter);
  Serial.println(old_value);
  Serial.println(new_value);
  
 
  //if the change is not signficant, assume there was no movement
  if (abs(abs(new_value) - abs(old_value)) <= 0.3){
      error = 0; //reset the glitch counter
  }

  //if the change is greater than 0.3, assume it was a glitch, unless it happened three times, then it must have actually moved
  else {
    error++;
    if(error == 3){
      counter = 0; //reset time
      error = 0; //reset the glitch counter
    }
  }
 
  counter++; // add one to the clock

  // if the time for non-movement has elapsed, move servo to unlock the egg
  if (counter == unlocktime){
     myservo.write (unlocked); //set servo to unlocked position
     counter = 0;  
  }

  // press the button to relock the egg; 10 seconds to close the egg before the servo moves
  // relocking starts by moving the servo to unlocked position to in case the user doesn't relock in time on the first try
  if (buttonState == HIGH) {
    Serial.println("Button was pressed");
    Serial.println("Locking in 10 seconds.");

    myservo.write (unlocked); //set servo to unlocked position
    for(i=1; i<=10: i++){
      Serial.println(i);
      delay(1000);
    }

    Serial.println("Egg is locked.");
    
    myservo.write (locked); //set servo to locked position
    
    counter = 0; //reset time
    error = 0; //reset the glitch counter
  }

  Serial.println();
  
  old_value = new_value; //reassign the old_value to be what was the new_value for a later comparison
  
  delay(1000); //wait 1 second before redoing the loop
}
