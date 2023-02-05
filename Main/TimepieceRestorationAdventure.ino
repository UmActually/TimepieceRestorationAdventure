/**
 * @file Timepiece Restoration Adventure.ino
 * Leonel Óscar Corona Villarreal
 * Leonardo Corona Garza
 */

#include <Stepper.h>

#define STEPS_PER_REVOLUTION 2048
#define RPM 12

// Grande: 75
// Chico: 18
// Pasos: 142.2 periódico

Stepper hours(STEPS_PER_REVOLUTION, 8, 10, 9, 11);
Stepper minutes(STEPS_PER_REVOLUTION, 4, 6, 5, 7);

int cycle = 0;
int hourCounter = 0;
int leapHourCounter = 0;
int leapMinuteCounter = 0;
int hourSteps = 0;
int minuteSteps = 0;

unsigned int totalHourSteps = 0;
unsigned int totalMinuteSteps = 0;

void setup() {
  Serial.begin(9600);
  hours.setSpeed(RPM);
  minutes.setSpeed(RPM);
}

void loop() {
  delay(59500);

  cycle++;
  hourCounter++;
  leapHourCounter++;
  leapMinuteCounter++;
  
  if (leapMinuteCounter == 3) {
    minuteSteps = -154;
    leapMinuteCounter = 0;
  } else {
    minuteSteps = -142;
  }

  int absMinuteSteps = minuteSteps * -1;
  totalMinuteSteps += absMinuteSteps;

  Serial.print("Cycle "); Serial.print(cycle);
  Serial.print(" | Stepping "); Serial.print(absMinuteSteps);
  Serial.print(" | Total "); Serial.println(totalMinuteSteps);

  minutes.step(minuteSteps);

  if (hourCounter == 30) {
    if (leapHourCounter == 3) {
      hourSteps = 726;
      leapHourCounter = 0;
    } else {
      hourSteps = 725;
    }

    totalHourSteps += hourSteps;

    Serial.print("----> Hour Cycle "); Serial.print(cycle);
    Serial.print(" | Stepping "); Serial.print(hourSteps);
    Serial.print(" | Total "); Serial.println(totalHourSteps);

    hours.step(hourSteps);
    hourCounter = 0;
  }
}
