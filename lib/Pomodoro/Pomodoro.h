#pragma once
#include <stdint.h>

namespace pomo {
struct Settings {
    uint8_t focus = 25, shortBreak = 5, longBreak = 15, interval = 4;
    uint8_t sound = 3;
    bool autoStart = false;
    bool valid() const {
        return focus >= 5 && focus <= 60 && focus % 5 == 0 &&
            shortBreak >= 1 && shortBreak <= 15 && longBreak >= 5 &&
            longBreak <= 30 && longBreak % 5 == 0 && interval >= 2 && interval <= 6 && sound <= 3;
    }
    bool operator==(const Settings& b) const {
        return focus == b.focus && shortBreak == b.shortBreak &&
            longBreak == b.longBreak && interval == b.interval &&
            sound == b.sound && autoStart == b.autoStart;
    }
};
enum class Phase { Focus, ShortBreak, LongBreak };
enum class Status { Idle, Running, Paused, Complete };
struct Snapshot {
    Phase phase;
    Status status;
    uint64_t remainingMs;
    uint8_t completed;
};
class Timer {
public:
    explicit Timer(uint32_t minuteMs = 60000) : minuteMs_(minuteMs) { reset(); }
    void configure(const Settings& s) { settings_ = s.valid() ? s : Settings{}; reset(); }
    const Settings& settings() const { return settings_; }
    void reset() {
        phase_ = Phase::Focus; status_ = Status::Idle; completed_ = 0;
        remaining_ = duration();
    }
    // Caller supplies a monotonic 64-bit millisecond timestamp.
    bool update(uint64_t now) {
        if (status_ != Status::Running || now < deadline_) return false;
        remaining_ = 0; status_ = Status::Complete;
        if (phase_ == Phase::Focus) ++completed_;
        return true;
    }
    void confirm(uint64_t now) {
        // A late confirm cannot pause a finished phase or skip its completion screen.
        if (update(now)) return;
        if (status_ == Status::Running) {
            remaining_ = now < deadline_ ? deadline_ - now : 0;
            status_ = Status::Paused;
        } else {
            if (status_ == Status::Complete) {
                if (phase_ == Phase::Focus)
                    phase_ = completed_ >= settings_.interval ? Phase::LongBreak : Phase::ShortBreak;
                else {
                    if (phase_ == Phase::LongBreak) completed_ = 0;
                    phase_ = Phase::Focus;
                }
                remaining_ = duration();
            }
            deadline_ = now + remaining_; status_ = Status::Running;
        }
    }
    Snapshot snapshot(uint64_t now) const {
        return {phase_, status_, status_ == Status::Running ?
            (now < deadline_ ? deadline_ - now : 0) : remaining_, completed_};
    }
private:
    uint64_t duration() const {
        return uint64_t(phase_ == Phase::Focus ? settings_.focus :
            phase_ == Phase::ShortBreak ? settings_.shortBreak : settings_.longBreak) * minuteMs_;
    }
    Settings settings_;
    uint32_t minuteMs_;
    Phase phase_ = Phase::Focus;
    Status status_ = Status::Idle;
    uint8_t completed_ = 0;
    uint64_t remaining_ = 0, deadline_ = 0;
};
enum class Page { Timer, Menu, EndConfirm, Settings, ResetConfirm, Edit };
struct Effects { bool completed = false, save = false; };
class Controller {
public:
    explicit Controller(uint32_t minuteMs = 60000) : timer(minuteMs) {}
    Timer timer;
    Page page = Page::Timer;
    uint8_t selection = 0, field = 0;
    Settings draft;
    Effects step(uint64_t now, bool confirm, bool select) {
        Effects e;
        if (timer.update(now)) {
            page = Page::Timer; selection = 0; e.completed = true;
            if (timer.settings().autoStart) timer.confirm(now);
            return e; // Completion wins over simultaneous input.
        }
        const auto state = timer.snapshot(now).status;
        if (select) {
            if (page == Page::Timer) { page = Page::Menu; selection = 0; }
            else if (page == Page::Menu) selection = (selection + 1) % (state == Status::Idle ? 3 : 2);
            else if (page == Page::EndConfirm) selection = 1 - selection;
            else if (page == Page::Settings) selection = (selection + 1) % 8;
            else if (page == Page::ResetConfirm) selection = 1 - selection;
            else increment();
            return e;
        }
        if (!confirm) return e;
        switch (page) {
        case Page::Timer: timer.confirm(now); break;
        case Page::Menu:
            if (selection == 0) page = Page::Timer;
            else if (selection == 1) { page = Page::EndConfirm; selection = 0; }
            else { page = Page::Settings; selection = 0; }
            break;
        case Page::EndConfirm:
            if (selection == 1) timer.reset();
            page = Page::Timer; selection = 0; break;
        case Page::Settings:
            if (selection == 7) page = Page::Timer;
            else if (selection == 6) { page = Page::ResetConfirm; selection = 0; }
            else { field = selection; draft = timer.settings(); page = Page::Edit; }
            break;
        case Page::ResetConfirm:
            if (selection == 1) {
                const Settings defaults;
                e.save = !(timer.settings() == defaults);
                if (e.save) timer.configure(defaults);
                page = Page::Timer; selection = 0;
            } else {
                page = Page::Settings; selection = 6;
            }
            break;
        case Page::Edit:
            e.save = !(draft == timer.settings());
            if (e.save) timer.configure(draft);
            page = Page::Settings; selection = field; break;
        }
        return e;
    }
private:
    void increment() {
        switch (field) {
        case 0: draft.focus = draft.focus == 60 ? 5 : draft.focus + 5; break;
        case 1: draft.shortBreak = draft.shortBreak == 15 ? 1 : draft.shortBreak + 1; break;
        case 2: draft.longBreak = draft.longBreak == 30 ? 5 : draft.longBreak + 5; break;
        case 3: draft.interval = draft.interval == 6 ? 2 : draft.interval + 1; break;
        case 4: draft.sound = (draft.sound + 1) % 4; break;
        case 5: draft.autoStart = !draft.autoStart; break;
        }
    }
};
} // namespace pomo
