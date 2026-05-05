#include "normalclickwindow.h"
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QKeySequenceEdit>
#include <QMessageBox>
#include <QCloseEvent>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

#include "util/hotkey_utils.h"

NormalClickWindow::NormalClickWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("AutoClicker");
    setMinimumSize(420, 520);

    ConfigManager::loadNormal(m_config);

    m_engine = new NormalClickerEngine(this);
    m_overlay = new OverlayWindow(nullptr);
    // Since normal clicks use only 1 point when picking, we don't display multipoints.
    
    connect(m_engine, &NormalClickerEngine::started, this, &NormalClickWindow::onEngineStarted);
    connect(m_engine, &NormalClickerEngine::stopped, this, &NormalClickWindow::onEngineStopped);
    connect(m_engine, &NormalClickerEngine::statusMessage, this, &NormalClickWindow::onStatusMessage);

    m_overlay->hide();  // only used for picking, not for displaying points
    connect(m_overlay, &OverlayWindow::pointCaptured, this, &NormalClickWindow::onPositionPicked);
    connect(m_overlay, &OverlayWindow::captureAborted, this, &NormalClickWindow::onCaptureAborted);

    buildUI();
    connectSignals();
    loadConfigtoUI();

    registerHotkey();
    onStatusMessage("Ready.");
}

NormalClickWindow::~NormalClickWindow()
{
    unregisterHotkey();
    delete m_overlay;
}

void NormalClickWindow::buildUI()
{
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout* root = new QVBoxLayout(central);
    root->setSpacing(8);
    root->setContentsMargins(10, 10, 10, 10);

    // Header
    {
        QPushButton* backBtn = new QPushButton("← Back", this);
        connect(backBtn, &QPushButton::clicked, this, &NormalClickWindow::backRequested);
        
        QLabel* title = new QLabel("Normal AutoClicker", this);
        QFont tf = title->font();
        tf.setPointSize(tf.pointSize() + 2);
        tf.setBold(true);
        title->setFont(tf);

        QHBoxLayout* hl = new QHBoxLayout;
        hl->addWidget(backBtn);
        hl->addWidget(title);
        hl->addStretch();
        root->addLayout(hl);
    }

    // Interval Group
    {
        QGroupBox* grp = new QGroupBox("Click interval", this);
        QHBoxLayout* hl = new QHBoxLayout(grp);
        m_hrsSpin = new QSpinBox(this); m_hrsSpin->setRange(0, 99); m_hrsSpin->setSuffix(" hrs");
        m_minsSpin = new QSpinBox(this); m_minsSpin->setRange(0, 59); m_minsSpin->setSuffix(" mins");
        m_secsSpin = new QSpinBox(this); m_secsSpin->setRange(0, 59); m_secsSpin->setSuffix(" secs");
        m_msSpin = new QSpinBox(this); m_msSpin->setRange(0, 999); m_msSpin->setSuffix(" ms");
        hl->addWidget(m_hrsSpin);
        hl->addWidget(m_minsSpin);
        hl->addWidget(m_secsSpin);
        hl->addWidget(m_msSpin);
        root->addWidget(grp);
    }

    // Options Group
    {
        QGroupBox* grp = new QGroupBox("Click options", this);
        QFormLayout* fl = new QFormLayout(grp);
        m_mouseBtnCombo = new QComboBox(this);
        m_mouseBtnCombo->addItems({"Left", "Middle", "Right"});
        m_clickTypeCombo = new QComboBox(this);
        m_clickTypeCombo->addItems({"Single", "Double"});
        fl->addRow("Mouse button:", m_mouseBtnCombo);
        fl->addRow("Click type:", m_clickTypeCombo);
        root->addWidget(grp);
    }

    // Repeat Group
    {
        QGroupBox* grp = new QGroupBox("Click repeat", this);
        QVBoxLayout* vl = new QVBoxLayout(grp);
        
        QHBoxLayout* r1 = new QHBoxLayout;
        m_repTimesRadio = new QRadioButton("Repeat", this);
        m_repCountSpin = new QSpinBox(this);
        m_repCountSpin->setRange(1, 999999);
        m_repCountSpin->setSuffix(" times");
        r1->addWidget(m_repTimesRadio);
        r1->addWidget(m_repCountSpin);
        r1->addStretch();
        
        m_repUntilRadio = new QRadioButton("Repeat until stopped", this);
        vl->addLayout(r1);
        vl->addWidget(m_repUntilRadio);
        root->addWidget(grp);
    }

    // Position Group
    {
        QGroupBox* grp = new QGroupBox("Cursor position", this);
        QVBoxLayout* vl = new QVBoxLayout(grp);
        m_posCurrentRadio = new QRadioButton("Current location", this);
        
        QHBoxLayout* r2 = new QHBoxLayout;
        m_posPickedRadio = new QRadioButton("Pick position", this);
        m_pickBtn = new QPushButton("Pick", this);
        m_coordsLbl = new QLabel("(X, Y)", this);
        r2->addWidget(m_posPickedRadio);
        r2->addWidget(m_pickBtn);
        r2->addWidget(m_coordsLbl);
        r2->addStretch();
        
        vl->addWidget(m_posCurrentRadio);
        vl->addLayout(r2);
        root->addWidget(grp);
    }

    // Control Bar
    {
        QHBoxLayout* ctrl = new QHBoxLayout;
        m_startBtn = new QPushButton("Start", this);
        m_startBtn->setMinimumHeight(38);
        m_stopBtn = new QPushButton("Stop", this);
        m_stopBtn->setMinimumHeight(38);
        m_saveBtn = new QPushButton("💾 Save", this);
        m_saveBtn->setMinimumHeight(38);
        m_settingsBtn = new QPushButton("⚙ Settings", this);
        m_settingsBtn->setMinimumHeight(38);
        ctrl->addWidget(m_startBtn, 2);
        ctrl->addWidget(m_stopBtn, 2);
        ctrl->addWidget(m_saveBtn, 1);
        ctrl->addWidget(m_settingsBtn, 1);
        root->addLayout(ctrl);
    }

    m_statusLbl = new QLabel("Ready.", this);
    m_statusLbl->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    m_statusLbl->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_statusLbl->setMinimumHeight(24);
    root->addWidget(m_statusLbl);
    
    updateHotkeyLabels();
}

