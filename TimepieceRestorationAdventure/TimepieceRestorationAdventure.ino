/**
 * @file Timepiece Restoration Adventure.ino
 * Leonel Óscar Corona Villarreal
 * Leonardo Corona Garza
 */

#include <Stepper.h>
#include <virtuabotixRTC.h>               //Include library for clock module

#define STEPS_PER_REVOLUTION 2048
#define RPM 12

#define M_SENSOR_PIN A0
#define H_SENSOR_PIN A1

// Grande: 75
// Chico: 18
// Pasos: 142.2 periódico

//UNO//
//Stepper hours(STEPS_PER_REVOLUTION, 8, 10, 9, 11);
//Stepper minutes(STEPS_PER_REVOLUTION, 4, 6, 5, 7);
//virtuabotixRTC clock(3, 12, 13);    //Create a clock object attached to pins 3, 12, 13 - CLK, DAT, RST

//NANO//
Stepper minutes(STEPS_PER_REVOLUTION, 5, 3, 4, 2); //pins nano pcb in1,2,3,4 / 5,4,3,2
Stepper hours(STEPS_PER_REVOLUTION, 9, 7, 8, 6); //pins nano pcb in1,2,3,4 / 9,8,7,6
virtuabotixRTC clock(12, 11, 10);    //NANO Create a clock object attached to pins 12, 11, 10 - CLK, DAT, RST

// seconds, minutes, hours, day of the week, day of the month, month, year
//clock.setDS1302Time(00, 00, 18, 1, 05, 2, 2023);        //Only required once to reset the clock time

int currentMinute = 0;
int minutecycle = 0;
int minuteHomeCounter = 0;
int leapHourCounter = 0;
int leapMinuteCounter = 0;
byte state;

bool isMinuteatHome = false;
bool isHouratHome = false;
bool minuteAutoCalibration = false;

unsigned int totalHourSteps = 0;
unsigned int totalMinuteSteps = 0;

void powerDownStepper (String HM)
{
  if (HM == "H"){
    for (byte i = 6 ; i <= 9 ; i++) 
    {
      state = (state << 1) + digitalRead(i) ;
      pinMode (i, INPUT) ;
    }
  } else if (HM == "M"){
    for (byte i = 2 ; i <= 5 ; i++) 
    {
      state = (state << 1) + digitalRead(i) ;
      pinMode (i, INPUT) ;
    }
  }
}

void powerUpStepper (String HM)
{
  if (HM == "H"){
    for (byte i = 6 ; i <= 9 ; i++) 
    {
      pinMode (i, OUTPUT) ;
      digitalWrite (i, (state & 0x8) == 0x8) ;
      state <<= 1 ;
    }
  } else if (HM == "M"){
    for (byte i = 2 ; i <= 5 ; i++) 
    {
      pinMode (i, OUTPUT) ;
      digitalWrite (i, (state & 0x8) == 0x8) ;
      state <<= 1 ;
    }
  }
}

bool bumpHalfHour(int amount = 1) {
  powerUpStepper("H"); // start the motor  
  int hourSteps;
  leapHourCounter++;
  if (leapHourCounter == 3) {
    hourSteps = 726;
    leapHourCounter = 0;
  } else {
    hourSteps = 725;
  }
  long resp = long(hourSteps) * long(amount);

  if (resp > 32767){
    int respdiff = resp - 32767;
    hours.step(respdiff);
    hours.step(32767);
  }else{
    hours.step(resp);
  }
  
  totalHourSteps += resp;
  Serial.print(F(" | Stepping ")); Serial.print(resp);
  Serial.print(F(" | Total ")); Serial.println(totalHourSteps);
  powerDownStepper("H"); // stop the motor to avoid overheat
  return !digitalRead(H_SENSOR_PIN);
}

