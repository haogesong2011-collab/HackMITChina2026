#include <esp_now.h>
#include <WiFi.h>
#include <string.h>

// 目标信息结构体
typedef struct {
  uint8_t target_id;
  int8_t angle;       // 正值：右侧，负值：左侧
  uint8_t distance;   // 单位：m (0~100)
  uint16_t speed;     // 单位：km/h
  char direction[4];  // "近" 或 "远"
} TargetInfo;

TargetInfo receivedTarget;
unsigned long lastDataTimeLeft = 0;
unsigned long lastDataTimeRight = 0;

// LED 引脚定义
const int LED_LEFT_PIN  = 6;
const int LED_RIGHT_PIN = 7;

// LED 控制变量
unsigned long previousMillisLeft = 0;
unsigned long previousMillisRight = 0;
bool leftLEDState = false;
bool rightLEDState = false;

// 固定开灯时长（ms）
const unsigned long LED_ON_DURATION = 500;

// PWM输出占空比（约2.9V输出）
const int PWM_DUTY = 225;

// ESP-NOW接收回调函数
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  if (len == sizeof(TargetInfo)) {
    memcpy(&receivedTarget, incomingData, sizeof(TargetInfo));

    Serial.printf("收到目标%d: 角度 %+03d° | 距离 %03dm | 方向 %s | 速度 %03dkm/h\n",
                  receivedTarget.target_id,
                  receivedTarget.angle,
                  receivedTarget.distance,
                  receivedTarget.direction,
                  receivedTarget.speed);

    if (receivedTarget.distance <= 100 && receivedTarget.speed >= 5 && strcmp(receivedTarget.direction, "近") == 0) {
      unsigned long blinkInterval = calculateBlinkInterval(receivedTarget.distance, receivedTarget.speed);

      // **左侧目标**
      if (receivedTarget.angle < -3) {
        lastDataTimeLeft = millis();  // 更新左侧数据时间
        previousMillisLeft = 0;       // 让 LED 立即执行更新
      }

      // **右侧目标**
      else if (receivedTarget.angle > 3) {
        lastDataTimeRight = millis(); // 更新右侧数据时间
        previousMillisRight = 0;      // 让 LED 立即执行更新
      }

      // **目标在后方（|angle| ≤ 3）**
      else {
        lastDataTimeLeft = millis();
        lastDataTimeRight = millis();
        previousMillisLeft = 0;
        previousMillisRight = 0;
      }
    }
  }
}

// 根据距离与速度计算闪烁间隔（单位：ms）
unsigned long calculateBlinkInterval(uint8_t distance, uint16_t speed) {
  float interval = 2.0;  // 初始最大间隔 2 秒

  // **距离因素**（100m ~ 0m）
  if (distance > 40) interval = 2.0;   // 远离（>80m）
  else if (distance > 30) interval = 1.5;
  else if (distance > 20) interval = 1.0;
  else if (distance > 10) interval = 0.5;
  else if (distance > 5) interval = 0.25;
  else interval = 0.0;   // ≤10m，持续亮灯

  // **速度因素**（5km/h ~ 80km/h）
  if (speed > 60) interval -= 1.5;   // 超高速（亮灯）
  else if (speed > 40) interval -= 1.0;
  else if (speed > 20) interval -= 0.5;
  else if (speed > 10) interval -= 0.25;

  // **确保间隔在 0.0~2.0 之间**
  if (interval < 0.0) interval = 0.0;
  if (interval > 2.0) interval = 2.0;

  return (unsigned long)(interval * 1000);  // 转换为毫秒
}


// **LED 更新函数**（独立处理）
void updateLED(uint8_t pin, unsigned long &previousMillis, bool &ledState, unsigned long interval) {
  unsigned long currentMillis = millis();

  if (ledState && currentMillis - previousMillis >= LED_ON_DURATION) {
    previousMillis = currentMillis;
    ledState = false;
    analogWrite(pin, 0);
  } else if (!ledState && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = true;
    analogWrite(pin, PWM_DUTY);
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初始化失败");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);

  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(LED_RIGHT_PIN, OUTPUT);

  analogWrite(LED_LEFT_PIN, 0);
  analogWrite(LED_RIGHT_PIN, 0);
}

void loop() {
  unsigned long currentMillis = millis();

  // **独立处理左侧 LED**
  if (currentMillis - lastDataTimeLeft <= 2000) {
    unsigned long blinkIntervalLeft = calculateBlinkInterval(receivedTarget.distance, receivedTarget.speed);
    updateLED(LED_LEFT_PIN, previousMillisLeft, leftLEDState, blinkIntervalLeft);
  } else {
    analogWrite(LED_LEFT_PIN, 0);  // 超时后关闭
  }

  // **独立处理右侧 LED**
  if (currentMillis - lastDataTimeRight <= 2000) {
    unsigned long blinkIntervalRight = calculateBlinkInterval(receivedTarget.distance, receivedTarget.speed);
    updateLED(LED_RIGHT_PIN, previousMillisRight, rightLEDState, blinkIntervalRight);
  } else {
    analogWrite(LED_RIGHT_PIN, 0);  // 超时后关闭
  }

  delay(10);
}
