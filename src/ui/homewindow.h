#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

/**
 * @class HomeWindow
 * @brief Main menu/launcher window for the AutoClicker application
 *
 * Provides three entry points to different automation modes:
 *   1. AutoClicker - Multi-point sequence mode (standard)
 *   2. AutoClicker Pro - Advanced multi-point mode with additional features
 *   3. Normal Clicker - Simple single-location timed clicking
 *
 * Also displays and allows configuration of the config directory where
 * settings and automation profiles are stored.
 *
 * This window serves as the hub that users see when the application starts.
 * It emits signals that cause the main window to navigate to different modes.
 */
class HomeWindow : public QWidget
{
    Q_OBJECT
public:
    /// Constructs the HomeWindow
    explicit HomeWindow(QWidget* parent = nullptr);

signals:
    /// Opens the standard multi-point auto-clicker mode
    void openAutoClicker();
    /// Opens the advanced/pro multi-point auto-clicker mode
    void openAutoClickerPro();
    /// Opens the simple single-location clicking mode
    void openNormalClicker();

private slots:
    /// Opens a directory picker to select a new config directory
    void onBrowseConfigDir();
    /// Resets the config directory to the factory default
    void onResetConfigDir();

private:
    /// Constructs the UI layout with mode buttons and config directory controls
    void buildUI();

    QLineEdit* m_configDirEdit = nullptr;  ///< Displays the current config directory path
};
