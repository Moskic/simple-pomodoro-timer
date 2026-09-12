# 🍅 StickS3 简单番茄时钟

一个运行在 **M5Stack StickS3** 上的简单离线番茄钟。基于 **PlatformIO + Arduino + M5Unified**。

由于实在是找不到在StickS3上跑的番茄时钟，只能被迫自己搓一个。它只负责倒计时，然后提醒你该休息了。

## 使用

默认配置：

- Focus：**25 分钟**
- Short break：**5 分钟**
- Long break：**15 分钟**
- 每完成 **4 个 Focus** 进入一次 Long break

每个阶段结束后，需要按 **Key1** 才会进入下一阶段。

机器可以提醒你休息，但不会替你休息。

| 位置        | Key1（G11）                   | Key2（G12） |
| ----------- | ----------------------------- | ----------- |
| 主界面      | 开始 / 暂停 / 继续 / 下一阶段 | 打开菜单    |
| 菜单 / 设置 | 确认                          | 选择        |
| 编辑设置    | 保存                          | 修改        |

## 界面

主界面显示：

- 当前阶段与轮次
- 大号倒计时
- 电量百分比
- 充电状态

倒计时颜色：

- 灰白：待开始
- 绿色：计时中
- 琥珀黄：暂停
- 蓝色：完成

电量每 **5 秒**更新一次。

低于或等于 20% 时变为琥珀黄；读取失败显示 `--%`；充电时显示黄色闪电。

30 秒无操作后，屏幕亮度会从约 60% 降到约 5%。

屏幕不会彻底关闭，毕竟一个看不到剩余时间的倒计时器，多少有点失去职业尊严。

## Actions

菜单包含：

- Back
- End round
- Settings

**End round** 需要二次确认，并默认选中 Cancel。

因为误触结束一轮专注，不属于效率工具该提供的惊喜。

Settings 仅在待开始状态下可用。

## 设置

| 设置          | 范围      | 默认 |
| ------------- | --------- | ---- |
| Focus         | 5–60 分钟 | 25   |
| Short break   | 1–15 分钟 | 5    |
| Long break    | 5–30 分钟 | 15   |
| Long interval | 2–6       | 4    |
| Sound         | Off / Low / Medium / High | High |
| Auto start    | On / Off  | Off  |

Settings 中会直接显示当前值。

设置会持久化保存；重启后保留配置，但当前计时和已完成轮次会重置。

如果出现：

```text
Save failed: RAM only
```

说明新设置只在当前运行中有效，重启后设备会选择性失忆。

## 🔊 提示音

阶段完成后播放一次内嵌提示音。

可在 Settings 中关闭。

播放结束后扬声器会关闭，不会继续在那里默默耗电。

## 构建

```sh
pio run -e sticks3
```

烧录：

```sh
pio run -e sticks3 -t upload
```

串口监视：

```sh
pio device monitor -b 115200
```

固件位于：

```text
.pio/build/sticks3/firmware.bin
```

## 项目结构

```text
lib/Pomodoro/Pomodoro.h   计时状态机、设置与菜单逻辑
src/Hardware.h            按键、电池、背光、扬声器
src/Display.h             UI
src/SettingsStore.h       设置持久化
src/main.cpp              把这些东西粘在一起
```

## 硬件

目标设备：**M5Stack StickS3**

目前主要使用：

- ESP32-S3
- LCD
- Key1 / Key2
- 内置扬声器
- 电池与充电状态
- M5PM1

BMI270、麦克风、Wi-Fi、红外等功能目前没有为了“硬件有，所以必须用”而强行加入。功能不是集邮。

## 为什么做这个

因为手机番茄钟有一个经典问题：

```text
打开手机看计时
↓
看到通知
↓
顺手点开
↓
二十分钟以后
↓
我刚才要干嘛？
```

所以做一个只会倒计时的设备，它甚至没有互联网。非常先进。

## 参考

M5Stack StickS3 官方文档：

https://docs.m5stack.com/en/core/StickS3
