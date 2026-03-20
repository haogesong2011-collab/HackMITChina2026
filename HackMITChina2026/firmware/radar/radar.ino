// 发送端
#include <HardwareSerial.h>
#include <esp_now.h>
#include <WiFi.h>

#define MAX_ESPNOW_PAYLOAD 250  // ESP-NOW 最大负载限制

// 设置接收端 MAC 地址（请替换为实际地址）
uint8_t receiverMac[] = {0x94, 0xA9, 0x90, 0x96, 0xD4, 0x94};

/**
 * 目标信息结构体，用于 ESP-NOW 发送
 */
typedef struct {
  uint8_t target_id;
  int8_t angle;
  uint8_t distance;
  uint16_t speed;
  char direction[4]; // "近" 或 "远"
} TargetInfo;

HardwareSerial RadarSerial(1);  // 使用 ESP32-C3 的 UART1

// 协议定义
const uint8_t FRAME_HEADER[4] = {0xF4, 0xF3, 0xF2, 0xF1}; // 帧头（逆序存储）
const uint8_t FRAME_FOOTER[4] = {0xF8, 0xF7, 0xF6, 0xF5}; // 帧尾（逆序存储）
uint8_t buffer[128];     // 数据接收缓冲区
int bufIndex = 0;        // 当前缓冲区写入位置

// ESP‑NOW 发送回调函数
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "ESP-NOW 发送成功" : "ESP-NOW 发送失败");
}

/**
 * 发送目标信息到 ESP-NOW
 */
void sendTargetInfoViaESPNOW(TargetInfo* targetInfo) {
  esp_err_t result = esp_now_send(receiverMac, (uint8_t*)targetInfo, sizeof(TargetInfo));
  Serial.println(result == ESP_OK ? "目标信息发送成功" : "目标信息发送失败");
}

void setup() {
  // 初始化调试串口
  Serial.begin(115200);
  // 初始化雷达串口（此处 RX=5, TX=4，根据实际接线调整）
  RadarSerial.begin(115200, SERIAL_8N1, 5, 4);
  Serial.println("LD2451 Frame Printer Ready");

  // 初始化 WiFi，并设置为 STA 模式
  WiFi.mode(WIFI_STA);
  // 初始化 ESP‑NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初始化失败");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("添加 ESP‑NOW 节点失败");
    return;
  }
}

void loop() {
  // 实时接收数据
  while (RadarSerial.available()) {
    // 读取字节存入缓冲区
    buffer[bufIndex] = RadarSerial.read();
    if (++bufIndex >= sizeof(buffer)) bufIndex = 0;
    // 尝试解析完整数据帧
    tryParseFrame();
  }
}

/**
 * 帧解析核心函数
 * 在缓冲区中查找完整数据帧并进行处理
 */
void tryParseFrame() {
  // 最小长度检查（帧头4 + 长度字段2）
  if (bufIndex < 6) return;
  // 遍历缓冲区查找可能的帧头位置
  for (int start = 0; start <= bufIndex - 4; start++) {
    if (memcmp(buffer + start, FRAME_HEADER, 4) == 0) {
      // 解析数据长度（小端格式：低字节在前）
      uint16_t dataLength = buffer[start + 4] | (buffer[start + 5] << 8);
      // 计算总帧长度 = 头4 + 长度2 + 数据N + 尾4
      int totalLength = 4 + 2 + dataLength + 4;
      if (start + totalLength > bufIndex) return;
      // 验证帧尾
      if (memcmp(buffer + start + totalLength - 4, FRAME_FOOTER, 4) == 0) {
        // 处理完整帧
        parseFrameData(start, start + totalLength);
        // 移除已处理数据
        int remain = bufIndex - (start + totalLength);
        memmove(buffer, buffer + start + totalLength, remain);
        bufIndex = remain;
        return; // 每次只处理一个帧
      }
    }
  }
}

/**
 * 解析帧数据并发送目标信息
 */
void parseFrameData(int frameStart, int frameEnd) {
  uint8_t* payload = buffer + frameStart + 4;  // 跳过帧头
  uint16_t dataLength = payload[0] | (payload[1] << 8);
  payload += 2; // 移动指针到有效数据区

  if (dataLength == 0) {
    Serial.println("无目标靠近");
    return;
  }

  // 解析目标数量和报警状态
  uint8_t targetCount = payload[0];
  uint8_t alarmStatus = payload[1];

  Serial.printf("目标数量：%d | 报警：%s\n", 
                targetCount, 
                (alarmStatus & 0x01) ? "有目标靠近" : "无报警");

  // 解析目标信息并发送
  for (int i = 0; i < targetCount; i++) {
    uint8_t* target = payload + 2 + i * 5;

    if (target + 4 >= buffer + frameEnd) {
      Serial.println("数据不完整");
      break;
    }

    // 解析目标数据
    int8_t angle = target[0] - 0x80;
    uint8_t distance = target[1];
    bool direction = (target[2] & 0x01);
    uint16_t speed = target[3];
    uint8_t snr = target[4];

    Serial.printf("目标%d：角度%+03d° 距离%03dm %s 速度%03dkm/h SNR%03d\n",
                  i + 1, angle, distance,
                  direction ? "靠近" : "远离",
                  speed, snr);

    // 组织目标信息结构体并发送
    TargetInfo targetInfo;
    targetInfo.target_id = i + 1;
    targetInfo.angle = angle;
    targetInfo.distance = distance;
    targetInfo.speed = speed;
    strcpy(targetInfo.direction, direction ? "近" : "远");

    sendTargetInfoViaESPNOW(&targetInfo);
  }
  Serial.println("-------------------");
}
