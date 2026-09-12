#include "Hardware.h"
#include "Display.h"
#include "SettingsStore.h"

static constexpr uint32_t minuteMs = 60000;

Hardware hardware;
Display display;
SettingsStore storage;
pomo::Controller app(minuteMs);
bool ready = false;
uint64_t saveErrorUntil = 0;
bool hadSaveError = false;

void setup() {
    Serial.begin(115200);
    const bool hardwareReady = hardware.begin();
    if (!hardwareReady || !display.begin()) {
        M5.Display.setBrightness(153);
        M5.Display.fillScreen(TFT_BLACK);
        M5.Display.setTextColor(TFT_RED);
        M5.Display.drawString("Init failed. See serial.", 4, 45);
        return;
    }
    app.timer.configure(storage.load());
    ready = true;
    display.render(app, Hardware::now(), true, false, hardware.batteryPercent(), hardware.charging());
}
void loop() {
    if (!ready) { delay(10); return; }
    const auto input = hardware.poll();
    const uint64_t now = Hardware::now();
    if (input.confirm || input.select) hardware.touch(now);
    const auto effects = app.step(now, input.confirm, input.select);
    if (effects.completed) hardware.alert(now, app.timer.settings().sound);
    if (effects.save && !storage.save(app.timer.settings())) {
        Serial.println("NVS save failed; settings apply in RAM only");
        saveErrorUntil = now + 3000;
    }
    hardware.update(now);
    const bool saveError = now < saveErrorUntil;
    display.render(app, now, input.confirm || input.select || effects.completed || saveError != hadSaveError, saveError, hardware.batteryPercent(), hardware.charging());
    hadSaveError = saveError;
    delay(1); // Yield to the ESP32 scheduler; never wait for countdown or audio.
}
