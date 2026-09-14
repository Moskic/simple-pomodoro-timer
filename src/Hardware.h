#pragma once
#include <M5Unified.h>
#include <esp32-hal-cpu.h>
#include <esp_timer.h>
#include "NotificationSound.h"

struct Input { bool confirm, select; };
class Hardware {
public:
    void begin() {
        auto cfg = M5.config();
        cfg.internal_imu = false; cfg.internal_rtc = false;
        cfg.internal_mic = false;
        cfg.internal_spk = true; // Configure routing; shut down until an alert is needed.
        cfg.fallback_board = m5::board_t::board_M5StickS3;
        M5.begin(cfg);
        setCpuFrequencyMhz(80);
        M5.Display.setRotation(1);
        M5.Speaker.end();
        M5.BtnA.setDebounceThresh(30); M5.BtnB.setDebounceThresh(30);
        M5.Power.setExtOutput(false);
        touch(now());
        readBattery(now());
        Serial.printf("Flash: %u; PSRAM: %u\n", ESP.getFlashChipSize(), ESP.getPsramSize());
    }
    static uint64_t now() { return uint64_t(esp_timer_get_time()) / 1000; }
    int batteryPercent() const { return batteryPercent_; }
    bool charging() const { return charging_; }
    Input poll() {
        M5.update();
        return {M5.BtnA.wasPressed(), M5.BtnB.wasPressed()};
    }
    void touch(uint64_t time) {
        lastActivity_ = time;
        if (dimmed_) { M5.Display.setBrightness(153); dimmed_ = false; }
    }
    void alert(uint64_t time, uint8_t sound) {
        touch(time);
        playSound(sound, 3);
    }
    void previewSound(uint64_t time, uint8_t sound) {
        touch(time);
        if (sound == 0) {
            if (audioPlaying_) {
                M5.Speaker.stop();
                M5.Speaker.end();
                audioPlaying_ = false;
            }
            return;
        }
        playSound(sound, 1);
    }
    void update(uint64_t time) {
        if (time >= nextBatteryRead_) readBattery(time);
        if (!dimmed_ && time - lastActivity_ >= 30000) {
            M5.Display.setBrightness(13); dimmed_ = true;
        }
        if (audioPlaying_ && !M5.Speaker.isPlaying()) {
            M5.Speaker.end(); audioPlaying_ = false;
        }
    }
private:
    void playSound(uint8_t sound, uint32_t repeat) {
        if (sound == 0) return;
        if (!M5.Speaker.begin()) return;
        M5.Speaker.setVolume(sound == 1 ? 128 : sound == 2 ? 181 : 255);
        audioPlaying_ = M5.Speaker.playWav(notificationSound, notificationSoundLength, repeat, -1, true);
        if (!audioPlaying_) M5.Speaker.end();
    }
    void readBattery(uint64_t time) {
        const int level = M5.Power.getBatteryLevel();
        batteryPercent_ = level < 0 ? -1 : (level > 100 ? 100 : level);
        charging_ = M5.Power.isCharging() == m5::Power_Class::is_charging;
        nextBatteryRead_ = time + 5000;
    }
    int batteryPercent_ = -1;
    bool charging_ = false;
    uint64_t nextBatteryRead_ = 0;
    bool dimmed_ = true;
    bool audioPlaying_ = false;
    uint64_t lastActivity_ = 0;
};
