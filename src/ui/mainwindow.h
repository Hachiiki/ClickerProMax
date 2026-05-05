#pragma once
#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "core/clickpoint.h"
#include "core/configmanager.h"
#include "engine/clickerengine.h"
#include "overlaywindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

signals:
    void backRequested();

protected:
    // Used to intercept WM_HOTKEY on Windows
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void closeEvent(QCloseEvent*) override;
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;

private slots:
    // Toolbar / button actions
    void onAddPoint();
    void onRemovePoint();
    void onEditPoint();
    void onClearAll();
    void onMoveUp();
    void onMoveDown();
    void onStartStop();
    void onSaveConfig();
    void onOpenSettings();

    // Overlay callbacks
    void onPointCaptured(QPoint screenPos);
    void onCaptureAborted();

    // Engine callbacks
    void onEngineStarted();
    void onEngineStopped(const QString& reason);
    void onCurrentPointChanged(int index);
    void onStatusMessage(const QString& msg);

private:
    // ── Setup ──────────────────────────────────────────────────────────────────
    void buildUI();
    void connectSignals();

    // ── Point list helpers ─────────────────────────────────────────────────────
    void refreshPointList();
    void pushPointsToEngine();

    // ── Hotkey (Windows RegisterHotKey) ────────────────────────────────────────
    void registerHotkey();
    void unregisterHotkey();

    // ── Misc ───────────────────────────────────────────────────────────────────
    void setRunning(bool running);
    void updateHotkeyLabel();
    void updateCondLabel();

    // ── UI widgets ─────────────────────────────────────────────────────────────
    QListWidget* m_pointList   = nullptr;
    QPushButton* m_addBtn      = nullptr;
    QPushButton* m_removeBtn   = nullptr;
    QPushButton* m_editBtn     = nullptr;
    QPushButton* m_clearBtn    = nullptr;
    QPushButton* m_upBtn       = nullptr;
    QPushButton* m_downBtn     = nullptr;
    QPushButton* m_startBtn    = nullptr;
    QPushButton* m_saveBtn     = nullptr;
    QPushButton* m_settingsBtn = nullptr;
    QLabel*      m_statusLbl   = nullptr;
    QLabel*      m_hotkeyLbl   = nullptr;
    QLabel*      m_condLbl     = nullptr;

    // ── Core objects ────────────────────────────────────────────────────────────
    OverlayWindow* m_overlay = nullptr;
    ClickerEngine* m_engine  = nullptr;
    AppConfig      m_config;

    bool m_running = false;

    // Windows hotkey IDs
    static constexpr int kHotkeyToggleId = 9001;
};
