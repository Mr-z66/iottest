# 使用手册

## 1. 当前版本能做什么

当前版本已经把 ESP32 主板补成了一个可联调的局域网设备服务端。

已完成功能：

- 主板连接 Wi‑Fi
- 采集 7 类传感器数据
- 提供实时环境快照接口
- 提供统一设备列表接口
- 提供执行器控制接口
- 推送实时遥测、设备状态和命令结果

当前支持的传感器：

- DHT11
- SGP30
- HW-390
- DS18B20
- MH-RD
- HC-SR501
- GY-302

当前支持的执行器：

- `pump-001`
- `growlight-001`

## 2. 先改哪些配置

先打开 [config.h](/e:/软工/iottest/iottest/src/core/config.h)。

最少先改：

```c
#define WIFI_SSID "your-ssid"
#define WIFI_PASSWORD "your-password"
```

可按需修改：

- `GATEWAY_ID`
- `FIRMWARE_VERSION`
- `GATEWAY_HTTP_PORT`
- `PUMP_RELAY_PIN`
- `GROW_LIGHT_RELAY_PIN`

## 3. 默认接线

### 3.1 传感器

| 设备 | 引脚连接 |
| --- | --- |
| DHT11 | `DATA -> GPIO4` |
| SGP30 | `SDA -> GPIO18`, `SCL -> GPIO19` |
| HW-390 | `AO -> GPIO34` |
| DS18B20 | `DATA -> GPIO5` |
| MH-RD | `DO -> GPIO13` |
| HC-SR501 | `OUT -> GPIO14` |
| GY-302 | `SDA -> GPIO18`, `SCL -> GPIO19` |

### 3.2 HC-SR501 红外传感器

| HC-SR501 | ESP32 | 说明 |
| --- | --- | --- |
| `VCC` | `5V` | 供电 |
| `GND` | `GND` | 共地 |
| `OUT` | `GPIO14` | 高电平表示检测到移动 |

说明：

- 上电后建议等待 30 到 60 秒再看稳定输出
- 如果后续改了 `PIR_SENSOR_PIN`，接线也要同步修改

### 3.3 GY-302 光照传感器

| GY-302 | ESP32 | 说明 |
| --- | --- | --- |
| `VCC` | `3.3V` | 供电 |
| `GND` | `GND` | 共地 |
| `SDA` | `GPIO18` | I2C 数据线 |
| `SCL` | `GPIO19` | I2C 时钟线 |

说明：

- 与 SGP30 共用同一组 I2C 总线
- 默认地址 `0x23`

### 3.4 执行器

| 执行器 | ESP32 引脚 |
| --- | --- |
| 水泵继电器输入 | `GPIO26` |
| 补光灯继电器输入 | `GPIO27` |

说明：

- 当前默认按低电平触发继电器处理
- 如果你的继电器模块是高电平触发，需要修改：
  - `PUMP_ACTIVE_LEVEL`
  - `GROW_LIGHT_ACTIVE_LEVEL`

## 4. 启动后会提供哪些接口

默认端口：

- HTTP：`8080`
- WebSocket：同端口路径 `/ws`

### 4.1 `GET /gateway/status`

用途：查看主板在线状态、IP、RSSI、固件版本、设备数量。

### 4.2 `GET /telemetry/realtime`

用途：查看实时环境快照。

当前可能返回的字段：

- `temperature`
- `humidity`
- `light`
- `co2`
- `eco2`
- `tvoc`
- `soilMoisture`
- `timestamp`

### 4.3 `GET /devices`

用途：查看统一设备列表，包括传感器和执行器。

### 4.4 `POST /devices/{id}/command`

用途：控制执行器。

当前支持设备：

- `pump-001`
- `growlight-001`

请求示例：

```json
{
  "command": "TURN_ON",
  "params": {
    "durationSec": 30
  },
  "requestId": "cmd-20260324-0001"
}
```

### 4.5 `GET /commands/{requestId}`

用途：查询命令最终结果。

## 5. WebSocket 推送

当前会推送 3 类消息：

- `telemetry.update`
- `device.status.changed`
- `command.result`

说明：

- 当前实现使用同端口 WebSocket 路径：

```text
ws://<esp-ip>:8080/ws
```

## 6. 联调建议顺序

### 第一步：先验证主板联网

看串口输出，确认主板已经连上 Wi‑Fi。

### 第二步：验证基础接口

建议先测：

1. `GET /gateway/status`
2. `GET /telemetry/realtime`
3. `GET /devices`

### 第三步：验证执行器控制

先发：

```json
{
  "command": "TURN_ON",
  "requestId": "cmd-test-0001"
}
```

到：

```text
POST /devices/growlight-001/command
```

再查：

```text
GET /commands/cmd-test-0001
```

说明：

- 当前命令处理已调整为“先 accepted，再异步执行”
- 如果设备不存在、命令不支持、参数非法，会直接返回错误，不再伪装成 accepted

## 7. 当前版本的已知说明

- 当前已经能联调主板侧核心接口
- `co2` 当前兼容返回，语义上对应 SGP30 的 `eCO2`
- 同时也会返回 `eco2` 字段，便于上层按更准确语义解析
- 本机当前环境缺少 `pio` 命令，所以仓库内代码尚未在这里完成编译验证

## 8. 后续还可以继续补的点

- 更严格的命令状态机
- WebSocket 心跳和重连策略
- 采样去抖与阈值上报
- 更完整的错误码
- 时间同步失败时的降级处理
- 同端口 `/ws` WebSocket 实现