bool bumpMinute(int amount = 1) {
  powerUpStepper("M"); // start the motor

  int minuteSteps;
  long minuteStepsInit;

  //for to get the perfect steps when sending more than 1
  if (amount > 1 ){
    for (int i=1; i <= amount; i++){

      minutecycle++;
      if (minutecycle == 60){
        minutecycle = 0;
      } else if (minutecycle > 60) {
        minutecycle = minutecycle - 60;
      }
      
      leapMinuteCounter++;
      if (minutecycle == 30){
        minuteSteps = -151;
        leapMinuteCounter = 0;
      } else if (leapMinuteCounter == 10) {
        //minuteSteps = -152;
        if (minutecycle != 30){
          minuteSteps = -152;
        }
        leapMinuteCounter = 0;
      } else {
        //minuteSteps = -151;
        minuteSteps = -153;
      }
      minuteStepsInit += minuteSteps;
    }

    Serial.print(F("leapMinuteCounter ")); Serial.print(leapMinuteCounter);
    Serial.print(F(" Minute Steps ")); Serial.print(minuteSteps);
    Serial.print(F(" Minute Cycle ")); Serial.println(minutecycle);

    minutes.step(minuteStepsInit);

    int absMinuteSteps = minuteStepsInit * -1;
    totalMinuteSteps += absMinuteSteps;

  } else {

    minutecycle = minutecycle + amount;
    if (minutecycle == 60){
      minutecycle = 0;
    } else if (minutecycle > 60) {
      minutecycle = minutecycle - 60;
    }

    leapMinuteCounter++;
    if (minutecycle == 30){
        minuteSteps = -151;
        leapMinuteCounter = 0;
      } else if (leapMinuteCounter == 10) {
        //minuteSteps = -152;
        if (minutecycle != 30){
          minuteSteps = -152;
        }
        leapMinuteCounter = 0;
      } else {
        //minuteSteps = -151;
        minuteSteps = -153;
    }
    Serial.print(F("leapMinuteCounter ")); Serial.print(leapMinuteCounter);
    Serial.print(F(" Minute Steps ")); Serial.print(minuteSteps);
    Serial.print(F(" Minute Cycle ")); Serial.println(minutecycle);
    long resp = minuteSteps * amount;
    minutes.step(resp);
    
    //debug
    int absMinuteSteps = resp * -1;
    totalMinuteSteps += absMinuteSteps;
    //Serial.print("Cycle "); Serial.print(cycle);
    Serial.print(F(" | Stepping ")); Serial.print(absMinuteSteps);
    Serial.print(F(" | Total ")); Serial.println(totalMinuteSteps);
  }

  powerDownStepper("M"); // stop the motor to avoid overheat

  if (!digitalRead(M_SENSOR_PIN)){
    minuteHomeCounter ++;
      if(minuteHomeCounter == 1){
        totalMinuteSteps = 0;
        leapMinuteCounter = 0;
       
      }
  }else{
    minuteHomeCounter = 0;
  }

  if (!digitalRead(M_SENSOR_PIN)){
    return true;
  }else {
    return false;
  }
}

void testRTC(){
  clock.updateTime(); //Update the time
  Serial.print(clock.hours);
  Serial.print(":");
  Serial.print(clock.minutes);
  Serial.print(":");
  Serial.print(clock.seconds);
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
  //go home
  isHouratHome = findSensorHome("H");

  // Ajustar cuál de las dos tarjetas de hora debe salir según el minuto
  //get the most recent time from clock
  clock.updateTime();
  currentMinute = clock.minutes;
  int halfHourDiff = clock.hours * 2;
  if (clock.minutes >= 30) {
      halfHourDiff++;
    } 

  if (isHouratHome){
    bumpHalfHour(halfHourDiff);
  }

  ////Minutes Next////
  //go home
  isMinuteatHome = findSensorHome("M");

  //get the most recent time from clock
  clock.updateTime();
  currentMinute = clock.minutes;
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
}

void loop() {
  delay(1000);

  clock.updateTime();
  
  if (currentMinute == clock.minutes) {
    return;
  }

  testRTC();
  currentMinute = clock.minutes;

  //bumpmotors //minutes// check if its at home when expected... wip
  if (!minuteAutoCalibration){
    isMinuteatHome = bumpMinute();
  }
  if (isMinuteatHome && clock.minutes != 0 && minuteHomeCounter == 1){ //Descalibrado// reloj muestra 00 y los minutos no son 00
    if (isMinuteatHome && clock.minutes > 0 && minuteHomeCounter == 1){
      isMinuteatHome = bumpMinute(clock.minutes);
      minuteAutoCalibration = false; 
    } else if (isMinuteatHome && clock.minutes < 0 && minuteHomeCounter == 1){
    //need to ignore bumps until clock.minutes reach 0
      minuteAutoCalibration = true;
    } 
  } else {
    if (minuteAutoCalibration){
        minuteAutoCalibration = false; 
        isMinuteatHome = bumpMinute();
    }
  }  

  //Bump Hours
  if (currentMinute == 0 || currentMinute == 30) {
      bool hourSteps = bumpHalfHour();
  } 

}