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

#define M_SENSOR_PIN A0
#define H_SENSOR_PIN A1

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

bool isMinuteatHome = false;
bool isHouratHome = false;

unsigned int totalHourSteps = 0;
unsigned int totalMinuteSteps = 0;

bool bumpHalfHour(int amount = 1) {
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
  
  totalHourSteps += resp;
  //Serial.print("----> Hour Cycle "); Serial.print(cycle);
  Serial.print(" | Stepping "); Serial.print(resp);
  Serial.print(" | Total "); Serial.println(totalHourSteps);
  
  return digitalRead(H_SENSOR_PIN);
}

bool bumpMinute(int amount = 1) {
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
  
  //debug
  int absMinuteSteps = resp * -1;
  totalMinuteSteps += absMinuteSteps;
  //Serial.print("Cycle "); Serial.print(cycle);
  Serial.print(" | Stepping "); Serial.print(absMinuteSteps);
  Serial.print(" | Total "); Serial.println(totalMinuteSteps);
  
  return digitalRead(M_SENSOR_PIN);
  
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

bool findSensorHome(String HM){
  //turn motors
  if (HM == "M"){
    while (digitalRead(M_SENSOR_PIN)){
      minutes.step(-1);
    }
    delay(1000);
    return true;
  }else{
    while (digitalRead(H_SENSOR_PIN)){
      hours.step(1);
    }
    delay(1000);
    return true;
  }
}

void sensorInitialAdjust(){
  
  ////Hours first////
  //go to home
  isHouratHome = findSensorHome("H");

  // Ajustar cuál de las dos tarjetas de hora debe salir
  // según el minuto
  //get the most recent time from clock
  clock.updateTime();
  currentMinute = clock.minutes;
  int halfHourDiff = clock.hours * 2;
  if (clock.minutes >= 30) {
      halfHourDiff++;
    } 
  //Serial.println(halfHourDiff);
  if (isHouratHome){
    bumpHalfHour(halfHourDiff);
  }

  ////Minutes Next////
  //go to home
  testRTC(); //just checking secconds in serialprint
  isMinuteatHome = findSensorHome("M");

  //get the most recent time from clock
  clock.updateTime();
  currentMinute = clock.minutes;
  Serial.print(" Clock Minute in sensorInitialAdjust: ");
  Serial.println(clock.minutes);
  testRTC(); //just checking secconds in serialprint
  if (isMinuteatHome){
    bumpMinute(clock.minutes);
  }
}

void setup() {
  Serial.begin(9600);
  hours.setSpeed(RPM);
  minutes.setSpeed(RPM);

  //sensors
  pinMode(M_SENSOR_PIN, INPUT);
  pinMode(H_SENSOR_PIN, INPUT);
  
  //setup
  sensorInitialAdjust();

  Serial.print("Current Minute Var: ");
  Serial.print(currentMinute);
  Serial.print(" Clock Minute: ");
  Serial.println(clock.minutes);

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

  //bumpmotors verificar si llegó al home cuando se estaba esperando... wip
  bool minuteHome = bumpMinute();

  if (currentMinute == 0 || currentMinute == 30) {
    bool hourSteps = bumpHalfHour();
  }
}

