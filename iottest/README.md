# ESP32 多传感器系统（模块化架构）

## 项目概述

这是一个基于 ESP32 的模块化传感器数据采集系统。项目使用 Arduino Framework 开发，传感器代码按模块拆分，便于维护和继续扩展。

目前项目已经支持 7 种传感器，并且新增了：

- `HC-SR501` 红外人体传感器
- `GY-302` 光照传感器

## 支持的传感器

| 传感器 | 类型 | 测量内容 | 接口方式 |
| --- | --- | --- | --- |
| DHT11 | 温湿度传感器 | 温度、湿度 | 单总线数字 |
| SGP30 | 空气质量传感器 | TVOC、eCO2 | I2C |
| HW-390 | 土壤湿度传感器 | 土壤湿度百分比 | 模拟 ADC |
| MH-RD | 雨滴传感器 | 是否检测到雨滴 | 数字输入 |
| DS18B20 | 温度传感器 | 温度 | 1-Wire |
| HC-SR501 | 红外人体传感器 | 是否检测到人体移动 | 数字输入 |
| GY-302 | 光照传感器 | 光照强度（lux） | I2C |

## 目录结构

```text
src/
|-- main.cpp
|-- config.h
|-- dht11_sensor.h / dht11_sensor.cpp
|-- sgp30_sensor.h / sgp30_sensor.cpp
|-- hw390_sensor.h / hw390_sensor.cpp
|-- rain_sensor.h / rain_sensor.cpp
|-- ds18b20_sensor.h / ds18b20_sensor.cpp
|-- pir_sensor.h / pir_sensor.cpp
`-- gy302_sensor.h / gy302_sensor.cpp
```

## 项目设计风格

### 配置集中管理

所有硬件引脚和关键参数都集中放在 `src/config.h` 中，包括：

- 串口波特率
- GPIO 引脚定义
- I2C SDA/SCL
- I2C 地址
- 传感器采样间隔

### 传感器模块统一风格

每个传感器模块保持类似结构：

- 独立的 `.h/.cpp`
- 提供初始化接口
- 提供读取接口
- 提供 getter 获取采样结果
- 使用 `SENSOR_OK / SENSOR_ERROR / SENSOR_DISABLED` 表示状态

本次新增的 `HC-SR501` 与 `GY-302` 也按同样风格实现，便于以后继续新增其他数字型或 I2C 型传感器。

## 新增传感器说明

### HC-SR501 红外人体传感器

- 默认接入引脚：`GPIO14`
- 输出方式：数字量
- 输出逻辑：
  - `HIGH` 表示检测到人体移动
  - `LOW` 表示当前无人体移动

代码文件：

- `src/pir_sensor.h`
- `src/pir_sensor.cpp`

在 `main.cpp` 中已经完成：

- 初始化
- 周期读取
- 串口状态输出
- 系统状态汇总

### GY-302 光照传感器

- 默认使用 I2C 接口
- 默认引脚：
  - `SDA = GPIO18`
  - `SCL = GPIO19`
- 默认地址：`0x23`
- 使用 `BH1750` 库驱动
- 输出单位：`lux`

代码文件：

- `src/gy302_sensor.h`
- `src/gy302_sensor.cpp`

在 `main.cpp` 中已经完成：

- I2C 初始化
- 光照采样
- lux 输出
- 简单亮度等级判断

亮度判断规则：

- `< 10 lux`：极暗
- `< 100 lux`：较暗
- `< 1000 lux`：室内正常光照
- `>= 1000 lux`：强光环境

## 默认硬件配置

当前 `src/config.h` 中的默认配置如下：

| 传感器 | 默认配置 | 说明 |
| --- | --- | --- |
| DHT11 | `GPIO4` | 温湿度数据脚 |
| SGP30 | `GPIO18/19`, `0x58` | I2C |
| HW-390 | `GPIO34` | ADC 输入 |
| MH-RD | `GPIO13` | 数字输入 |
| DS18B20 | `GPIO5` | 1-Wire |
| HC-SR501 | `GPIO14` | 数字输入 |
| GY-302 | `GPIO18/19`, `0x23` | 与 SGP30 共用 I2C |

## 传感器接线说明

### DHT11

- `DATA -> GPIO4`
- `VCC -> 3.3V/5V`
- `GND -> GND`

### SGP30

- `SDA -> GPIO18`
- `SCL -> GPIO19`
- `VCC -> 3.3V`
- `GND -> GND`

### HW-390

- `AO -> GPIO34`
- `VCC -> 3.3V/5V`
- `GND -> GND`

### MH-RD

- `DO -> GPIO13`
- `VCC -> 3.3V/5V`
- `GND -> GND`

### DS18B20

- `DATA -> GPIO5`
- `VCC -> 3.3V/5V`
- `GND -> GND`

说明：`DATA` 与 `VCC` 之间建议串接 `4.7K` 上拉电阻。

### HC-SR501

- `OUT -> GPIO14`
- `VCC -> 5V`
- `GND -> GND`

说明：HC-SR501 上电后通常需要一定稳定时间，刚启动时短时间误触发是常见现象。

HC-SR501 推荐按下面方式连接：

| HC-SR501 引脚 | ESP32 引脚 | 说明 |
| --- | --- | --- |
| `VCC` | `5V` | 传感器供电 |
| `GND` | `GND` | 共地 |
| `OUT` | `GPIO14` | 数字输出，高电平表示检测到人体移动 |

补充说明：

- 如果你修改了 [config.h](/e:/软工/iottest/iottest/src/config.h#L21) 里的 `PIR_SENSOR_PIN`，README 中的接线也要一起改
- HC-SR501 模块上通常有两个电位器，可调灵敏度和触发延时
- 初次上电后建议等待 30 到 60 秒再观察稳定输出
- 如果你使用的是其他 ESP32 开发板，只要把 `OUT` 接到可用数字输入 GPIO 即可

### GY-302

- `SDA -> GPIO18`
- `SCL -> GPIO19`
- `VCC -> 3.3V`
- `GND -> GND`

说明：`GY-302` 与 `SGP30` 共用 I2C 总线，默认地址分别为：

- `SGP30 = 0x58`
- `GY-302 = 0x23`

GY-302 推荐按下面方式连接：

| GY-302 引脚 | ESP32 引脚 | 说明 |
| --- | --- | --- |
| `VCC` | `3.3V` | 模块供电 |
| `GND` | `GND` | 共地 |
| `SDA` | `GPIO18` | I2C 数据线 |
| `SCL` | `GPIO19` | I2C 时钟线 |

补充说明：

- 本项目中 `GY302_SDA` 和 `GY302_SCL` 默认复用 [config.h](/e:/软工/iottest/iottest/src/config.h#L29) 里的 I2C 总线定义
- 如果总线上已经接了 `SGP30`，可以直接并联在同一组 `SDA/SCL` 上，不需要单独再开一组 I2C
- 上电后如果一直读不到数据，优先检查 `VCC/GND` 是否接反，以及模块地址是否为 `0x23`
- 某些 GY-302 模块也支持 `0x5C` 地址，如果你的模块不是 `0x23`，需要同步修改 [config.h](/e:/软工/iottest/iottest/src/config.h#L32)

## 使用说明

### 编译项目

```bash
pio run
```

### 上传到设备

```bash
pio run --target upload
```

### 监控串口输出

```bash
pio device monitor
```

## 串口输出示例

```text
==================================
ESP32 多传感器系统（模块化架构）
==================================

