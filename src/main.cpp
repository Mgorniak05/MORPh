#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <VL53L1X.h>
#include "MotorController.h"
#include "secrets.h"
#include "Filters.h"

#define PWMA 5
#define AIN1 4
#define AIN2 7
#define PWMB 17
#define BIN1 15
#define BIN2 16
#define SERVO_PIN 14

#define I2C_SDA 2
#define I2C_SCL 1

MotorController robot(PWMA, AIN1, AIN2, PWMB, BIN1, BIN2);
WebSocketsServer webSocket = WebSocketsServer(81);
Servo radarServo;
VL53L1X tofSensor;

int currentSpeed = 80;
String currentAction = "STOP";
int distance = 0;
int rawDistance = 0;
DistanceFilter tofFilter;
bool isSensorReady = false;
unsigned long servoStartTime = 0;
bool isServoStepping = false;
bool isFindingZero = false;

const int SERVO_STOP = 94;

void setFastSensor()
{
  if (!isSensorReady)
    return;
  tofSensor.stopContinuous();
  tofSensor.setMeasurementTimingBudget(33000);
  tofSensor.startContinuous(35);
}

void setNormalSensor()
{
  if (!isSensorReady)
    return;
  tofSensor.stopContinuous();
  tofSensor.setMeasurementTimingBudget(220000);
  tofSensor.startContinuous(230);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    robot.stop();
    currentAction = "STOP";
    break;

  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected to: %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
  }
  break;

  case WStype_TEXT:
    String command = String((char *)payload);

    if (command == "Forward")
    {
      robot.goForward(currentSpeed);
      currentAction = "Forward";
    }
    else if (command == "Backward")
    {
      robot.goBack(currentSpeed);
      currentAction = "Backward";
    }
    else if (command == "Left")
    {
      robot.turnLeft(currentSpeed);
      currentAction = "Left";
    }
    else if (command == "Right")
    {
      robot.turnRight(currentSpeed);
      currentAction = "Right";
    }
    else if (command == "Stop")
    {
      robot.stop();
      currentAction = "STOP";
    }

    if (command == "ServoLeft")
    {
      radarServo.write(98); // q
      servoStartTime = millis();
      isServoStepping = true;
      if (isFindingZero)
        setNormalSensor();
      isFindingZero = false;
    }
    else if (command == "ServoRight")
    {
      radarServo.write(91); // e
      servoStartTime = millis();
      isServoStepping = true;
      if (isFindingZero)
        setNormalSensor();
      isFindingZero = false;
    }
    else if (command == "FindZero")
    {
      isFindingZero = true;
      setFastSensor();
      radarServo.write(98);
      servoStartTime = millis();
      isServoStepping = false;
    }
    else if (command == "ServoStop")
    {
      radarServo.write(SERVO_STOP);
      servoStartTime = millis();
      isServoStepping = false;
      if (isFindingZero)
        setNormalSensor();
      isFindingZero = false;
    }
    break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(2000);
  robot.begin();

  ESP32PWM::allocateTimer(0);
  radarServo.setPeriodHertz(50);
  radarServo.attach(SERVO_PIN, 500, 2400);
  radarServo.write(SERVO_STOP);

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setTimeOut(100);
  tofSensor.setTimeout(500);
  if (tofSensor.init())
  {
    isSensorReady = true;
    tofSensor.setDistanceMode(VL53L1X::Long);
    tofSensor.setROISize(16, 14);
    tofSensor.setROICenter(199);
    tofSensor.writeReg16Bit(VL53L1X::RANGE_CONFIG__MIN_COUNT_RATE_RTN_LIMIT_MCPS, 15);
    tofSensor.setMeasurementTimingBudget(220000);
    tofSensor.startContinuous(230);
    Serial.println("Sensor initialized successfully!");
  }
  else
  {
    Serial.println("Failed to detect and initialize sensor!");
  }
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("-> Open HTML and enter this IP: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop()
{
  webSocket.loop();

  if (isSensorReady && tofSensor.dataReady())
  {
    rawDistance = tofSensor.read(false);
    Serial.printf("RAW:%4d | st:%d | SIG:%.2f | AMB:%.2f\n",
                  rawDistance,
                  tofSensor.ranging_data.range_status,
                  tofSensor.ranging_data.peak_signal_count_rate_MCPS,
                  tofSensor.ranging_data.ambient_count_rate_MCPS);
    if (tofSensor.ranging_data.range_status != 4 && tofSensor.ranging_data.range_status != 5 && tofSensor.ranging_data.peak_signal_count_rate_MCPS >= 0.12f && rawDistance < 4000 && !isFindingZero)
    {
      distance = tofFilter.processDistance(rawDistance);
    }
    else if (tofSensor.ranging_data.range_status == 4 || rawDistance > 4000)
    {
      distance = 8191;
    }
    if (isFindingZero && (rawDistance > 20 && rawDistance < 150) && (millis() - servoStartTime > 250))
    {
      radarServo.write(SERVO_STOP);
      setNormalSensor();
      isFindingZero = false;
      Serial.println("Zero found");
    }
  }
  if (isFindingZero && (millis() - servoStartTime >= 4645))
  {
    radarServo.write(SERVO_STOP);
    setNormalSensor();
    isFindingZero = false;
    Serial.println("Timeout");
  }

  if (isServoStepping && (millis() - servoStartTime >= 4635)) //  37 ,  4632
  {
    radarServo.write(SERVO_STOP);
    isServoStepping = false;
  }

  static unsigned long lastTelemetry = 0;
  if (millis() - lastTelemetry > 150)
  {
    lastTelemetry = millis();

    String telemetryData = currentAction + "," + String(distance);
    webSocket.broadcastTXT(telemetryData);
  }
}