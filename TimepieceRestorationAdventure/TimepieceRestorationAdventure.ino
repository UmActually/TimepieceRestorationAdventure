/**
 * @file Timepiece Restoration Adventure.ino
 * Leonel Óscar Corona Villarreal
 * Leonardo Corona Garza
 */

#include <Stepper.h>
#include <virtuabotixRTC.h>               //Include library for clock module

#define STEPS_PER_REVOLUTION 2048
#define RPM 12

#define DISPLAYED_HOURS 18
#define DISPLAYED_MINUTES 0

// Grande: 75
// Chico: 18
// Pasos: 142.2 periódico

Stepper hours(STEPS_PER_REVOLUTION, 8, 10, 9, 11);
Stepper minutes(STEPS_PER_REVOLUTION, 4, 6, 5, 7);

virtuabotixRTC clock(3, 12, 13);    //Create a clock object attached to pins 3, 12, 13 - CLK, DAT, RST

int currentMinute = 0;
int cycle = 0;
int leapHourCounter = 0;
int leapMinuteCounter = 0;

unsigned int totalHourSteps = 0;
unsigned int totalMinuteSteps = 0;

int bumpHalfHour(int amount = 1) {
  int hourSteps;
  leapHourCounter++;
  if (leapHourCounter == 3) {
    hourSteps = 726;
    leapHourCounter = 0;
  } else {
    hourSteps = 725;
  }
  long resp = hourSteps * amount;
  Serial.print("Hour Steps "); Serial.println(resp);
  hours.step(resp);
  return resp;
}

int bumpMinute(int amount = 1) {
  int minuteSteps;
  leapMinuteCounter++;
  if (leapMinuteCounter == 3) {
    minuteSteps = -152;
    leapMinuteCounter = 0;
  } else {
    minuteSteps = -151;
  }
  Serial.print("Minute Steps "); Serial.println(minuteSteps);
  long resp = minuteSteps * amount;
  minutes.step(resp);
  return resp;
}

void testRTC(){
  clock.updateTime(); //Update the time
  Serial.print(clock.hours);
  Serial.print(":");
  Serial.print(clock.minutes);
  Serial.print(":");
  Serial.print(clock.seconds);
  Serial.print(" ");
  Serial.println(clock.dayofweek);
}

void initialAdjust(){
  clock.updateTime();
  currentMinute = clock.minutes;
  int hourDiff = (clock.hours - DISPLAYED_HOURS);
  int minuteDiff = (clock.minutes - DISPLAYED_MINUTES);

  int halfHourDiff = hourDiff * 2;

  if (minuteDiff != 0) {
    if (minuteDiff < 0) {
      minuteDiff += 60;
    }
    Serial.print("Minutes Diff: ");
    Serial.println(minuteDiff);
    bumpMinute(minuteDiff);

    // Ajustar cuál de las dos tarjetas de hora debe salir
    // según el minuto
    if (DISPLAYED_MINUTES < 30 && clock.minutes >= 30) {
      halfHourDiff++;
    } else if (DISPLAYED_MINUTES >= 30 && clock.minutes < 30) {
      halfHourDiff--;
    }
  }

  if (halfHourDiff != 0){
    if (halfHourDiff < 0){
      halfHourDiff += 48;
    }
    Serial.print("Half Hours Diff: ");
    Serial.println(halfHourDiff);
    bumpHalfHour(halfHourDiff);
  }
}

// void setup() {
//   hours.setSpeed(RPM);
//   minutes.setSpeed(RPM);
//   bumpHalfHour(47);
// }

// void loop() {

// }

void setup() {
  Serial.begin(9600);
  hours.setSpeed(RPM);
  minutes.setSpeed(RPM);

  //setup
  //initialAdjust();
}

void loop() {
  delay(1000);

  clock.updateTime();
  if (currentMinute == clock.minutes) {
    return;
  }

  testRTC();
  currentMinute = clock.minutes;
  cycle++;

  int minuteSteps = bumpMinute();
  int absMinuteSteps = minuteSteps * -1;
  totalMinuteSteps += absMinuteSteps;
  Serial.print("Cycle "); Serial.print(cycle);
  Serial.print(" | Stepping "); Serial.print(absMinuteSteps);
  Serial.print(" | Total "); Serial.println(totalMinuteSteps);

  if (currentMinute == 0 || currentMinute == 30) {
    int hourSteps = bumpHalfHour();
    totalHourSteps += hourSteps;
    Serial.print("----> Hour Cycle "); Serial.print(cycle);
    Serial.print(" | Stepping "); Serial.print(hourSteps);
    Serial.print(" | Total "); Serial.println(totalHourSteps);
  }
}