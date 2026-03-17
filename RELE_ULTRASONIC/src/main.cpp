#include <Arduino.h>
/// Порты датчика 
#define TRIG_L 25
#define ECHO_L 26
#define TRIG_C 27
#define ECHO_C 14
#define TRIG_R 12
#define ECHO_R 13
//////Порты РЕЛЕ
#define RELAY_L  4
#define RELAY_R  2
/// Порты управления 
#define FlagPort 5
// порог в сантиметрах (2.9 метра)
#define DIST_THRESHOLD_CM 290

void setup() {
  Serial.begin(115200);
/////// PINMODES
  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_C, OUTPUT);
  pinMode(ECHO_C, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);
  pinMode(RELAY_R, OUTPUT);
  pinMode(RELAY_L, OUTPUT);
  ///
  pinMode(FlagPort,INPUT);
  // включаем реле по умолчанию
  digitalWrite(RELAY_L, HIGH); 
  digitalWrite(RELAY_R, HIGH);
}

long measureDistanceCM_LEFT() {
  // очищаем TRIG
  digitalWrite(TRIG_L, LOW);
  delayMicroseconds(2);
  // импульс 10 мкс
  digitalWrite(TRIG_L, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_L, LOW);
  // измеряем echo (таймаут 30 мс ≈ 5 м)
  long duration = pulseIn(ECHO_L, HIGH, 30000);
  if (duration == 0) {
    return -1; // нет сигнала
  }
  // скорость звука 343 м/с
  long distance = duration * 0.0343 / 2;
  return distance;
}

long measureDistanceCM_CENTER() {
  // очищаем TRIG
  digitalWrite(TRIG_C, LOW);
  delayMicroseconds(2);
  // импульс 10 мкс
  digitalWrite(TRIG_C, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_C, LOW);
  // измеряем echo (таймаут 30 мс ≈ 5 м)
  long duration = pulseIn(ECHO_C, HIGH, 30000);
  if (duration == 0) {
    return -1; // нет сигнала
  }
  // скорость звука 343 м/с
  long distance = duration * 0.0343 / 2;
  return distance;
}

long measureDistanceCM_RIGHT() {
  // очищаем TRIG
  digitalWrite(TRIG_R, LOW);
  delayMicroseconds(2);
  // импульс 10 мкс
  digitalWrite(TRIG_R, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_R, LOW);
  // измеряем echo (таймаут 30 мс ≈ 5 м)
  long duration = pulseIn(ECHO_R, HIGH, 30000);
  if (duration == 0) {
    return -1; // нет сигнала
  }
  // скорость звука 343 м/с
  long distance = duration * 0.0343 / 2;
  return distance;
}

void loop() {
  long distanceL = measureDistanceCM_LEFT();
  long distanceC = measureDistanceCM_CENTER();
  long distanceR = measureDistanceCM_RIGHT();
 
  int Flag = digitalRead(FlagPort);

    Serial.print("Distance Left: ");
    Serial.print(distanceL);
    Serial.println(" cm");
    
    Serial.print("Distance Center: ");
    Serial.print(distanceC);
    Serial.println(" cm");

    Serial.print("Distance Right: ");
    Serial.print(distanceR);
    Serial.println(" cm");

if(Flag==LOW){
    if (distanceL > DIST_THRESHOLD_CM && distanceC >DIST_THRESHOLD_CM && distanceR >DIST_THRESHOLD_CM || distanceC , distanceR ,distanceL ==-1) {
      // объект ближе 3 м → выключаем реле
           digitalWrite(RELAY_L, HIGH);
           digitalWrite(RELAY_R, HIGH);
      Serial.println("RELAY ON FRONT");} 
    else if(distanceL < DIST_THRESHOLD_CM && distanceC < DIST_THRESHOLD_CM && distanceR > DIST_THRESHOLD_CM || 
    distanceL < DIST_THRESHOLD_CM && distanceC < DIST_THRESHOLD_CM && distanceR ==-1){    
           digitalWrite(RELAY_L, HIGH);
           digitalWrite(RELAY_R, LOW);
      Serial.println("RELAY ON- ROTATE RIGHT");}
    else if(distanceR < DIST_THRESHOLD_CM && distanceC < DIST_THRESHOLD_CM && distanceL > DIST_THRESHOLD_CM || 
    distanceR < DIST_THRESHOLD_CM && distanceC < DIST_THRESHOLD_CM && distanceL ==-1){    
           digitalWrite(RELAY_L, LOW);
           digitalWrite(RELAY_R, HIGH);
      Serial.println("RELAY ON- ROTATE LEFT");}
    else if(distanceR < DIST_THRESHOLD_CM && distanceC < DIST_THRESHOLD_CM && distanceL < DIST_THRESHOLD_CM){    
           digitalWrite(RELAY_L, LOW);
           digitalWrite(RELAY_R, LOW);
      Serial.println("RELAY ON- MOVE BACK");}
    else {
      digitalWrite(RELAY_L, LOW);
      digitalWrite(RELAY_R, LOW); 
      Serial.println("RELAY OFF");
    }}
    else{     
      digitalWrite(RELAY_L, LOW);
      digitalWrite(RELAY_R, LOW); 
      Serial.println("RELAY OFF");}
  // Serial.println("No echo signal");
  delay(50);
}
