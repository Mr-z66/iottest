# 通信测试指南

## 概述
nihao
本指南说明如何使用模拟数据模式测试ESP32网关的通信功能。模拟数据模式下，所有传感器数据将通过随机数生成器生成，无需连接真实硬件。haha

## 准备工作

### 1. 硬件连接（可选）
如果使用真实传感器，请按照以下连接：
- DHT11: GPIO4
- SGP30: I2C (SDA=18, SCL=19, 地址0x58)
- HW390 (土壤湿度): GPIO34
- DS18B20 (土壤温度): GPIO5 (OneWire)
- 雨传感器: GPIO13
- PIR (人体移动): GPIO14
- GY-302 (光照): I2C (地址0x23)

### 2. 网络配置
编辑 `src/core/config.h`：
```cpp
#define WIFI_SSID "你的WiFi名称"
#define WIFI_PASSWORD "你的WiFi密码"
```

### 3. 模拟数据模式
确保配置文件中设置为模拟模式：
```cpp
#define USE_MOCK_DATA true  // true=模拟数据, false=真实传感器
```

## 启动设备

### 使用 PlatformIO
```bash
# 上传代码到设备
pio run --target upload

# 监控串口输出
pio device monitor
```

### 使用 Arduino IDE
1. 选择开发板: ESP32 Dev Module
2. 选择端口
3. 点击上传
4. 打开串口监视器 (波特率 115200)

## 验证通信

### 1. 检查启动信息
设备启动后，串口应显示：
```
==================================
ESP32 Device Service
==================================
HTTP port: 8080
WebSocket path: /ws
```

### 2. 测试 HTTP API

#### 获取网关状态
```bash
curl http://[设备IP]:8080/api/v1/gateway/status
```

响应示例：
```json
{
  "gatewayId": "esp-gateway-001",
  "online": true,
  "firmwareVersion": "1.2.0",
  "uptimeSec": 123,
  "wifi": {
    "ssid": "your-ssid",
    "ip": "192.168.1.100",
    "rssi": -65,
    "connected": true
  },
  "connectedDeviceCount": 8
}
```

#### 获取遥测数据
```bash
curl http://[设备IP]:8080/api/v1/telemetry
```

响应示例（模拟数据）：
```json
{
  "timestamp": "2025-03-24T14:30:15Z",
  "temperature": 25.3,
  "humidity": 55.2,
  "light": 1250.5,
  "co2": 520,
  "tvoc": 45,
  "soilMoisture": 68.4,
  "soilTemperature": 21.7,
  "rainDetected": false,
  "motionDetected": true
}
```

#### 获取设备列表
```bash
curl http://[设备IP]:8080/api/v1/devices
```

### 3. 测试 WebSocket 连接

#### 使用浏览器 JavaScript 控制台
```javascript
// 连接到 WebSocket
const ws = new WebSocket('ws://[设备IP]:8080/ws');

ws.onopen = () => {
    console.log('WebSocket 已连接');
};

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    console.log('收到消息:', data);
    // 消息类型包括：
    // - telemetry: 遥测数据
    // - deviceStatus: 设备状态
    // - commandResult: 命令执行结果
};

ws.onclose = () => {
    console.log('WebSocket 已断开');
};
```

#### 订阅遥测数据（自动推送）
设备每5秒自动推送遥测数据到所有连接的WebSocket客户端。

### 4. 测试设备控制

#### 控制水泵
```bash
curl -X POST http://[设备IP]:8080/api/v1/devices/[设备ID]/command \
  -H "Content-Type: application/json" \
  -d '{"command":"turnOn", "durationSec":30}'
```

示例设备ID：
- `pump-001` (水泵)
- `grow-light-001` (生长灯)

响应：
```json
{
  "requestId": "uuid-here",
  "deviceId": "pump-001",
  "result": "accepted",
  "finalStatus": "turnedOn",
  "timestamp": "2025-03-24T14:35:00Z"
}
```

#### 停止设备
```bash
curl -X POST http://[设备IP]:8080/api/v1/devices/[设备ID]/command \
  -H "Content-Type: application/json" \
  -d '{"command":"turnOff"}'
```

### 5. 验证模拟数据变化

观察多次调用API时的数据变化：
```bash
# 连续查询5次，每次间隔2秒
for i in {1..5}; do
    curl -s http://[设备IP]:8080/api/v1/telemetry | jq '.'
    sleep 2
done
```

每次响应中，以下数值应会变化：
- `temperature`: 20-30°C 随机
- `humidity`: 40-70% 随机
- `soilMoisture`: 30-80% 随机
- `soilTemperature`: 18-25°C 随机
- `light`: 100-2000 lux 随机
- `co2`: 400-1000 ppm 随机
- `tvoc`: 10-200 ppb 随机
- `rainDetected`: 随机 true/false
- `motionDetected`: 随机 true/false

## 常见问题排查

### 设备无法连接到WiFi
1. 检查 `WIFI_SSID` 和 `WIFI_PASSWORD` 配置
2. 确认WiFi信号强度
3. 查看串口输出连接状态

### WebSocket 连接失败
1. 确认设备IP正确
2. 确认端口8080未被防火墙拦截
3. 检查设备是否已启动WebSocket服务

### 模拟数据未变化
1. 确认 `USE_MOCK_DATA` 设置为 `true`
2. 检查编译时是否使用了正确的配置
3. 重新上传代码

### HTTP API 无响应
1. 检查设备IP地址（串口会显示IP）
2. 确认设备与测试电脑在同一网络
3. 使用 `ping [设备IP]` 测试连通性

## 切换回真实传感器模式

修改配置：
```cpp
#define USE_MOCK_DATA false
```

重新编译上传即可使用真实传感器。

## 性能测试建议

### 压力测试
使用工具批量发送命令：
```bash
# 批量发送10个开灯命令
for i in {1..10}; do
    curl -X POST http://[设备IP]:8080/api/v1/devices/grow-light-001/command \
      -H "Content-Type: application/json" \
      -d '{"command":"turnOn", "durationSec":10}' &
done
wait
```

### 多客户端WebSocket测试
同时打开多个浏览器标签页连接WebSocket，观察数据推送是否稳定。

## 调试技巧

1. 开启详细日志：在 `src/main.cpp` 中增加 `Serial.println` 调试
2. 查看设备内存使用：ESP32-core-decoder 工具
3. 监控网络流量：Wireshark 捕获ESP32流量

## 联系支持

如遇问题，请检查：
1. 串口输出的错误信息
2. 设备LED状态指示
3. GitHub Issues: https://github.com/Mr-z66/iottest/issues