#include "mainwindow.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QSpinBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QKeySequenceEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QCloseEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QScreen>
#include <QFont>
#include <QDebug>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

#include "util/hotkey_utils.h"

// ─────────────────────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(bool isProMode, QWidget* parent)
    : QMainWindow(parent), m_isProMode(isProMode)
{
    setWindowTitle(m_isProMode ? "Multi-Point Pro AutoClicker" : "AutoClicker");
    setMinimumSize(420, 520);

    // Load saved config (ignore failure – defaults are already set)
    ConfigManager::load(m_config, configFilePath());

    buildUI();
    connectSignals();

    m_overlay = new OverlayWindow(nullptr);
    m_engine  = new ClickerEngine(this);

    connect(m_overlay, &OverlayWindow::pointCaptured,  this, &MainWindow::onPointCaptured);
    connect(m_overlay, &OverlayWindow::captureAborted, this, &MainWindow::onCaptureAborted);
    connect(m_engine, &ClickerEngine::started,             this, &MainWindow::onEngineStarted);
    connect(m_engine, &ClickerEngine::stopped,             this, &MainWindow::onEngineStopped);
    connect(m_engine, &ClickerEngine::currentPointChanged, this, &MainWindow::onCurrentPointChanged);
    connect(m_engine, &ClickerEngine::statusMessage,       this, &MainWindow::onStatusMessage);

    refreshPointList();

    // Show stop-condition summary
    onStatusMessage("Ready.");
}

MainWindow::~MainWindow()
{
    unregisterHotkey();
    delete m_overlay;
}

// ─────────────────────────────────────────────────────────────────────────────
// UI construction
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::buildUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* root = new QVBoxLayout(central);
    root->setSpacing(8);
    root->setContentsMargins(10, 10, 10, 10);

    // ── Header labels ────────────────────────────────────────────────────────
    {
        QFont titleFont = font();
        titleFont.setPointSize(titleFont.pointSize() + 2);
        titleFont.setBold(true);

        m_backBtn = new QPushButton("← Back", this);
        connect(m_backBtn, &QPushButton::clicked, this, &MainWindow::backRequested);

        QLabel* title = new QLabel(m_isProMode ? "Multi-Point Pro" : "AutoClicker", this);
        title->setFont(titleFont);

        m_hotkeyLbl = new QLabel(this);
        m_condLbl   = new QLabel(this);

        QHBoxLayout* headerRow = new QHBoxLayout;
        headerRow->addWidget(m_backBtn);
        headerRow->addWidget(title);
        headerRow->addStretch();
        headerRow->addWidget(m_condLbl);
        root->addLayout(headerRow);
        root->addWidget(m_hotkeyLbl);
    }

    // ── Point list ───────────────────────────────────────────────────────────
    {
        QGroupBox* grp = new QGroupBox("Click Points (click circles on screen)", this);
        QVBoxLayout* gl = new QVBoxLayout(grp);

        m_pointList = new QListWidget(this);
        m_pointList->setAlternatingRowColors(true);
        m_pointList->setSelectionMode(QAbstractItemView::SingleSelection);
        gl->addWidget(m_pointList);

        // Side-button column
        QVBoxLayout* btns = new QVBoxLayout;

        auto makeBtn = [&](const QString& text) -> QPushButton* {
            auto* b = new QPushButton(text, this);
            b->setFixedWidth(90);
            btns->addWidget(b);
            return b;
        };

        m_addBtn    = makeBtn("➕ Add");
        m_removeBtn = makeBtn("➖ Remove");
        m_editBtn   = makeBtn("✏ Edit");
        m_upBtn     = makeBtn("▲ Up");
        m_downBtn   = makeBtn("▼ Down");
        m_clearBtn  = makeBtn("🗑 Clear All");
        btns->addStretch();

        QHBoxLayout* listRow = new QHBoxLayout;
        listRow->addWidget(m_pointList);
        listRow->addLayout(btns);
        gl->addLayout(listRow);

        root->addWidget(grp);
    }

    // ── Control bar ──────────────────────────────────────────────────────────
    {
        QHBoxLayout* ctrl = new QHBoxLayout;

        m_startBtn = new QPushButton("▶  Start", this);
        m_startBtn->setMinimumHeight(38);
        QFont f = m_startBtn->font();
        f.setBold(true);
        m_startBtn->setFont(f);

        m_saveBtn = new QPushButton("💾  Save", this);
        m_saveBtn->setMinimumHeight(38);

        m_settingsBtn = new QPushButton("⚙  Settings", this);
        m_settingsBtn->setMinimumHeight(38);

        ctrl->addWidget(m_startBtn, 2);
        ctrl->addWidget(m_saveBtn, 1);
        ctrl->addWidget(m_settingsBtn, 1);
        root->addLayout(ctrl);
    }

    // ── Status bar ───────────────────────────────────────────────────────────
    m_statusLbl = new QLabel("Ready.", this);
    m_statusLbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_statusLbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_statusLbl->setMinimumHeight(24);
    root->addWidget(m_statusLbl);

    updateHotkeyLabel();
    updateCondLabel();
}

