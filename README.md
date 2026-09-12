# 🍅 Simple Pomodoro Timer for StickS3

English | [简体中文](README_zh-CN.md)

A simple offline Pomodoro timer for the **M5Stack StickS3**, built with **PlatformIO + Arduino + M5Unified**.

I couldn't find a Pomodoro timer for the StickS3, so I was forced to make one myself. It counts down and reminds you when it's time to take a break.

## Usage

Default configuration:

- Focus: **25 minutes**
- Short break: **5 minutes**
- Long break: **15 minutes**
- A Long break after every **4 Focus sessions**

After each phase finishes, press **Key1** to start the next one.

The device can remind you to take a break. It cannot take the break for you.

| Screen          | Key1 (G11)                          | Key2 (G12) |
| --------------- | ----------------------------------- | ---------- |
| Timer           | Start / Pause / Resume / Next phase | Open menu  |
| Menu / Settings | Confirm                             | Select     |
| Edit setting    | Save                                | Change     |

## Display

The main screen shows:

- Current phase and round count
- Large countdown timer
- Battery percentage
- Charging status

Timer colors:

- Gray-white: Ready
- Green: Running
- Amber: Paused
- Blue: Complete

Battery status updates every **5 seconds**.

At or below 20%, the battery indicator turns amber. If the battery level cannot be read, it shows `--%`. A yellow lightning bolt appears while charging.

After 30 seconds of inactivity, screen brightness drops from about 60% to about 5%.

The screen never turns off completely. A countdown timer that refuses to show the remaining time would be neglecting its professional duties.

## Actions

The menu contains:

- Back
- End round
- Settings

**End round** requires confirmation and defaults to Cancel.

Accidentally ending a focus session is not the kind of surprise a productivity tool should provide.

Settings are only available while the timer is idle.

## Settings

| Setting       | Range                     | Default |
| ------------- | ------------------------- | ------- |
| Focus         | 5–60 min                  | 25      |
| Short break   | 1–15 min                  | 5       |
| Long break    | 5–30 min                  | 15      |
| Long interval | 2–6                       | 4       |
| Sound         | Off / Low / Medium / High | High    |
| Auto start    | On / Off                  | Off     |

The current value is shown directly in Settings.

Settings are saved persistently. After a reboot, your configuration is preserved, while the current timer and completed round count are reset.

If you see:

```text
Save failed: RAM only
```

the new settings are only active for the current session. After rebooting, the device will develop selective amnesia.

## 🔊 Sound

An embedded alert sound is played when a phase finishes.

It can be disabled in Settings.

The speaker is shut down after playback, instead of quietly consuming power for no particular reason.

## Build

```sh
pio run -e sticks3
```

Upload:

```sh
pio run -e sticks3 -t upload
```

Serial monitor:

```sh
pio device monitor -b 115200
```

The firmware binary is located at:

```text
.pio/build/sticks3/firmware.bin
```

## Project Structure

```text
lib/Pomodoro/Pomodoro.h   Timer state machine, settings, and menu logic
src/Hardware.h            Buttons, battery, backlight, and speaker
src/Display.h             UI
src/SettingsStore.h       Persistent settings
src/main.cpp              Glues everything together
```

## Hardware

Target device: **M5Stack StickS3**

Currently used:

- ESP32-S3
- LCD
- Key1 / Key2
- Built-in speaker
- Battery and charging status
- M5PM1

The BMI270, microphone, Wi-Fi, infrared, and other hardware are not being used just because they happen to exist.

Features are not collectibles.

## Why

Phone-based Pomodoro timers have a classic problem:

```text
Open phone to check timer
↓
See notification
↓
Tap notification
↓
Twenty minutes later
↓
What was I doing again?
```

So this is a device that only counts down.

It doesn't even have internet access.

Very advanced.

## Reference

Official M5Stack StickS3 documentation:

https://docs.m5stack.com/en/core/StickS3