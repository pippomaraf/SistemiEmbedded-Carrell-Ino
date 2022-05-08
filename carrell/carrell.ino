/**
 * @file carrell.ino
 * @author Filippo Maraffio
 * @brief Automatic, line-following electric cart
 * @version 0.1
 */

#include "HX711.h"
HX711 loadcell;

//-----------------------------
//      GPIO DEFINITIONS
//-----------------------------

// HC-SR04 ultrasonic sensor
#define ULTRASONIC_TRIG 13
#define ULTRASONIC_ECHO 12

// KY-033 line detecting sensor
// note: due to digital pin shortage, analog pins are used
// to perform digitalReads;
#define LINE_FOLLOWER_RR_SIGNAL A5
#define LINE_FOLLOWER_R_SIGNAL A4
#define LINE_FOLLOWER_L_SIGNAL A3
#define LINE_FOLLOWER_LL_SIGNAL A2

// L298N DC motor driver
// note: PWM capable pin needed
// note: the motors only need to move in one direction,
// IN pins are hard-coded (IN1 = IN3 = HIGH, IN2 = IN4 = LOW)
#define MOTOR_DRIVER_ENA 6
#define MOTOR_DRIVER_ENB 5

// HX711 load cell module
#define LOAD_CELL_CK 3
#define LOAD_CELL_DATA 2

// led
#define LED_PIN 11

// button
#define BUTTON_PIN 4

// buzzer
#define BUZZER_PIN 10

//-----------------------------
//      CONSTANTS
//-----------------------------

// uncomment to calibrate the scale
// #define SCALE_CALIBRATION

// load cell calibration, calculated with the calibrateScale() function
#define LOADCELL_DIVIDER 497.5

// found by trial and error
#define MINIMUM_SPEED 120 // PWM value, out of 255
#define NORMAL_SPEED 130
#define TUNRING_SPEED 20

// note: measure errors are present. Consider the minimum weight
// to be 10g above this treshold
#define MINIMUM_WEIGHT_TRESHOLD 40

// how many readings of the loadcell to do
#define LOADCELL_READS 10

// number of loadcell reds under the treshold before
// the carrell.ino will start moving
#define CHECKS_BEFORE_START 5

// for stability
#define STABILITY_DELAY 50

//-----------------------------
//      FUNCTIONS
//-----------------------------

/* L298N Motor driver
  Existing libraries are way too complex for what we'll need, so we'll just
  define a couple of functions.

  Our project only needs to move forward, so pins 1,2,3 and 4 are hard-coded
*/
void moveForward(int speed)
{
  analogWrite(MOTOR_DRIVER_ENA, speed);
  analogWrite(MOTOR_DRIVER_ENB, speed);
}

void steer(int speedLeft, int speedRight)
{
  analogWrite(MOTOR_DRIVER_ENA, speedLeft);
  analogWrite(MOTOR_DRIVER_ENB, speedRight);
}

void stop()
{
  moveForward(0);
}

/* Tunes
 */
void arrivalTune()
{
  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  tone(BUZZER_PIN, 494, 700);
  delay(80);

  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  tone(BUZZER_PIN, 659, 600);
  delay(80);

  digitalWrite(LED_PIN, HIGH);
  delay(50);
  digitalWrite(LED_PIN, LOW);
  tone(BUZZER_PIN, 740, 600);
  delay(80);

  digitalWrite(LED_PIN, HIGH);
  tone(BUZZER_PIN, 988, 850);
  delay(850);
  digitalWrite(LED_PIN, LOW);
}

void startTune()
{
  for (int i = 0; i <= 3; i++)
  {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1319, 200);
    delay(200);
    tone(BUZZER_PIN, 1397, 200);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    tone(BUZZER_PIN, 2637, 200);
    delay(200);
    tone(BUZZER_PIN, 2794, 200);
    delay(200);
  }
}

/** Scale
 * to initialize the scale, uncomment the SCALE_CALIBRATION definition and set
 * LOADCELL_DIVIDER to 1.
 */
void calibrateScale()
{
  // calibration start
  bool stopped;
  Serial.println("Begin calibration. Please remove any weight on the scale.");
  Serial.println("Send anything from serial monitor when you are ready");
  stopped = true;
  // clear serial
  while (Serial.available())
  {
    char foo = Serial.read();
  }
  while (stopped)
  {
    if (Serial.available() > 0)
    {
      stopped = false;
    }
  }
  loadcell.tare();
  Serial.println("Put a known weight on the scale, then send anything from the serial monitor");
  stopped = true;
  // clear serial
  while (Serial.available())
  {
    char foo = Serial.read();
  }
  while (stopped)
  {
    if (Serial.available() > 0)
    {
      stopped = false;
    }
  }
  Serial.print("Weight: ");
  Serial.println(loadcell.get_units(10), 2);
  Serial.println("Divide this for the known weight and set LOADCELL_DIVIDER accordingly");
  Serial.println("Calibration ended");
}



