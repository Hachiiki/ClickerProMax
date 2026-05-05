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

class NormalClickWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit NormalClickWindow(QWidget* parent = nullptr);
    ~NormalClickWindow() override;

signals:
    void backRequested();

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void closeEvent(QCloseEvent*) override;

private slots:
    void onStartStop();
    void onSaveConfig();
    void onOpenSettings();
    void onPickPosition();
    void onPositionPicked(QPoint pos);
    void onCaptureAborted();

    void onEngineStarted();
    void onEngineStopped(const QString& reason);
    void onStatusMessage(const QString& msg);

private:
    void buildUI();
    void connectSignals();
    void updateHotkeyLabels();
    void setRunning(bool running);

    void saveUItoConfig();
    void loadConfigtoUI();

    void registerHotkey();
    void unregisterHotkey();

    NormalAppConfig m_config;
    NormalClickerEngine* m_engine = nullptr;
    OverlayWindow* m_overlay = nullptr;
    bool m_running = false;
    static constexpr int kHotkeyToggleId = 9002;

    // UI elements
    QSpinBox* m_hrsSpin;
    QSpinBox* m_minsSpin;
    QSpinBox* m_secsSpin;
    QSpinBox* m_msSpin;

    QComboBox* m_mouseBtnCombo;
    QComboBox* m_clickTypeCombo;

    QRadioButton* m_repTimesRadio;
    QRadioButton* m_repUntilRadio;
    QSpinBox* m_repCountSpin;

    QRadioButton* m_posCurrentRadio;
    QRadioButton* m_posPickedRadio;
    QPushButton* m_pickBtn;
    QLabel* m_coordsLbl;

    QPushButton* m_startBtn;
    QPushButton* m_stopBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_settingsBtn;
    QLabel* m_statusLbl;
};
