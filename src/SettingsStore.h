#pragma once
#include <Preferences.h>
#include <Pomodoro.h>

class SettingsStore {
public:
    pomo::Settings load() {
        pomo::Settings s;
        if (!prefs_.begin("pomodoro", false)) return s;
        ready_ = true;
        const uint32_t value = prefs_.getUInt("settings", 0);
        // One atomic NVS value: magic/version, auto-start, sound, interval, long, short, focus.
        if ((value >> 24) != 0x51) return s;
        s.focus = value & 63; s.shortBreak = (value >> 6) & 15;
        s.longBreak = (value >> 10) & 31; s.interval = (value >> 15) & 7;
        s.sound = (value >> 18) & 3;
        s.autoStart = (value >> 20) & 1;
        return s.valid() ? s : pomo::Settings{};
    }
    bool save(const pomo::Settings& s) {
        if (!ready_) return false;
        const uint32_t value = (0x51UL << 24) | uint32_t(s.autoStart) << 20 | uint32_t(s.sound) << 18 |
            uint32_t(s.interval) << 15 | uint32_t(s.longBreak) << 10 |
            uint32_t(s.shortBreak) << 6 | s.focus;
        return prefs_.putUInt("settings", value) == sizeof(value);
    }
private:
    Preferences prefs_;
    bool ready_ = false;
};
