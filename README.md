# StickS3 番茄时钟

基于 **PlatformIO + Arduino + M5Unified** 的离线番茄时钟，英文横屏、大号彩色倒计时。依赖版本固定在 `platformio.ini`。

## 使用

默认专注 **25 分钟**、短休息 **5 分钟**，每完成 **4 次**专注后长休息 **15 分钟**。每个阶段结束后等待 Key1 确认，才开始下一阶段。

| 操作位置 | Key1（G11） | Key2（G12） |
|---|---|---|
| 主界面 | 开始／暂停／继续／下一阶段 | 打开菜单 |
| 菜单与设置列表 | 确认 | 循环选择 |
| 设置值编辑 | 保存 | 循环修改 |

菜单中的 **End round** 需二次确认，默认取消；**Settings** 仅在待开始时可用。打开菜单不会暂停计时。

数字颜色：**灰白＝待开始，绿色＝计时中，琥珀黄＝暂停，蓝色 `00:00`＝完成**。顶部左侧显示阶段，中间显示本轮次数，右上角显示电量百分比（每 5 秒更新，低于或等于 20% 显示琥珀黄，读取失败显示 `--%`）。正在充电时，电量左侧显示黄色闪电，停止充电后隐藏（最多延迟 5 秒更新）。电量由电压估算，充电或负载变化时可能波动。

| 设置 | 范围 | 默认 |
|---|---|---|
| Focus duration | 5–60 分钟，步进 5 | 25 |
| Short break | 1–15 分钟，步进 1 | 5 |
| Long break | 5–30 分钟，步进 5 | 15 |
| Long break interval | 2–6 次 | 4 |
| Sound | On / Off | On |

设置确认后保存，重启保留设置但重置计时和次数。出现 `Save failed: RAM only` 时，新设置仅在本次运行生效。

30 秒无操作后屏幕由 60% 调暗至约 10%；按键或到时恢复亮度。到时播放一次短提示音，可在设置中关闭。

## 构建与烧录

安装 PlatformIO，在项目目录运行：

```sh
pio run -e sticks3
pio device list
pio run -e sticks3 -t upload
pio device monitor -b 115200
```

多个串口时，上传命令添加 `--upload-port <实际串口>`。固件位于 `.pio/build/sticks3/firmware.bin`，使用上述上传命令处理分区和引导程序。

## 项目结构

- `lib/Pomodoro/Pomodoro.h`：独立于硬件的计时状态机、设置与菜单控制器。
- `src/Hardware.h`、`src/Display.h`、`src/SettingsStore.h`：硬件、绘制与持久化适配。
- `src/main.cpp`：协调输入、计时、提醒、保存和显示。

参考：[StickS3 官方文档](https://docs.m5stack.com/en/core/StickS3)
