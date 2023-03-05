/**
 * @file Timepiece Restoration Adventure.ino
 * Leonel Óscar Corona Villarreal
 * Leonardo Corona Garza
 */

#include <Stepper.h>
#include <virtuabotixRTC.h> //Include library for clock module

#define STEPS_PER_REVOLUTION 2048
#define RPM 12

#define M_SENSOR_PIN A0
#define H_SENSOR_PIN A1

// Grande: 75
// Chico: 18
// Pasos: 142.2 periódico

// UNO
//Stepper hours(STEPS_PER_REVOLUTION, 8, 10, 9, 11);
//Stepper minutes(STEPS_PER_REVOLUTION, 4, 6, 5, 7);
//virtuabotixRTC clock(3, 12, 13);  //Create a clock object attached to pins 3, 12, 13 - CLK, DAT, RST

// NANO
Stepper minutes(STEPS_PER_REVOLUTION, 5, 3, 4, 2);  // Pins nano pcb in1,2,3,4 / 5,4,3,2
Stepper hours(STEPS_PER_REVOLUTION, 9, 7, 8, 6);  // Pins nano pcb in1,2,3,4 / 9,8,7,6
virtuabotixRTC clock(12, 11, 10);  // NANO Create a clock object attached to pins 12, 11, 10 - CLK, DAT, RST

// seconds, minutes, hours, day of the week, day of the month, month, year
// clock.setDS1302Time(00, 00, 18, 1, 05, 2, 2023);  // Only required once to reset the clock time

int currentMinute = 0;
int minuteHomeCounter = 0;
unsigned int leapMinuteCounter = 0;
long totalMinuteSteps = 0;

bool isMinuteAtHome = false;
bool ignoreBump = false;

enum class HM {
  hour, minute
};

void powerStepper(HM type, bool enable) {
  int rangeStart, rangeEnd;

  if (type == HM::hour) {
    rangeStart = 6;
    rangeEnd = 9;
  } else {
    rangeStart = 2;
    rangeEnd = 5;
  }

  for (int pin = rangeStart; pin <= rangeEnd; pin++) {
    pinMode(pin, enable ? OUTPUT : INPUT);
  }
}

bool bumpHalfHour(int amount = 1) {
  powerStepper(HM::hour, true); // Start the motor  

  for (int i = 0; i < amount; i++) {
    hours.step(725);
  }

  powerStepper(HM::hour, false); // Stop the motor to avoid overheat
  return !digitalRead(H_SENSOR_PIN);
}

bool bumpMinute(int amount = 1) {
  powerStepper(HM::minute, true); // Start the motor

  long minuteSteps = 0;

  for (int i = 0; i < amount; i++) {
    leapMinuteCounter++;
    if (leapMinuteCounter == 60) {
      leapMinuteCounter = 0;
    }

    if (leapMinuteCounter == 30) {
      minuteSteps -= 151;
    } else if (leapMinuteCounter % 10 == 0) {
      minuteSteps -= 152;
    } else {
      minuteSteps -= 153;
    }
  }

  long absMinuteSteps = abs(minuteSteps);
  totalMinuteSteps += absMinuteSteps;

  Serial.print(F("leapMinuteCounter ")); Serial.print(leapMinuteCounter);
  Serial.print(F(" Minute Steps ")); Serial.print(absMinuteSteps);
  Serial.print(F(" Total Minute Steps ")); Serial.println(totalMinuteSteps);

  minutes.step(minuteSteps);

  powerStepper(HM::minute, false); // Stop the motor to avoid overheat

  if (!digitalRead(M_SENSOR_PIN)) {
    minuteHomeCounter++;
  } else {
    minuteHomeCounter = 0;
  }

  if (minuteHomeCounter == 1) {
    totalMinuteSteps = 0;
    return true;
  }

  return false;
}

void testRTC(){
  clock.updateTime(); // Update the time
  Serial.print(clock.hours);
  Serial.print(":");
  Serial.print(clock.minutes);
  Serial.print(":");
  Serial.print(clock.seconds);
}

void findSensorHome(HM type){
  // Turn motors
  if (type == HM::minute) {
    while (digitalRead(M_SENSOR_PIN)){
      minutes.step(-1);
    }
    delay(1000);
  } else {
    while (digitalRead(H_SENSOR_PIN)){
      hours.step(1);
    }
    delay(1000);
  }
}

void sensorInitialAdjust(){
  // Horas
  findSensorHome(HM::hour);

  // Ajustar cuál de las dos tarjetas de hora debe salir según el minuto
  // get the most recent time from clock
  clock.updateTime();
  int halfHourDiff = clock.hours * 2;
  if (clock.minutes >= 30) {
    halfHourDiff++;
  }

  bumpHalfHour(halfHourDiff);

  // Minutos
  findSensorHome(HM::minute);

  clock.updateTime();

  // Tener un valor inicial para currentMinute, para que no siempre se bumpee
  // en la primera vez que se corre el loop
  currentMinute = clock.minutes;
  bumpMinute(currentMinute);
}

void setup() {
  Serial.begin(9600);
  hours.setSpeed(RPM);
  minutes.setSpeed(RPM);

  // Sensores
  pinMode(M_SENSOR_PIN, INPUT);
  pinMode(H_SENSOR_PIN, INPUT);
  
  // Setup
  sensorInitialAdjust();
}

void loop() {
  delay(1000);

  clock.updateTime();
  
  if (currentMinute == clock.minutes) {
    return;
  }

  testRTC();

  currentMinute = clock.minutes;

  if (!ignoreBump){
    isMinuteAtHome = bumpMinute();
  }

  // Descalibrado - reloj muestra 00 y los minutos no son 00
  if (isMinuteAtHome && currentMinute != 0) {
    // Está atrasado
    if (currentMinute < 30) {
      isMinuteAtHome = bumpMinute(currentMinute);
      ignoreBump = false;

    // Está adelantado
    } else {
      // Ignorar bumps hasta que realmente sean las 00
      ignoreBump = true;
    }
  
  // Calibrado e ignoreBump es verdadero
  } else if (ignoreBump) {
    ignoreBump = false;
  }

  // Bump Hours
  if (currentMinute == 0 || currentMinute == 30) {
      bool hourSteps = bumpHalfHour();
  }
}
