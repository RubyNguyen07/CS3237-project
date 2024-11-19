#include "driver/rtc_io.h"

#define ACTION_A_TRIGGER_PIN 21   // Pin to check if Action A is triggered by ESP32-1
#define ACTION_A_COMPLETE_PIN 26  // Pin to signal completion of Action A to ESP32-1
#define ACTION_B_RESPONSE_PIN 25  // Pin to receive Action B completion signal from ESP32-1
#define ACTION_C_COMPLETE_PIN 32  // Pin to signal completion of Action C to ESP32-1

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 8           // Time ESP32-2 will sleep (in seconds)

// Ultrasonic Sensor Pins & Declarations
#define TRIG_SENSOR_1 16                            // Trigger pin for sensor 1
#define ECHO_SENSOR_1 17                            // Echo pin for sensor 1
#define TRIG_SENSOR_2 23                            // Trigger pin for sensor 2
#define ECHO_SENSOR_2 22                            // Echo pin for sensor 2
const float SPEED_OF_SOUND_CM_PER_US = 0.0343 / 2;  // Speed of sound in cm/us (round trip)
const float Distance_Threshold = 20.0;              // Distance threshold in cm
const long Timeout_Duration = 20000;                // Timeout duration in microseconds

// DC Motors Movement Pins & Declarations
// Define GPIO pins for the L293D (Left Side) and relay
#define leftEnable 4     // Enable for both Left Motors
#define leftForward 18   // Combined forward direction control for Left Motors
#define leftBackward 19  // Combined backward direction control for Left Motors

// Define GPIO pins for the L293D (Right Side) and relay
#define rightEnable 13       // Enable for both Right Motors
#define rightForward 14      // Combined forward direction control for Right Motors
#define rightBackward 27     // Combined backward direction control for Right Motors
const int motorSpeed = 150;  // PWM speed for slow movement

// Line Tracking Sensor & Declarations
#define IR_SENSOR_1 35  //Front Left
#define IR_SENSOR_2 34  //Front Right
#define IR_SENSOR_3 39  //Back Left
#define IR_SENSOR_4 36  //Back Right
volatile int sensor1_value = 0;
volatile int sensor2_value = 0;
volatile int sensor3_value = 0;
volatile int sensor4_value = 0;
const int IR_threshold = 3000;

// Universal Declarations
RTC_DATA_ATTR int state = 0;  // State to toggle between forward and backward
// state = 0: In the Forward State
// state = 1: In the Recording State
// state = 2: In the Backward State