void MainWindow::updateHotkeyLabel()
{
    m_hotkeyLbl->setText(
        QString("Toggle hotkey:  <b>%1</b>").arg(m_config.toggleHotkey.toString()));
}

void MainWindow::updateCondLabel()
{
    QString text;
    switch (m_config.stopCondition) {
    case StopCondition::Indefinite:
        text = "Stop: indefinitely";
        break;
    case StopCondition::TimeLimit:
        text = QString("Stop: after %1 s").arg(m_config.timeLimitSecs);
        break;
    case StopCondition::CycleCount:
        text = QString("Stop: after %1 cycle(s)").arg(m_config.maxCycles);
        break;
    }
    m_condLbl->setText(text);
}

// ─────────────────────────────────────────────────────────────────────────────
// Signal wiring
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::connectSignals()
{
    connect(m_addBtn,      &QPushButton::clicked, this, &MainWindow::onAddPoint);
    connect(m_removeBtn,   &QPushButton::clicked, this, &MainWindow::onRemovePoint);
    connect(m_editBtn,     &QPushButton::clicked, this, &MainWindow::onEditPoint);
    connect(m_clearBtn,    &QPushButton::clicked, this, &MainWindow::onClearAll);
    connect(m_upBtn,       &QPushButton::clicked, this, &MainWindow::onMoveUp);
    connect(m_downBtn,     &QPushButton::clicked, this, &MainWindow::onMoveDown);
    connect(m_startBtn,    &QPushButton::clicked, this, &MainWindow::onStartStop);
    connect(m_saveBtn,     &QPushButton::clicked, this, &MainWindow::onSaveConfig);
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainWindow::onOpenSettings);

    // Double-click to edit
    connect(m_pointList, &QListWidget::itemDoubleClicked,
            this, [this](QListWidgetItem*){ onEditPoint(); });
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots – point management
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onAddPoint()
{
    if (m_running) return;
    m_statusLbl->setText("Click anywhere on screen to place a point…");
    m_overlay->enterCaptureMode();
}

void MainWindow::onPointCaptured(QPoint pos)
{
    m_config.points.append(ClickPoint(pos, 300));
    refreshPointList();
    m_statusLbl->setText(QString("Point %1 added at (%2, %3).")
                         .arg(m_config.points.size()).arg(pos.x()).arg(pos.y()));
}

void MainWindow::onCaptureAborted()
{
    m_statusLbl->setText("Point capture cancelled.");
}

void MainWindow::onRemovePoint()
{
    const int row = m_pointList->currentRow();
    if (row < 0 || row >= m_config.points.size()) return;
    m_config.points.removeAt(row);
    refreshPointList();
}

