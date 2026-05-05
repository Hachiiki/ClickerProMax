#include "homewindow.h"
#include "core/appsettings.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QFileDialog>

HomeWindow::HomeWindow(QWidget* parent) : QWidget(parent)
{
    setWindowTitle("App Navigation");
    setMinimumSize(420, 520);
    buildUI();
}

void HomeWindow::buildUI()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(20);

    QLabel* title = new QLabel("App Navigation", this);
    QFont f = title->font();
    f.setPointSize(f.pointSize() + 4);
    f.setBold(true);
    title->setFont(f);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    // ── Feature cards ────────────────────────────────────────────────────────
    QGridLayout* grid = new QGridLayout;

    // Multi-Point Card
    QPushButton* acCard = new QPushButton("Multi-Point AutoClicker\n\nClick here to use", this);
    acCard->setMinimumSize(200, 150);
    QFont cf = acCard->font();
    cf.setPointSize(cf.pointSize() + 2);
    acCard->setFont(cf);
    connect(acCard, &QPushButton::clicked, this, &HomeWindow::openAutoClicker);
    grid->addWidget(acCard, 0, 0);

    // Normal Clicker Card
    QPushButton* normalCard = new QPushButton("Normal AutoClicker\n\nClick here to use", this);
    normalCard->setMinimumSize(200, 150);
    normalCard->setFont(cf);
    connect(normalCard, &QPushButton::clicked, this, &HomeWindow::openNormalClicker);
    grid->addWidget(normalCard, 0, 1);

    // Padding for grid to keep cards in top-left
    grid->setRowStretch(1, 1);
    grid->setColumnStretch(2, 1);

    layout->addLayout(grid);

    // ── Config folder settings ───────────────────────────────────────────────
    QGroupBox* cfgGrp = new QGroupBox("Config Folder", this);
    QVBoxLayout* cfgVl = new QVBoxLayout(cfgGrp);

    QLabel* cfgHint = new QLabel(
        "All configuration files are saved in this folder.\n"
        "Each feature stores its own config file here.", this);
    cfgHint->setWordWrap(true);
    cfgVl->addWidget(cfgHint);

    m_configDirEdit = new QLineEdit(this);
    m_configDirEdit->setReadOnly(true);
    m_configDirEdit->setText(
        QDir::toNativeSeparators(AppSettings::instance().configDir()));
    cfgVl->addWidget(m_configDirEdit);

    QHBoxLayout* cfgBtns = new QHBoxLayout;
    QPushButton* browseBtn = new QPushButton("Browse…", this);
    QPushButton* resetBtn  = new QPushButton("Reset to Default", this);
    cfgBtns->addWidget(browseBtn);
    cfgBtns->addWidget(resetBtn);
    cfgBtns->addStretch();
    cfgVl->addLayout(cfgBtns);

    connect(browseBtn, &QPushButton::clicked, this, &HomeWindow::onBrowseConfigDir);
    connect(resetBtn,  &QPushButton::clicked, this, &HomeWindow::onResetConfigDir);

    layout->addWidget(cfgGrp);
}

void HomeWindow::onBrowseConfigDir()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, "Choose Config Folder",
        AppSettings::instance().configDir());
    if (dir.isEmpty()) return;                       // user cancelled

    AppSettings::instance().setConfigDir(dir);
    m_configDirEdit->setText(QDir::toNativeSeparators(dir));
}

void HomeWindow::onResetConfigDir()
{
    const QString def = AppSettings::defaultConfigDir();
    AppSettings::instance().setConfigDir(def);
    m_configDirEdit->setText(QDir::toNativeSeparators(def));
}