/** LINE FOLLOWER
 * @brief moves forward at the given speed. Carrell.ino will try it's best to
 * follow a dark line on a light surface, using its infrared sensors. To 
 * function properly this function should be cycled for the time that you 
 * intend the carrell.ino to follow the line. Keep in mind that the infrared 
 * sensor are checked in order every cycle, the busyer the cycle you put this
 * function in the slower the carrell.ino will respond!
 */
void followLine(int speed)
{
  // when R sensor is on the line, for the time that it reads the line
  // gently steer right
  if (digitalRead(LINE_FOLLOWER_R_SIGNAL))
  {
    while (digitalRead(LINE_FOLLOWER_R_SIGNAL))
    {
      steer(speed, TUNRING_SPEED);
      // if it gets stuck, break out of the cycle
      if (digitalRead(LINE_FOLLOWER_L_SIGNAL))
        break;
      // if it's very off, steer hard until the left sensor is on the line
      if (digitalRead(LINE_FOLLOWER_RR_SIGNAL))
      {
        while (!digitalRead(LINE_FOLLOWER_L_SIGNAL))
        {
          steer(MINIMUM_SPEED, 0);
        }
      }
    }
  }
  // when L sensor is on the line, for the time that it reads the line
  // gently steer left
  else if (digitalRead(LINE_FOLLOWER_L_SIGNAL))
  {
    while (digitalRead(LINE_FOLLOWER_L_SIGNAL))
    {
      steer(TUNRING_SPEED, speed);
      // if it gets stuck, break out of the cycle
      if (digitalRead(LINE_FOLLOWER_R_SIGNAL))
        break;
      // if it's very off, steer hard until the right sensor is on the line
      if (digitalRead(LINE_FOLLOWER_LL_SIGNAL))
      {
        while (!digitalRead(LINE_FOLLOWER_R_SIGNAL))
        {
          steer(0, MINIMUM_SPEED);
        }
      }
    }
  }
  // if it's very off, steer hard until it's straight again
  else if (digitalRead(LINE_FOLLOWER_RR_SIGNAL))
  {
    while (!digitalRead(LINE_FOLLOWER_L_SIGNAL))
    {
      steer(MINIMUM_SPEED, 0);
    }
  }
  else if (digitalRead(LINE_FOLLOWER_LL_SIGNAL))
  {
    while (!digitalRead(LINE_FOLLOWER_R_SIGNAL))
    {
      steer(0, MINIMUM_SPEED);
    }
  }
  // otherwise, go straight
  else if (!digitalRead(LINE_FOLLOWER_R_SIGNAL) && !digitalRead(LINE_FOLLOWER_L_SIGNAL))
    moveForward(speed);
}

//-----------------------------
//      STATES
//-----------------------------
/**
 * @brief the carrell.ino is stopped. Once a load of at least
 * 50 grams is put on the
 * scale, the start sequence will be played
 */
void loadingState()
{
  long weight = 0L;
  int times = CHECKS_BEFORE_START;

  while (times >= 0)
  {
    Serial.println(times);
    Serial.print("Weight: ");
    weight = loadcell.get_units(LOADCELL_READS);
    Serial.println(weight);
    if (weight >= MINIMUM_WEIGHT_TRESHOLD)
    {
      times--;
    }
    else
    {
      times = CHECKS_BEFORE_START;
    }
  }
  startTune();
}

/**
 * @brief carrell.ino starts moving. It will follow a black line on the floor,
 * and it will stop when a  wall is near in front of it.
 */
void wayThereState()
{
  // start moving forward
  // moveForward(NORMAL_SPEED);
  Serial.println("moving");
  while (true)
  {
    followLine(NORMAL_SPEED);
    // check for the end wall, if near stop
  }

}

void unloadingState() {}

void wayBackState() {}

void finishState() {}

//-----------------------------
//      LOOP AND SETUP
//-----------------------------

void setup()
{
  // pin modes
  pinMode(ULTRASONIC_TRIG, OUTPUT);
  pinMode(ULTRASONIC_ECHO, INPUT);
  pinMode(LINE_FOLLOWER_RR_SIGNAL, INPUT);
  pinMode(LINE_FOLLOWER_R_SIGNAL, INPUT);
  pinMode(LINE_FOLLOWER_L_SIGNAL, INPUT);
  pinMode(LINE_FOLLOWER_LL_SIGNAL, INPUT);
  pinMode(MOTOR_DRIVER_ENA, OUTPUT);
  pinMode(MOTOR_DRIVER_ENB, OUTPUT);
  loadcell.begin(LOAD_CELL_DATA, LOAD_CELL_CK);
  loadcell.set_scale(LOADCELL_DIVIDER);
  loadcell.tare();
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // serial monitor (for developement purposes)
  Serial.begin(9600);

// scale calibration
#ifdef SCALE_CALIBRATION
  calibrateScale();
#endif
}

void loop()
{
  // LoadingState(); // --- uncomment
  wayThereState();
  unloadingState();
  wayBackState();
  finishState();
}