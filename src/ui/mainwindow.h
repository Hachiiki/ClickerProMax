#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "core/clickpoint.h"
#include "core/configmanager.h"
#include "engine/clickerengine.h"
#include "overlaywindow.h"

/**
 * @class MainWindow
 * @brief Main UI for multi-point automated clicking
 *
 * Manages the complete workflow for creating, editing, and executing
 * automated click sequences. Features include:
 *   - Click point list management (add, remove, reorder, edit)
 *   - Visual overlay for point capture and positioning
 *   - Automation engine control (start/stop)
 *   - Configuration persistence (save/load to JSON)
 *   - Stop condition configuration (infinite, time limit, cycle count)
 *   - Global hotkey registration for start/stop
 *   - Real-time status updates and feedback
 *
 * Can operate in standard or "Pro" mode (passed via constructor).
 * Handles Windows native events to intercept the registered hotkey.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /// Constructs MainWindow for standard or pro mode
    explicit MainWindow(bool isProMode = false, QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    /// Emitted when user requests to return to home screen
    void backRequested();

protected:
    /// Intercepts Windows native WM_HOTKEY messages for global hotkey support
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    /// Handles cleanup when window is closed
    void closeEvent(QCloseEvent*) override;
    /// Performs setup when window becomes visible
    void showEvent(QShowEvent*) override;
    /// Performs cleanup when window is hidden
    void hideEvent(QHideEvent*) override;

private slots:
    /// User clicked "Add Point" button - enables overlay capture mode
    void onAddPoint();
    /// User clicked "Remove Point" button - deletes selected point
    void onRemovePoint();
    /// User clicked "Edit Point" button - allows editing point's delay/label
    void onEditPoint();
    /// User clicked "Clear All" button - removes all points with confirmation
    void onClearAll();
    /// User clicked "Move Up" button - reorders selected point earlier in sequence
    void onMoveUp();
    /// User clicked "Move Down" button - reorders selected point later in sequence
    void onMoveDown();
    /// User clicked "Start/Stop" button or pressed hotkey - toggles automation
    void onStartStop();
    /// User clicked "Save" button - persists configuration to JSON file
    void onSaveConfig();
    /// User clicked "Settings" button - opens configuration dialog
    void onOpenSettings();

    /// Overlay emitted pointCaptured signal - adds new point at screen position
    void onPointCaptured(QPoint screenPos);
    /// User pressed Escape while in capture mode - aborts point capture
    void onCaptureAborted();

    /// Engine started running - updates UI to reflect running state
    void onEngineStarted();
    /// Engine stopped - updates UI with stop reason
    void onEngineStopped(const QString& reason);
    /// Engine completed a click - updates UI to highlight current point
    void onCurrentPointChanged(int index);
    /// Engine emitted status message - displays feedback to user
    void onStatusMessage(const QString& msg);

private:
    /// Constructs the entire UI layout with buttons, list, and status labels
    void buildUI();
    /// Connects signals from UI widgets, overlay, and engine to their handlers
    void connectSignals();

    /// Updates the point list display to reflect current m_config.points
    void refreshPointList();
    /// Copies current points from m_config to the engine for execution
    void pushPointsToEngine();

    /// Registers the global hotkey with Windows (if on Windows platform)
    void registerHotkey();
    /// Unregisters the global hotkey with Windows
    void unregisterHotkey();

    /// Updates UI to show running/idle state (button text, enable/disable controls)
    void setRunning(bool running);
    /// Refreshes the hotkey display label with current hotkey binding
    void updateHotkeyLabel();
    /// Refreshes the stop condition display label with current settings
    void updateCondLabel();

    // UI widgets
    QListWidget* m_pointList = nullptr;    ///< Displays the list of click points
    QPushButton* m_addBtn = nullptr;       ///< Button to add a new point
    QPushButton* m_removeBtn = nullptr;    ///< Button to remove selected point
    QPushButton* m_editBtn = nullptr;      ///< Button to edit selected point
    QPushButton* m_clearBtn = nullptr;     ///< Button to clear all points
    QPushButton* m_backBtn = nullptr;      ///< Button to return to home
    QPushButton* m_upBtn = nullptr;        ///< Button to move point up in sequence
    QPushButton* m_downBtn = nullptr;      ///< Button to move point down in sequence
    QPushButton* m_startBtn = nullptr;     ///< Button to start/stop automation
    QPushButton* m_saveBtn = nullptr;      ///< Button to save configuration
    QPushButton* m_settingsBtn = nullptr;  ///< Button to open settings dialog
    QLabel* m_statusLbl = nullptr;         ///< Displays current status messages
    QLabel* m_hotkeyLbl = nullptr;         ///< Displays configured hotkey
    QLabel* m_condLbl = nullptr;           ///< Displays stop condition settings

    // Core objects
    OverlayWindow* m_overlay = nullptr;    ///< Visual overlay for point capture
    ClickerEngine* m_engine = nullptr;     ///< Automation execution engine
    AppConfig m_config;                    ///< Current automation configuration

    bool m_running = false;                ///< Is automation currently running
    bool m_isProMode = false;              ///< Is window in Pro mode

    /// Gets the appropriate config file path (pro or standard)
    QString configFilePath() const {
        return m_isProMode ? ConfigManager::defaultProConfigPath() : ConfigManager::defaultConfigPath();
    }

    // Windows hotkey IDs
    static constexpr int kHotkeyToggleId = 9001;  ///< Unique ID for the global hotkey
};
