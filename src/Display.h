#pragma once
#include <M5Unified.h>
#include <Pomodoro.h>
#include <stdio.h>

class Display {
public:
    bool begin() { canvas_.setColorDepth(16); return canvas_.createSprite(240, 135) != nullptr; }
    void render(const pomo::Controller& app, uint64_t now, bool dirty, bool saveError, int batteryPercent, bool charging) {
        const auto s = app.timer.snapshot(now);
        const uint64_t seconds = (s.remainingMs + 999) / 1000;
        const bool secondsChanged = app.page == pomo::Page::Timer && seconds != lastSeconds_;
        if (!dirty && !secondsChanged && batteryPercent == lastBatteryPercent_ && charging == lastCharging_) return;
        lastBatteryPercent_ = batteryPercent;
        lastCharging_ = charging;
        lastSeconds_ = seconds;
        canvas_.fillScreen(0x0841);
        canvas_.setTextColor(TFT_WHITE); canvas_.setTextSize(1);
        canvas_.setFont(&fonts::Font2);
        const uint16_t accent = s.phase == pomo::Phase::Focus ? 0xFC89 : 0x4E99;
        if (app.page == pomo::Page::Timer) {
            const char* phase = s.phase == pomo::Phase::Focus ? "FOCUS" :
                s.phase == pomo::Phase::ShortBreak ? "SHORT BREAK" : "LONG BREAK";
            canvas_.setTextSize(1);
            text(phase, 10, 5, accent);
            char count[20]; snprintf(count, sizeof(count), "%u/%u", s.completed, app.timer.settings().interval);
            text(count, 10 + canvas_.textWidth(phase) + 8, 5, 0xBDF7);
            char battery[8];
            if (batteryPercent < 0) snprintf(battery, sizeof(battery), "--%%");
            else snprintf(battery, sizeof(battery), "%d%%", batteryPercent);
            const int batteryX = 230 - canvas_.textWidth(battery);
            if (charging) {
                const int x = batteryX - 13;
                // Two filled triangles form a compact 9x13 lightning bolt.
                canvas_.fillTriangle(x + 6, 6, x, 13, x + 5, 13, TFT_YELLOW);
                canvas_.fillTriangle(x + 3, 11, x + 8, 11, x + 2, 18, TFT_YELLOW);
            }
            text(battery, batteryX, 5,
                batteryPercent >= 0 && batteryPercent <= 20 ? 0xFDE0 : 0xBDF7);
            canvas_.setTextSize(1);
            // RGB565: ready gray-white, running green, paused amber, complete blue.
            const uint16_t timerColor = s.status == pomo::Status::Idle ? 0xD69A :
                s.status == pomo::Status::Running ? 0x4E68 :
                s.status == pomo::Status::Paused ? 0xFDE0 : 0x55BF;
            char clock[16];
            snprintf(clock, sizeof(clock), "%02u:%02u", unsigned(seconds / 60), unsigned(seconds % 60));
            canvas_.setFont(&fonts::Font7);
            canvas_.setTextSize(1.6f);
            // Center the approximately 77px-high digits below the 24px header.
            text(clock, (240 - canvas_.textWidth(clock)) / 2, 43, timerColor);
            canvas_.setTextSize(1);
            canvas_.setFont(&fonts::Font2);
        } else {
            const char* title = app.page == pomo::Page::Menu ? "ACTIONS" :
                app.page == pomo::Page::EndConfirm ? "END THIS ROUND?" :
                app.page == pomo::Page::Settings ? "SETTINGS" : "EDIT SETTING";
            text(title, 10, 4, accent);
            if (app.page == pomo::Page::Menu) {
                row("Back", 0, app.selection); row("End round", 1, app.selection);
                if (s.status == pomo::Status::Idle) row("Settings", 2, app.selection);
            } else if (app.page == pomo::Page::EndConfirm) {
                row("Cancel", 0, app.selection); row("End round", 1, app.selection);
            } else if (app.page == pomo::Page::Settings) {
                // Scroll a three-row viewport so text stays legible on the small LCD.
                const int start = app.selection < 3 ? 0 : 3;
                const char* page = start == 0 ? "1/2" : "2/2";
                text(page, 230 - canvas_.textWidth(page), 4, 0xBDF7);
                const auto& settings = app.timer.settings();
                for (int i = start; i < start + 3; ++i) {
                    char value[12] = "";
                    if (i == 0) snprintf(value, sizeof(value), "%um", settings.focus);
                    else if (i == 1) snprintf(value, sizeof(value), "%um", settings.shortBreak);
                    else if (i == 2) snprintf(value, sizeof(value), "%um", settings.longBreak);
                    else if (i == 3) snprintf(value, sizeof(value), "%u", settings.interval);
                    else if (i == 4) snprintf(value, sizeof(value), "%s", settings.sound ? "On" : "Off");
                    row(label(i), i - start, app.selection - start, value);
                }
            } else {
                text(label(app.field), 10, 32, 0xBDF7);
                char value[24];
                const auto& d = app.draft;
                if (app.field == 4) snprintf(value, sizeof(value), "%s", d.sound ? "On" : "Off");
                else snprintf(value, sizeof(value), "%u %s", app.field == 0 ? d.focus : app.field == 1 ? d.shortBreak :
                    app.field == 2 ? d.longBreak : d.interval, app.field == 3 ? "sessions" : "min");
                canvas_.fillRect(10, 59, 4, 34, accent);
                canvas_.setFont(&fonts::Font4); text(value, 22, 61, accent);
                canvas_.setFont(&fonts::Font2);
            }
            if (app.page == pomo::Page::Edit) footer("K1 Save", "Change K2");
            else footer("K1 Confirm", "Select K2");
        }
        if (saveError) { canvas_.fillRect(0, 112, 240, 23, TFT_RED); text("Save failed: RAM only", 10, 115, TFT_WHITE); }
        canvas_.pushSprite(0, 0);
    }
private:
    M5Canvas canvas_{&M5.Display};
    int lastBatteryPercent_ = -2;
    bool lastCharging_ = false;
    uint64_t lastSeconds_ = UINT64_MAX;
    void text(const char* s, int x, int y, uint16_t color) {
        canvas_.setTextColor(color); canvas_.drawString(s, x, y);
    }
    void row(const char* s, int position, int selected, const char* value = nullptr) {
        int y = 29 + position * 25;
        if (position == selected) canvas_.fillRoundRect(6, y, 228, 23, 4, 0x2945);
        const uint16_t color = position == selected ? TFT_WHITE : 0xBDF7;
        text(s, 14, y + 2, color);
        if (value && value[0]) text(value, 226 - canvas_.textWidth(value), y + 2, color);
    }
    void footer(const char* left, const char* right) {
        text(left, 10, 115, 0xBDF7);
        text(right, 230 - canvas_.textWidth(right), 115, 0xBDF7);
    }
    static const char* label(int i) {
        static const char* labels[] = {"Focus", "Short break", "Long break", "Long interval", "Sound", "Back"};
        return labels[i];
    }
};