void NormalClickWindow::connectSignals()
{
    connect(m_startBtn, &QPushButton::clicked, this, &NormalClickWindow::onStartStop);
    connect(m_stopBtn, &QPushButton::clicked, this, &NormalClickWindow::onStartStop);
    connect(m_saveBtn, &QPushButton::clicked, this, &NormalClickWindow::onSaveConfig);
    connect(m_settingsBtn, &QPushButton::clicked, this, &NormalClickWindow::onOpenSettings);
    connect(m_pickBtn, &QPushButton::clicked, this, &NormalClickWindow::onPickPosition);
    
    // Toggle spinbox/btn states on radio checked
    auto syncRep = [&]() {
        m_repCountSpin->setEnabled(m_repTimesRadio->isChecked());
    };
    connect(m_repTimesRadio, &QRadioButton::toggled, this, syncRep);
    connect(m_repUntilRadio, &QRadioButton::toggled, this, syncRep);

    auto syncPos = [&]() {
        m_pickBtn->setEnabled(m_posPickedRadio->isChecked());
    };
    connect(m_posCurrentRadio, &QRadioButton::toggled, this, syncPos);
    connect(m_posPickedRadio, &QRadioButton::toggled, this, syncPos);
}

void NormalClickWindow::updateHotkeyLabels()
{
    QString hkStr = m_config.toggleHotkey.toString();
    m_startBtn->setText(QString("Start (%1)").arg(hkStr));
    m_stopBtn->setText(QString("Stop (%1)").arg(hkStr));
}

void NormalClickWindow::saveUItoConfig()
{
    m_config.hours = m_hrsSpin->value();
    m_config.mins  = m_minsSpin->value();
    m_config.secs  = m_secsSpin->value();
    m_config.ms    = m_msSpin->value();

    m_config.buttonType = static_cast<MouseButtonType>(m_mouseBtnCombo->currentIndex());
    m_config.clickType  = static_cast<ClickType>(m_clickTypeCombo->currentIndex());

    m_config.repeatUntilStopped = m_repUntilRadio->isChecked();
    m_config.repeatTimes        = m_repCountSpin->value();

    m_config.positionType = m_posCurrentRadio->isChecked() ? CursorPositionType::Current : CursorPositionType::Picked;
    // pickedX and pickedY are updated on picking logic.
}

void NormalClickWindow::loadConfigtoUI()
{
    m_hrsSpin->setValue(m_config.hours);
    m_minsSpin->setValue(m_config.mins);
    m_secsSpin->setValue(m_config.secs);
    m_msSpin->setValue(m_config.ms);

    m_mouseBtnCombo->setCurrentIndex(static_cast<int>(m_config.buttonType));
    m_clickTypeCombo->setCurrentIndex(static_cast<int>(m_config.clickType));

    if (m_config.repeatUntilStopped) m_repUntilRadio->setChecked(true);
    else m_repTimesRadio->setChecked(true);
    m_repCountSpin->setValue(m_config.repeatTimes);

    if (m_config.positionType == CursorPositionType::Current) m_posCurrentRadio->setChecked(true);
    else m_posPickedRadio->setChecked(true);

    m_coordsLbl->setText(QString("(%1, %2)").arg(m_config.pickedX).arg(m_config.pickedY));

    // Force states
    m_repCountSpin->setEnabled(!m_config.repeatUntilStopped);
    m_pickBtn->setEnabled(m_config.positionType == CursorPositionType::Picked);
}

