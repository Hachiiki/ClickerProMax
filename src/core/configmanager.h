#pragma once
#include <QVector>
#include <QString>
#include <QKeySequence>
#include "clickpoint.h"

/**
 * @brief Determines when the click sequence should terminate
 *
 * Used to configure automatic stop behavior during automation runs.
 */
enum class StopCondition {
    Indefinite  = 0,   ///< Run forever until user manually stops
    TimeLimit   = 1,   ///< Stop after specified duration (in seconds)
    CycleCount  = 2    ///< Stop after N complete cycles through all points
};

/**
 * @brief Specifies which mouse button to use for clicks
 */
enum class MouseButtonType {
    Left   = 0,  ///< Left mouse button (primary)
    Middle = 1,  ///< Middle mouse button (wheel)
    Right  = 2   ///< Right mouse button (secondary)
};

/**
 * @brief Specifies the click action type
 */
enum class ClickType {
    Single = 0,  ///< Single mouse button press
    Double = 1   ///< Double-click (two presses in quick succession)
};

/**
 * @brief Determines cursor position behavior during automation
 */
enum class CursorPositionType {
    Current = 0,  ///< Use current cursor position
    Picked  = 1    ///< Use pre-defined/picked position
};

/**
 * @struct HotkeyConfig
 * @brief Configures the global hotkey for toggling automation on/off
 *
 * Stores both the key code and modifier flags to allow flexible hotkey
 * combinations (e.g., Ctrl+F6, Shift+Space, etc.).
 */
struct HotkeyConfig {
    int key       = Qt::Key_F6;   ///< Qt::Key_* enum value (default: F6)
    int modifiers = 0;            ///< Qt::KeyboardModifiers bitmask (Ctrl, Shift, Alt, Meta)

    /**
     * @brief Converts the hotkey to a human-readable string
     * @return Display string like "F6" or "Ctrl+F6"
     */
    QString toString() const {
        return QKeySequence(modifiers | key).toString();
    }
};

/**
 * @struct AppConfig
 * @brief Complete configuration snapshot for the application
 *
 * Encapsulates all automation settings including:
 * - The sequence of click points to execute
 * - Stop conditions (time limit, cycle count, etc.)
 * - Global hotkey binding
 * - UI preferences (e.g., overlay visibility)
 *
 * Typically serialized to/from JSON for persistence.
 */
struct AppConfig {
    QVector<ClickPoint> points;  ///< Ordered list of target click points to execute

    StopCondition stopCondition   = StopCondition::Indefinite;  ///< When to stop the automation
    int           timeLimitSecs   = 60;   ///< Maximum runtime in seconds (if stopCondition == TimeLimit)
    int           maxCycles       = 10;   ///< Maximum cycles to execute (if stopCondition == CycleCount)

    HotkeyConfig  toggleHotkey;            ///< Global hotkey to start/stop automation
    bool          hideOverlayWhenRunning = false;  ///< Whether to hide visual overlays during execution
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

// ConfigManager
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
    static QString defaultProConfigPath();
};