void setup() {
  Serial.begin(115200);
  delay(2000);

  // Setting Up Motor Pins
  pinMode(leftEnable, OUTPUT);
  pinMode(leftForward, OUTPUT);
  pinMode(leftBackward, OUTPUT);

  pinMode(rightEnable, OUTPUT);
  pinMode(rightForward, OUTPUT);
  pinMode(rightBackward, OUTPUT);

  // Setting Up Ultrasonic Sensor Pins
  pinMode(TRIG_SENSOR_1, OUTPUT);
  pinMode(ECHO_SENSOR_1, INPUT);
  pinMode(TRIG_SENSOR_2, OUTPUT);
  pinMode(ECHO_SENSOR_2, INPUT);
  digitalWrite(TRIG_SENSOR_1, LOW);
  digitalWrite(TRIG_SENSOR_2, LOW);

  // Set IR sensor pins as input
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(IR_SENSOR_3, INPUT);
  pinMode(IR_SENSOR_4, INPUT);

  // Set up pins
  pinMode(ACTION_A_TRIGGER_PIN, INPUT);
  pinMode(ACTION_A_COMPLETE_PIN, OUTPUT);
  pinMode(ACTION_B_RESPONSE_PIN, INPUT);
  pinMode(ACTION_C_COMPLETE_PIN, OUTPUT);

  // Initialize output pins
  digitalWrite(ACTION_A_COMPLETE_PIN, LOW);
  digitalWrite(ACTION_C_COMPLETE_PIN, LOW);  // Initially set to LOW for Action C signal

  Serial.println("Pinout Set...");

  int state = 0;
  delay(2000);

  // Print wakeup reason for debugging
  print_wakeup_reason();

  // Check if Action A is triggered by ESP32-1
  if (digitalRead(ACTION_A_TRIGGER_PIN) == HIGH) {
    // Action A
    Serial.println("Action A triggered by ESP32-1.");
    delay(1000);
    Serial.println("Performing Action A...");
    while (state == 0) {
      float distance1 = ultrasonic_distance(1, TRIG_SENSOR_1, ECHO_SENSOR_1);  // Check Ultrasonic Sensor 1
      IRsensors(IR_SENSOR_1, IR_SENSOR_2);                                     // Read IR sensors 1 and 2
      if (distance1 <= Distance_Threshold) {
        if ((sensor1_value > IR_threshold) && (sensor2_value > IR_threshold)) {
          stopMotors();
          Serial.println("End of Forward State - Switching to Recording State");
          state = 1;
          delay(1000);
          break;
        } else {
          stopMotors();
          Serial.println("Obstacle Detected - Waiting to Resume");
          while (distance1 <= Distance_Threshold) {
            delay(100);
            distance1 = ultrasonic_distance(1, TRIG_SENSOR_1, ECHO_SENSOR_1);
          }
          Serial.println("Obstacle Cleared - Resuming");
        }
      } else {
        if ((sensor1_value > IR_threshold) && (sensor2_value < IR_threshold)) {
          turnLeft();
          Serial.println("Adjusting: Turn Left");
        } else if ((sensor1_value < IR_threshold) && (sensor2_value > IR_threshold)) {
          turnRight();
          Serial.println("Adjusting: Turn Right");
        } else {
          moveForward();
          Serial.println("Moving Forward");
        }
      }
    }
    digitalWrite(ACTION_A_COMPLETE_PIN, HIGH);
    delay(1000);

    // Perform Action B
    while (state == 1) {
      if (digitalRead(ACTION_B_RESPONSE_PIN) == LOW) {
        Serial.println("Waiting...");
        delay(1000);
      }
      else {
        Serial.println("Received Action B completion signal from ESP32-1. Starting Action C...");
        state = 2;
        delay(1000);
      }  
    }

    // Perform Action C
    Serial.println("Performing Action C...");
    delay(1000);
    while (state == 2) {
      float distance2 = ultrasonic_distance(2, TRIG_SENSOR_2, ECHO_SENSOR_2);  // Check Ultrasonic Sensor 1
      IRsensors(IR_SENSOR_3, IR_SENSOR_4);                                     // Read IR sensors 1 and 2
      if (distance2 <= Distance_Threshold) {
        if ((sensor3_value > IR_threshold) && (sensor4_value > IR_threshold)) {
          stopMotors();
          Serial.println("End of Program");
          state = 0;
          delay(1000);
          break;
        } else {
          stopMotors();
          Serial.println("Obstacle Detected - Waiting to Resume");
          while (distance2 <= Distance_Threshold) {
            delay(100);
            distance2 = ultrasonic_distance(2, TRIG_SENSOR_2, ECHO_SENSOR_2);
          }
          Serial.println("Obstacle Cleared - Resuming");
        }
      } else {
        if ((sensor4_value > IR_threshold) && (sensor3_value < IR_threshold)) {
          turnLeft();
          Serial.println("Adjusting: Turn Left");
        } else if ((sensor4_value < IR_threshold) && (sensor3_value > IR_threshold)) {
          turnRight();
          Serial.println("Adjusting: Turn Right");
        } else {
          moveBackward();
          Serial.println("Moving Forward");
        }
      }
    }
    digitalWrite(ACTION_C_COMPLETE_PIN, HIGH);
    Serial.println("Action C complete.");
    delay(1000);
  }

    // Configure deep sleep timer for periodic wake-up
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("ESP32-2 configured to wake up every " + String(TIME_TO_SLEEP) + " seconds.");
  Serial.println("Going to sleep now.");
  Serial.flush();

  // Enter deep sleep mode
  esp_deep_sleep_start();
}