正在初始化传感器...
DHT11 初始化成功
SGP30 初始化成功
HW-390 初始化成功
DS18B20 初始化成功
MH-RD 雨滴传感器初始化成功
HC-SR501 红外传感器初始化成功
GY-302 光照传感器初始化成功

==================================
系统就绪，开始读取数据...
==================================

----------------------------------
运行时间: 2 秒

[传感器 HC-SR501]
  数字电平: 1
  状态说明: 检测到人体移动
  读取状态: 正常

[传感器 GY-302]
  光照强度: 356.40 lux
  环境判断: 室内正常光照
  读取状态: 正常
```

## 如何继续添加新传感器

推荐沿用当前项目的接入方式：

1. 创建 `xxx_sensor.h` 和 `xxx_sensor.cpp`
2. 在 `config.h` 中增加引脚或地址定义
3. 在 `main.cpp` 中增加：
   - 全局实例
   - `setup()` 初始化
   - `loop()` 读取与输出
   - `printSystemStatus()` 汇总

这次新增内容可以作为模板参考：

- `pir_sensor.*`：适合数字输入类传感器
- `gy302_sensor.*`：适合 I2C 类传感器

## 依赖库

`platformio.ini` 中已配置：

- `adafruit/DHT sensor library`
- `adafruit/Adafruit Unified Sensor`
- `adafruit/Adafruit SGP30 Sensor`
- `claws/BH1750`
- `paulstoffregen/OneWire`
- `milesburton/DallasTemperature`

## 开发建议

- 首次接线后先分别验证单个传感器是否正常工作
- I2C 设备增多时，优先确认地址是否冲突
- 如果 HC-SR501 输出过于敏感，可通过模块电位器调节灵敏度和延时
- 如果 GY-302 数据波动较大，可以后续增加滑动平均处理
