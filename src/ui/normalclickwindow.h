#pragma once
#include <QMainWindow>
#include <QSpinBox>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include "core/configmanager.h"
#include "engine/normalclickerengine.h"
#include "overlaywindow.h"

/**
 * @class NormalClickWindow
 * @brief UI for simple timed clicking at a single location
 *
 * Provides a simplified interface for users who want to click repeatedly
 * at a fixed interval without managing multiple points. Features include:
 *   - Configurable timing (hours, minutes, seconds, milliseconds)
 *   - Button type selection (left, right, middle)
 *   - Click type (single or double-click)
 *   - Repeat mode (until stopped or N times)
 *   - Position mode (current cursor or pre-selected location)
 *   - Global hotkey to start/stop
 *   - Configuration persistence
 *
 * Uses OverlayWindow for visual position selection and NormalClickerEngine
 * for the actual clicking execution.
 */
class NormalClickWindow : public QMainWindow
{
    Q_OBJECT
public:
    /// Constructs the Normal Click window
    explicit NormalClickWindow(QWidget* parent = nullptr);
    ~NormalClickWindow() override;

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
    /// User clicked "Start/Stop" button or pressed hotkey - toggles clicking
    void onStartStop();
    /// User clicked "Save" button - persists configuration to JSON
    void onSaveConfig();
    /// User clicked "Settings" button - opens configuration dialog
    void onOpenSettings();
    /// User clicked "Pick Position" button - enters overlay capture mode
    void onPickPosition();
    /// Overlay emitted pointCaptured signal - saves selected position
    void onPositionPicked(QPoint pos);
    /// User pressed Escape during capture - aborts position selection
    void onCaptureAborted();

    /// Engine started running - updates UI to reflect running state
    void onEngineStarted();
    /// Engine stopped - updates UI with stop reason
    void onEngineStopped(const QString& reason);
    /// Engine emitted status message - displays feedback to user
    void onStatusMessage(const QString& msg);

private:
    /// Constructs the UI layout with controls for timing and behavior
    void buildUI();
    /// Connects signals from UI widgets, overlay, and engine to their handlers
    void connectSignals();
    /// Refreshes the hotkey display label
    void updateHotkeyLabels();
    /// Updates UI to show running/idle state
    void setRunning(bool running);

    /// Reads UI control values and updates m_config
    void saveUItoConfig();
    /// Populates UI controls from m_config values
    void loadConfigtoUI();

    /// Registers the global hotkey with Windows
    void registerHotkey();
    /// Unregisters the global hotkey with Windows
    void unregisterHotkey();

    NormalAppConfig m_config;          ///< Current clicking configuration
    NormalClickerEngine* m_engine = nullptr;  ///< Clicking execution engine
    OverlayWindow* m_overlay = nullptr;       ///< Visual overlay for position selection
    bool m_running = false;                   ///< Is clicking currently running
    static constexpr int kHotkeyToggleId = 9002;  ///< Unique ID for the global hotkey

    // UI elements - Timing controls
    QPushButton* m_backBtn = nullptr;
    QSpinBox* m_hrsSpin = nullptr;     ///< Hours input
    QSpinBox* m_minsSpin = nullptr;    ///< Minutes input
    QSpinBox* m_secsSpin = nullptr;    ///< Seconds input
    QSpinBox* m_msSpin = nullptr;      ///< Milliseconds input

    // UI elements - Click behavior
    QComboBox* m_mouseBtnCombo = nullptr;   ///< Button type selector
    QComboBox* m_clickTypeCombo = nullptr;  ///< Single/double-click selector

    // UI elements - Repeat mode
    QRadioButton* m_repTimesRadio = nullptr;  ///< Repeat N times option
    QRadioButton* m_repUntilRadio = nullptr;  ///< Repeat until stopped option
    QSpinBox* m_repCountSpin = nullptr;       ///< Repeat count input

    // UI elements - Position selection
    QRadioButton* m_posCurrentRadio = nullptr;  ///< Use current cursor position option
    QRadioButton* m_posPickedRadio = nullptr;   ///< Use pre-selected position option
    QPushButton* m_pickBtn = nullptr;           ///< Button to start position picker
    QLabel* m_coordsLbl = nullptr;              ///< Displays selected coordinates

    // UI elements - Control buttons
    QPushButton* m_startBtn = nullptr;   ///< Start clicking button
    QPushButton* m_stopBtn = nullptr;    ///< Stop clicking button
    QPushButton* m_saveBtn = nullptr;    ///< Save configuration button
    QPushButton* m_settingsBtn = nullptr;  ///< Settings button
    QLabel* m_statusLbl;
};
