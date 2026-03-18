#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BNO055.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55);

/// Пины
#define TRIG_L 25
#define ECHO_L 26
#define TRIG_C 27
#define ECHO_C 14
#define TRIG_R 12
#define ECHO_R 13

#define RELAY_L 4
#define RELAY_R 2

#define FlagPort 5
#define SIGNAL_PIN 15

#define DIST_THRESHOLD_CM 290

/// ===== Shared Data =====
struct SharedData {
  long distanceL;
  long distanceC;
  long distanceR;
  float yaw;
};

SharedData data;

SemaphoreHandle_t dataMutex;

/// ===== Ультразвук =====
long measureDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long d = pulseIn(echo, HIGH, 30000);
  if (d == 0) return -1;
  return d * 0.0343 / 2;
}

/// ===== Гироскоп =====
float getYaw() {
  sensors_event_t event;
  bno.getEvent(&event);
  return event.orientation.x;
}

/// ===== МОТОРЫ =====
void forward() {
  digitalWrite(RELAY_L, HIGH);
  digitalWrite(RELAY_R, HIGH);
}

void right() {
  digitalWrite(RELAY_L, HIGH);
  digitalWrite(RELAY_R, LOW);
}

void left() {
  digitalWrite(RELAY_L, LOW);
  digitalWrite(RELAY_R, HIGH);
}

void stopM() {
  digitalWrite(RELAY_L, LOW);
  digitalWrite(RELAY_R, LOW);
}

/// ===== FSM =====
enum State {
  FORWARD,
  TURN_RIGHT,
  TURN_LEFT,
  STOPPED
};

State currentState = FORWARD;

/// ===== TASK 1: ДАТЧИКИ =====
void TaskSensors(void *pvParameters) {
  while (true) {

    long dL = measureDistance(TRIG_L, ECHO_L);
    long dC = measureDistance(TRIG_C, ECHO_C);
    long dR = measureDistance(TRIG_R, ECHO_R);

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      data.distanceL = dL;
      data.distanceC = dC;
      data.distanceR = dR;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(60 / portTICK_PERIOD_MS);
  }
}

/// ===== TASK 2: ГИРО =====
void TaskGyro(void *pvParameters) {
  while (true) {

    float y = getYaw();

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      data.yaw = y;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}

/// ===== TASK 3: ЛОГИКА =====
void TaskControl(void *pvParameters) {

  while (true) {

    long dL, dC, dR;
    float yaw;

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      dL = data.distanceL;
      dC = data.distanceC;
      dR = data.distanceR;
      yaw = data.yaw;
      xSemaphoreGive(dataMutex);
    }

    bool L_free = (dL == -1 || dL > DIST_THRESHOLD_CM);
    bool C_free = (dC == -1 || dC > DIST_THRESHOLD_CM);
    bool R_free = (dR == -1 || dR > DIST_THRESHOLD_CM);

    int flag = digitalRead(FlagPort);

    if (flag == HIGH) {
      forward();
      vTaskDelay(30 / portTICK_PERIOD_MS);
      continue;
    }

    switch (currentState) {

      case FORWARD:
        if (C_free) {
          forward();
        } else {
          if (R_free) currentState = TURN_RIGHT;
          else if (L_free) currentState = TURN_LEFT;
          else currentState = STOPPED;
        }
        break;

      case TURN_RIGHT:
        right();
        if (C_free) currentState = FORWARD;
        break;

      case TURN_LEFT:
        left();
        if (C_free) currentState = FORWARD;
        break;

      case STOPPED:
        stopM();
        digitalWrite(SIGNAL_PIN, HIGH);
        break;
    }

    Serial.print("Yaw: "); Serial.print(yaw);
    Serial.print(" C: "); Serial.print(dC);
    Serial.print(" State: "); Serial.println(currentState);

    vTaskDelay(30 / portTICK_PERIOD_MS);
  }
}

/// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Wire.begin();

  bno.begin();
  delay(1000);

  pinMode(TRIG_L, OUTPUT);
  pinMode(ECHO_L, INPUT);
  pinMode(TRIG_C, OUTPUT);
  pinMode(ECHO_C, INPUT);
  pinMode(TRIG_R, OUTPUT);
  pinMode(ECHO_R, INPUT);

  pinMode(RELAY_L, OUTPUT);
  pinMode(RELAY_R, OUTPUT);
  pinMode(FlagPort, INPUT);
  pinMode(SIGNAL_PIN, OUTPUT);

  dataMutex = xSemaphoreCreateMutex();

  /// Запуск задач

  xTaskCreatePinnedToCore(TaskSensors, "Sensors", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskGyro, "Gyro", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TaskControl, "Control", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // пусто — всё работает в задачах
}