#pragma once
#include <QObject>
#include <QTimer>
#include "core/configmanager.h"

/**
 * @class NormalClickerEngine
 * @brief Simple timed clicking engine for basic automation
 *
 * Implements a straightforward clicking loop at a single screen location
 * with configurable timing intervals. Unlike ClickerEngine, this engine
 * doesn't manage a sequence of points but rather repeats clicks at a
 * single location until stopped or repeat limit reached.
 *
 * Features:
 *   - Configurable click interval (hours, minutes, seconds, milliseconds)
 *   - Selectable button type (left, right, middle)
 *   - Single or double-click modes
 *   - Optional position picker for selecting target coordinates
 *   - Current cursor position or pre-selected position support
 *   - Repeat until stopped or fixed repeat count
 *   - Global hotkey support
 */
class NormalClickerEngine : public QObject
{
    Q_OBJECT
public:
    /// Constructs a NormalClickerEngine instance
    explicit NormalClickerEngine(QObject* parent = nullptr);
    ~NormalClickerEngine() override;

    /// Updates the engine configuration (timing, button, position, etc.)
    void setConfig(const NormalAppConfig& config);
    /// Returns true if clicking is currently running
    bool isRunning() const { return m_running; }

    /// Begins the clicking sequence
    void start();
    /// Stops the clicking sequence
    void stop();

signals:
    /// Emitted when clicking successfully starts
    void started();
    /// Emitted when clicking stops with a reason
    void stopped(const QString& reason);
    /// Emitted after each click with the total click count
    void clicksUpdated(int totalClicks);
    /// Emitted for status updates (user feedback)
    void statusMessage(const QString& msg);

private slots:
    /// Timer callback that executes the next click
    void tick();

private:
    /// Performs a single mouse click using Windows API or system calls
    void doClick();

    QTimer* m_timer;               ///< Timer controlling click intervals
    NormalAppConfig m_config;      ///< Current configuration (interval, button, etc.)
    bool m_running = false;        ///< Current execution state
    int m_clickCount = 0;          ///< Total clicks executed in this session
};
