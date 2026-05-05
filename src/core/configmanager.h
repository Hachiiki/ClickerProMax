#pragma once
#include <QVector>
#include <QString>
#include <QKeySequence>
#include "clickpoint.h"

// ── Shared enums ─────────────────────────────────────────────────────────────
enum class StopCondition {
    Indefinite  = 0,   // run forever until manually stopped
    TimeLimit   = 1,   // stop after N seconds
    CycleCount  = 2    // stop after N full cycles through all points (multipoint), or N clicks (normal)
};

enum class MouseButtonType {
    Left   = 0,
    Middle = 1,
    Right  = 2
};

enum class ClickType {
    Single = 0,
    Double = 1
};

enum class CursorPositionType {
    Current = 0,
    Picked  = 1
};

// ── Hotkey descriptor ─────────────────────────────────────────────────────────
struct HotkeyConfig {
    int key       = Qt::Key_F6;   // Qt::Key_*
    int modifiers = 0;            // Qt::KeyboardModifiers bitmask

    // Returns a human-readable string like "F6" or "Ctrl+F6"
    QString toString() const {
        return QKeySequence(modifiers | key).toString();
    }
};

// ── Full application config ───────────────────────────────────────────────────
struct AppConfig {
    QVector<ClickPoint> points;

    StopCondition stopCondition   = StopCondition::Indefinite;
    int           timeLimitSecs   = 60;
    int           maxCycles       = 10;

    HotkeyConfig  toggleHotkey;      // single key that starts / stops the clicker
    bool          hideOverlayWhenRunning = false;  // hide circles while clicking
};

struct NormalAppConfig {
    int hours = 0;
    int mins  = 0;
    int secs  = 0;
    int ms    = 100;

    MouseButtonType buttonType = MouseButtonType::Left;
    ClickType       clickType  = ClickType::Single;

    bool repeatUntilStopped = true;
    int  repeatTimes = 1;

    CursorPositionType positionType = CursorPositionType::Current;
    int pickedX = 0;
    int pickedY = 0;

    HotkeyConfig toggleHotkey;
};

// ── ConfigManager ─────────────────────────────────────────────────────────────
class ConfigManager {
public:
    // Save returns false on I/O error.
    static bool save(const AppConfig& config, const QString& filePath = {});

    // Load returns false if the file does not exist or is malformed.
    // On failure `config` is left unchanged.
    static bool load(AppConfig& config, const QString& filePath = {});

    static bool saveNormal(const NormalAppConfig& config, const QString& filePath = {});
    static bool loadNormal(NormalAppConfig& config, const QString& filePath = {});

    static QString defaultConfigPath();
    static QString defaultNormalConfigPath();
};