void loop() {
  // This will not be called due to deep sleep
}

// Function to print wakeup reason
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}

/////////////////////////////////////////////////////////////////
///////////////////// Function Declarations /////////////////////
/////////////////////////////////////////////////////////////////

// IRSensors Function: To Obtain Analog IR Readings
void IRsensors(int sensor_a, int sensor_b) {
  // Check if the provided pins match the defined IR sensor pins
  if ((sensor_a == IR_SENSOR_1) && (sensor_b == IR_SENSOR_2)) {
    // Read analog values from the IR sensors connected to the given pins
    sensor1_value = analogRead(IR_SENSOR_1);
    sensor2_value = analogRead(IR_SENSOR_2);

    // Print the sensor values and their GPIO pins to the Serial Monitor
    Serial.print("Left IR Sensor (1): ");
    Serial.print(sensor1_value);
    Serial.print("\t Right IR Sensor (2): ");
    Serial.println(sensor2_value);
  } else if ((sensor_a == IR_SENSOR_3) && (sensor_b == IR_SENSOR_4)) {
    sensor3_value = analogRead(IR_SENSOR_3);
    sensor4_value = analogRead(IR_SENSOR_4);

    // Print the sensor values and their GPIO pins to the Serial Monitor
    Serial.print("Left IR Sensor (3): ");
    Serial.print(sensor3_value);
    Serial.print("\t Right IR Sensor (4): ");
    Serial.println(sensor4_value);
  } else {
    // Exit the function if the pins do not match expected pairs
    return;
  }
}


// Function to measure distance with an ultrasonic sensor
float ultrasonic_distance(int sensorNumber, int trigPin, int echoPin) {
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure echo duration with a timeout
  long duration = pulseIn(echoPin, HIGH, Timeout_Duration);
  float distance = 255;  // Default error value

  if (duration > 0) {
    distance = duration * SPEED_OF_SOUND_CM_PER_US;  // Convert duration to distance
  }

  // Print distance or error
  Serial.print("Distance Sensor ");
  Serial.print(sensorNumber);
  Serial.print(": ");
  if (distance != 255) {
    Serial.print(distance);
  } else {
    Serial.print("Error (255)");
  }
  Serial.println(" cm");

  return distance;
}

void moveForward() {
  digitalWrite(leftForward, HIGH);
  digitalWrite(leftBackward, LOW);
  analogWrite(leftEnable, motorSpeed);

  digitalWrite(rightForward, HIGH);
  digitalWrite(rightBackward, LOW);
  analogWrite(rightEnable, motorSpeed);

  delay(200);
  stopMotors();
}

void moveBackward() {
  digitalWrite(leftForward, LOW);
  digitalWrite(leftBackward, HIGH);
  analogWrite(leftEnable, motorSpeed);

  digitalWrite(rightForward, LOW);
  digitalWrite(rightBackward, HIGH);
  analogWrite(rightEnable, motorSpeed);

  delay(200);
  stopMotors();
}

void stopMotors() {
  digitalWrite(leftForward, LOW);
  digitalWrite(leftBackward, LOW);
  analogWrite(leftEnable, 0);

  digitalWrite(rightForward, LOW);
  digitalWrite(rightBackward, LOW);
  analogWrite(rightEnable, 0);
}

void turnRight() {
  digitalWrite(leftForward, HIGH);
  digitalWrite(leftBackward, LOW);
  analogWrite(leftEnable, motorSpeed);

  digitalWrite(rightForward, LOW);
  digitalWrite(rightBackward, HIGH);
  analogWrite(rightEnable, motorSpeed);

  delay(300);
  stopMotors();
}

void turnLeft() {
  digitalWrite(leftForward, LOW);
  digitalWrite(leftBackward, HIGH);
  analogWrite(leftEnable, motorSpeed);

  digitalWrite(rightForward, HIGH);
  digitalWrite(rightBackward, LOW);
  analogWrite(rightEnable, motorSpeed);

  delay(300);
  stopMotors();
}    