void MainWindow::onEditPoint()
{
    const int row = m_pointList->currentRow();
    if (row < 0 || row >= m_config.points.size()) return;

    ClickPoint& pt = m_config.points[row];

    // ── Inline edit dialog ──────────────────────────────────────────────────
    QDialog dlg(this);
    dlg.setWindowTitle(QString("Edit Point %1").arg(row + 1));
    dlg.setFixedSize(300, 160);

    QFormLayout* form = new QFormLayout(&dlg);

    QLabel* posLabel = new QLabel(
        QString("(%1, %2)").arg(pt.position.x()).arg(pt.position.y()), &dlg);

    QSpinBox* delaySpin = new QSpinBox(&dlg);
    delaySpin->setRange(0, 60000);
    delaySpin->setValue(pt.delayMs);
    delaySpin->setSuffix(" ms");
    delaySpin->setToolTip("Delay AFTER clicking this point before clicking the next one.");

    QLineEdit* labelEdit = new QLineEdit(pt.label, &dlg);
    labelEdit->setPlaceholderText("optional label");

    QLineEdit* keysEdit = nullptr;
    QSpinBox* keyDelaySpin = nullptr;
    if (m_isProMode) {
        keysEdit = new QLineEdit(pt.actionKeys, &dlg);
        keysEdit->setPlaceholderText("e.g. A, B, Space");

        keyDelaySpin = new QSpinBox(&dlg);
        keyDelaySpin->setRange(500, 60000);
        keyDelaySpin->setValue(pt.keyDelaySecs * 1000);
        keyDelaySpin->setSuffix(" ms");
        keyDelaySpin->setToolTip("Delay between individual key presses.");
    }

    form->addRow("Position:", posLabel);
    form->addRow("Delay after click:", delaySpin);
    form->addRow("Label:", labelEdit);
    
    if (m_isProMode) {
        form->addRow("Macro Keys:", keysEdit);
        form->addRow("Key Delay:", keyDelaySpin);
        dlg.setFixedSize(300, 220); // slightly larger if we show keys
    } else {
        dlg.setFixedSize(300, 160);
    }

    QDialogButtonBox* bb =
        new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(bb);

    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        pt.delayMs = delaySpin->value();
        pt.label   = labelEdit->text();
        if (m_isProMode && keysEdit && keyDelaySpin) {
            pt.actionKeys = keysEdit->text();
            pt.keyDelaySecs = keyDelaySpin->value() / 1000.0;
        }
        refreshPointList();
    }
}

void MainWindow::onClearAll()
{
    if (m_config.points.isEmpty()) return;
    if (QMessageBox::question(this, "Clear All", "Remove all points?")
        != QMessageBox::Yes) return;
    m_config.points.clear();
    refreshPointList();
}

void MainWindow::onMoveUp()
{
    const int row = m_pointList->currentRow();
    if (row <= 0 || row >= m_config.points.size()) return;
    m_config.points.swapItemsAt(row, row - 1);
    refreshPointList();
    m_pointList->setCurrentRow(row - 1);
}

