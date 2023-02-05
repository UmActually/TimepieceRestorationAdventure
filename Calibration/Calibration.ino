#include <Stepper.h>

#define STEPS_PER_REVOLUTION 2048
#define RPM 12

Stepper hours(STEPS_PER_REVOLUTION, 8, 10, 9, 11);
Stepper minutes(STEPS_PER_REVOLUTION, 4, 6, 5, 7);

int leapHourCounter = 0;
int leapMinuteCounter = 0;
int hourSteps = 0;
int minuteSteps = 0;

void incrementHour() {
  leapHourCounter++;
  if (leapHourCounter == 3) {
    hourSteps = 726;
    leapHourCounter = 0;
  } else {
    hourSteps = 725;
  }
  hours.step(hourSteps);
  delay(1000);
}

void incrementMinute() {
  leapMinuteCounter++;
  if (leapMinuteCounter == 3) {
    minuteSteps = -154;
    leapMinuteCounter = 0;
  } else {
    minuteSteps = -142;
  }
  minutes.step(minuteSteps);
  delay(1000);
}

void setup() {
  hours.setSpeed(RPM);
  minutes.setSpeed(RPM);
}

void loop() {
  // incrementHour();
  incrementMinute();
}