void NormalClickWindow::onStartStop()
{
    if (!m_running) {
        saveUItoConfig();
        m_engine->setConfig(m_config);
        m_engine->start();
    } else {
        m_engine->stop();
    }
}

void NormalClickWindow::onEngineStarted()
{
    setRunning(true);
}

void NormalClickWindow::onEngineStopped(const QString& reason)
{
    Q_UNUSED(reason);
    setRunning(false);
}

void NormalClickWindow::onStatusMessage(const QString& msg)
{
    m_statusLbl->setText(msg);
}

void NormalClickWindow::setRunning(bool running)
{
    m_running = running;
    m_startBtn->setEnabled(!running);
    m_stopBtn->setEnabled(running);
    m_saveBtn->setEnabled(!running);
    m_settingsBtn->setEnabled(!running);
    
    m_hrsSpin->setEnabled(!running);
    m_minsSpin->setEnabled(!running);
    m_secsSpin->setEnabled(!running);
    m_msSpin->setEnabled(!running);
    m_mouseBtnCombo->setEnabled(!running);
    m_clickTypeCombo->setEnabled(!running);
    m_repTimesRadio->setEnabled(!running);
    m_repUntilRadio->setEnabled(!running);
    m_posCurrentRadio->setEnabled(!running);
    m_posPickedRadio->setEnabled(!running);

    if (!running) {
        m_repCountSpin->setEnabled(m_repTimesRadio->isChecked());
        m_pickBtn->setEnabled(m_posPickedRadio->isChecked());
    } else {
        m_repCountSpin->setEnabled(false);
        m_pickBtn->setEnabled(false);
    }
}

void NormalClickWindow::onPickPosition()
{
    if (m_running) return;
    m_statusLbl->setText("Click anywhere on screen to point location...");
    m_overlay->enterCaptureMode();
}

void NormalClickWindow::onPositionPicked(QPoint pos)
{
    m_config.pickedX = pos.x();
    m_config.pickedY = pos.y();
    m_coordsLbl->setText(QString("(%1, %2)").arg(pos.x()).arg(pos.y()));
    m_statusLbl->setText(QString("Picked position: %1, %2").arg(pos.x()).arg(pos.y()));
}

void NormalClickWindow::onCaptureAborted()
{
    m_statusLbl->setText("Point pick cancelled.");
}

void NormalClickWindow::onOpenSettings()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Settings");
    dlg.setMinimumWidth(320);

    QVBoxLayout* vl = new QVBoxLayout(&dlg);
    QGroupBox* hkGrp = new QGroupBox("Toggle Hotkey (Start / Stop)", &dlg);
    QFormLayout* hkForm = new QFormLayout(hkGrp);

    QKeySequenceEdit* hkEdit = new QKeySequenceEdit(&dlg);
    hkEdit->setKeySequence(QKeySequence(m_config.toggleHotkey.modifiers | m_config.toggleHotkey.key));
    hkForm->addRow("Key:", hkEdit);
    vl->addWidget(hkGrp);

    QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dlg);
    vl->addWidget(bb);

    connect(bb, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted) {
        QKeySequence ks = hkEdit->keySequence();
        if (!ks.isEmpty()) {
            int combined = ks[0]
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                .toCombined();
#else
                ;
#endif
            m_config.toggleHotkey.key = combined & ~Qt::KeyboardModifierMask;
            m_config.toggleHotkey.modifiers = combined & Qt::KeyboardModifierMask;
        }

        unregisterHotkey();
        registerHotkey();
        updateHotkeyLabels();
    }
}

void NormalClickWindow::onSaveConfig()
{
    saveUItoConfig();
    if (ConfigManager::saveNormal(m_config))
        m_statusLbl->setText("Configuration saved.");
    else
        m_statusLbl->setText("Failed to save configuration!");
}

void NormalClickWindow::registerHotkey()
{
#ifdef Q_OS_WIN
    UINT vk = 0, mods = 0;
    if (!qtKeyToWin(m_config.toggleHotkey.key, m_config.toggleHotkey.modifiers, vk, mods)) {
        vk = VK_F6; mods = 0;
    }
    RegisterHotKey(reinterpret_cast<HWND>(winId()), kHotkeyToggleId, mods, vk);
#endif
}

void NormalClickWindow::unregisterHotkey()
{
#ifdef Q_OS_WIN
    UnregisterHotKey(reinterpret_cast<HWND>(winId()), kHotkeyToggleId);
#endif
}

bool NormalClickWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY && msg->wParam == kHotkeyToggleId) {
            onStartStop();
            *result = 0;
            return true;
        }
    }
#else
    Q_UNUSED(eventType); Q_UNUSED(message); Q_UNUSED(result);
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void NormalClickWindow::closeEvent(QCloseEvent* event)
{
    if (m_running) m_engine->stop();
    unregisterHotkey();
    m_overlay->hide();
    event->accept();
}