void MainWindow::onMoveDown()
{
    const int row = m_pointList->currentRow();
    if (row < 0 || row >= m_config.points.size() - 1) return;
    m_config.points.swapItemsAt(row, row + 1);
    refreshPointList();
    m_pointList->setCurrentRow(row + 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots – engine
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onStartStop()
{
    if (!m_running) {
        if (m_config.points.isEmpty()) {
            QMessageBox::information(this, "No Points",
                "Add at least one click point before starting.");
            return;
        }
        pushPointsToEngine();
        m_engine->setStopCondition(m_config.stopCondition,
                                   m_config.stopCondition == StopCondition::TimeLimit
                                       ? m_config.timeLimitSecs
                                       : m_config.maxCycles);
        m_engine->start();
    } else {
        m_engine->stop();
    }
}

void MainWindow::onEngineStarted()
{
    setRunning(true);
    if (m_config.hideOverlayWhenRunning)
        m_overlay->hide();
}

void MainWindow::onEngineStopped(const QString& reason)
{
    setRunning(false);
    m_overlay->setActivePoint(-1);
    m_overlay->show();   // always restore, harmless if already visible
    qDebug() << "Engine stopped:" << reason;
}

void MainWindow::onCurrentPointChanged(int index)
{
    m_overlay->setActivePoint(index);
    // Highlight the row too
    m_pointList->setCurrentRow(index);
}

void MainWindow::onStatusMessage(const QString& msg)
{
    m_statusLbl->setText(msg);
}

// ─────────────────────────────────────────────────────────────────────────────
// Settings dialog
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onOpenSettings()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Settings");
    dlg.setMinimumWidth(360);

    QVBoxLayout* vl = new QVBoxLayout(&dlg);

    // ── Stop condition ───────────────────────────────────────────────────────
    QGroupBox* stopGrp = new QGroupBox("Stop Condition", &dlg);
    QVBoxLayout* stopVl = new QVBoxLayout(stopGrp);
    QButtonGroup* stopBG = new QButtonGroup(&dlg);

    QRadioButton* rbInfinite = new QRadioButton("Run indefinitely", &dlg);
    QRadioButton* rbTime     = new QRadioButton("Stop after (seconds):", &dlg);
    QRadioButton* rbCycles   = new QRadioButton("Stop after (cycles):", &dlg);

    QSpinBox* timeSpin  = new QSpinBox(&dlg);
    timeSpin->setRange(1, 86400);
    timeSpin->setValue(m_config.timeLimitSecs);
    timeSpin->setSuffix(" s");

    QSpinBox* cycleSpin = new QSpinBox(&dlg);
    cycleSpin->setRange(1, 1000000);
    cycleSpin->setValue(m_config.maxCycles);
    cycleSpin->setSuffix(" cycle(s)");

    stopBG->addButton(rbInfinite, 0);
    stopBG->addButton(rbTime,     1);
    stopBG->addButton(rbCycles,   2);

    // Inline rows
    auto makeSpinRow = [&](QRadioButton* rb, QSpinBox* sb) {
        QHBoxLayout* row = new QHBoxLayout;
        row->addWidget(rb);
        row->addWidget(sb);
        return row;
    };

    stopVl->addWidget(rbInfinite);
    stopVl->addLayout(makeSpinRow(rbTime,   timeSpin));
    stopVl->addLayout(makeSpinRow(rbCycles, cycleSpin));
    vl->addWidget(stopGrp);

    // Set current selection
    switch (m_config.stopCondition) {
    case StopCondition::Indefinite:  rbInfinite->setChecked(true); break;
    case StopCondition::TimeLimit:   rbTime->setChecked(true);     break;
    case StopCondition::CycleCount:  rbCycles->setChecked(true);   break;
    }

    // Enable/disable spinboxes based on selection
    auto syncSpins = [&]() {
        timeSpin->setEnabled(rbTime->isChecked());
        cycleSpin->setEnabled(rbCycles->isChecked());
    };
    syncSpins();
    connect(rbInfinite, &QRadioButton::toggled, this, [syncSpins](bool){ syncSpins(); });
    connect(rbTime,     &QRadioButton::toggled, this, [syncSpins](bool){ syncSpins(); });
    connect(rbCycles,   &QRadioButton::toggled, this, [syncSpins](bool){ syncSpins(); });

    // ── Overlay ──────────────────────────────────────────────────────────────
    QGroupBox* overlayGrp = new QGroupBox("Overlay", &dlg);
    QVBoxLayout* overlayVl = new QVBoxLayout(overlayGrp);
    QCheckBox* hideOverlayCb = new QCheckBox(
        "Hide circles while clicking  (recommended for Roblox / games)", &dlg);
    hideOverlayCb->setChecked(m_config.hideOverlayWhenRunning);
    hideOverlayCb->setToolTip(
        "Hides the numbered circle overlay the moment clicking starts.\n"
        "The overlay reappears when stopped.\n"
        "Use this if the game sees the overlay as a target and misroutes clicks.");
    overlayVl->addWidget(hideOverlayCb);
    vl->addWidget(overlayGrp);

    // ── Hotkey ───────────────────────────────────────────────────────────────
    QGroupBox* hkGrp = new QGroupBox("Toggle Hotkey  (Start / Stop)", &dlg);
    QFormLayout* hkForm = new QFormLayout(hkGrp);

    QKeySequenceEdit* hkEdit = new QKeySequenceEdit(&dlg);
    hkEdit->setKeySequence(QKeySequence(m_config.toggleHotkey.modifiers
                                        | m_config.toggleHotkey.key));
    hkEdit->setToolTip(
        "Press a key or key combination.\n"
        "Supported: F1–F24, letters, digits, with optional Ctrl/Alt/Shift.\n"
        "Default: F6");

    hkForm->addRow("Key:", hkEdit);
    vl->addWidget(hkGrp);

    // ── Buttons ──────────────────────────────────────────────────────────────
    QDialogButtonBox* bb =
        new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    vl->addWidget(bb);

    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() != QDialog::Accepted) return;

    // ── Apply ─────────────────────────────────────────────────────────────────
    if (rbInfinite->isChecked())  m_config.stopCondition = StopCondition::Indefinite;
    else if (rbTime->isChecked()) m_config.stopCondition = StopCondition::TimeLimit;
    else                          m_config.stopCondition = StopCondition::CycleCount;

    m_config.timeLimitSecs = timeSpin->value();
    m_config.maxCycles     = cycleSpin->value();
    m_config.hideOverlayWhenRunning = hideOverlayCb->isChecked();

    // Parse the key sequence
    QKeySequence ks = hkEdit->keySequence();
    if (!ks.isEmpty()) {
        int combined = ks[0]
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            .toCombined();
#else
            ;
#endif
        m_config.toggleHotkey.key       = combined & ~Qt::KeyboardModifierMask;
        m_config.toggleHotkey.modifiers = combined &  Qt::KeyboardModifierMask;
    }

    unregisterHotkey();
    registerHotkey();
    updateHotkeyLabel();
    updateCondLabel();
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::refreshPointList()
{
    m_pointList->clear();
    for (int i = 0; i < m_config.points.size(); ++i) {
        const auto& pt = m_config.points[i];
        QString text = QString("[%1]  (%2, %3)   delay: %4 ms")
                       .arg(i + 1)
                       .arg(pt.position.x())
                       .arg(pt.position.y())
                       .arg(pt.delayMs);
        if (m_isProMode && !pt.actionKeys.isEmpty()) {
            text += QString("  [Keys: %1]").arg(pt.actionKeys);
        }
        if (!pt.label.isEmpty())
            text += QString("   \"%1\"").arg(pt.label);
        m_pointList->addItem(text);
    }
    m_overlay->setPoints(m_config.points);
}

void MainWindow::pushPointsToEngine()
{
    m_engine->setPoints(m_config.points);
}

void MainWindow::setRunning(bool running)
{
    m_running = running;
    m_startBtn->setText(running ? "⏹  Stop" : "▶  Start");
    if (m_backBtn) m_backBtn->setEnabled(!running);
    m_addBtn->setEnabled(!running);
    m_removeBtn->setEnabled(!running);
    m_editBtn->setEnabled(!running);
    m_clearBtn->setEnabled(!running);
    m_upBtn->setEnabled(!running);
    m_downBtn->setEnabled(!running);
    m_saveBtn->setEnabled(!running);
    m_settingsBtn->setEnabled(!running);
    if (!running)
        m_overlay->setActivePoint(-1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Global hotkey (Windows)
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::registerHotkey()
{
#ifdef Q_OS_WIN
    UINT vk = 0, mods = 0;
    if (!qtKeyToWin(m_config.toggleHotkey.key,
                    m_config.toggleHotkey.modifiers,
                    vk, mods))
    {
        qWarning() << "Unsupported hotkey – falling back to F6";
        vk   = VK_F6;
        mods = 0;
    }
    if (!RegisterHotKey(reinterpret_cast<HWND>(winId()), kHotkeyToggleId, mods, vk))
        qWarning() << "RegisterHotKey failed for toggle";
#endif
}

void MainWindow::unregisterHotkey()
{
#ifdef Q_OS_WIN
    UnregisterHotKey(reinterpret_cast<HWND>(winId()), kHotkeyToggleId);
#endif
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY) {
            if (msg->wParam == kHotkeyToggleId) {
                onStartStop();
                *result = 0;
                return true;
            }
        }
    }
#else
    Q_UNUSED(eventType); Q_UNUSED(message); Q_UNUSED(result);
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::onSaveConfig()
{
    if (ConfigManager::save(m_config, configFilePath()))
        m_statusLbl->setText("Configuration saved.");
    else
        m_statusLbl->setText("Failed to save configuration!");
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_running) m_engine->stop();
    unregisterHotkey();
    m_overlay->hide();
    event->accept();
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    if (!this->isMinimized()) {
        registerHotkey();
    }
    m_overlay->show();
}

void MainWindow::hideEvent(QHideEvent* event)
{
    QMainWindow::hideEvent(event);
    if (!this->isMinimized()) {
        unregisterHotkey();
    }
    m_overlay->hide();
}
