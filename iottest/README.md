# ESP32 主板侧设备接入服务

## 项目简介

这个项目用于把 ESP32 主板做成一个局域网内可访问的设备接入端。

当前已经具备：

- 多传感器统一采集
- 主板侧设备模型输出
- Wi‑Fi STA 联网与重连
- REST API
- WebSocket 推送
- 水泵 / 补光灯执行器控制
- 命令 `requestId` 跟踪

项目当前只实现主板侧，不包含 HAP / OpenHarmony 页面和网关宿主侧代码。

## 目录结构

```text
src/
|-- main.cpp
|-- core/
|   |-- config.h
|   |-- gateway_models.h
|   |-- time_utils.h / time_utils.cpp
|   |-- telemetry_cache.h / telemetry_cache.cpp
|   `-- device_registry.h / device_registry.cpp
|-- sensors/
|   |-- sensor_collector.h / sensor_collector.cpp
|   |-- dht11_sensor.*
|   |-- sgp30_sensor.*
|   |-- hw390_sensor.*
|   |-- ds18b20_sensor.*
|   |-- rain_sensor.*
|   |-- pir_sensor.*
|   `-- gy302_sensor.*
|-- network/
|   |-- wifi_manager.h / wifi_manager.cpp
|   |-- rest_server.h / rest_server.cpp
|   `-- ws_publisher.h / ws_publisher.cpp
`-- control/
    |-- actuator_controller.h / actuator_controller.cpp
    `-- command_tracker.h / command_tracker.cpp

docs/
`-- USAGE.md
```

## 模块说明

- `core/`：放配置、统一数据模型、时间工具、快照缓存、设备注册
- `sensors/`：放传感器驱动和统一采集器
- `network/`：放 Wi‑Fi、REST、WebSocket
- `control/`：放执行器控制和命令跟踪
- `docs/USAGE.md`：使用手册，包含配置、接线、接口和联调示例

## 当前已完成的功能

### 采集侧

- DHT11 温湿度采集
- SGP30 空气质量采集
- HW-390 土壤湿度采集
- DS18B20 土壤温度采集
- MH-RD 雨滴检测
- HC-SR501 红外人体检测
- GY-302 光照采集

### 主板服务侧

- `GET /gateway/status`
- `GET /telemetry/realtime`
- `GET /devices`
- `POST /devices/{id}/command`
- `GET /commands/{requestId}`
- WebSocket 推送：
  - `telemetry.update`
  - `device.status.changed`
  - `command.result`
- WebSocket 接入路径 `/ws`

### 控制侧

- `pump-001` 水泵继电器控制
- `growlight-001` 补光灯继电器控制
- 水泵最大持续时长保护
- 命令 30 秒幂等窗口

## 你先看哪份文档

- 总览看当前这个 README
- 实际使用、接线、接口示例、测试步骤看 [USAGE.md](/e:/软工/iottest/iottest/docs/USAGE.md)

## 编译

```bash
pio run
```

## 下载

```bash
pio run --target upload
```

## 串口监控

```bash
pio device monitor
```
