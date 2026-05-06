#pragma once
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "core/clickpoint.h"
#include "core/configmanager.h"

/**
 * @class ClickerEngine
 * @brief Executes multi-point automated clicking sequences
 *
 * Implements a state machine that orchestrates clicking through a list of target points
 * in sequence. Each point is clicked, then the engine waits for that point's configured
 * delay before moving to the next point.
 *
 * Timing Model:
 *   - Uses QTimer (single-shot) for precise inter-click delays
 *   - After clicking point[i], timer starts with point[i].delayMs
 *   - Timer fires → click point[i+1] → repeat
 *   - Includes human-like cursor movement and anti-detection features
 *
 * Stop Conditions:
 *   - Indefinite: run until manually stopped
 *   - TimeLimit: stop after N seconds of execution
 *   - CycleCount: stop after N complete cycles through all points
 *
 * Features:
 *   - Per-point keyboard action support (type text/keys at each click)
 *   - Human-like cursor movement with randomization
 *   - Cycle tracking and status signals for UI updates
 *   - Windows-specific window focus before starting
 */
class ClickerEngine : public QObject
{
    Q_OBJECT
public:
    /// Constructs a ClickerEngine instance
    explicit ClickerEngine(QObject* parent = nullptr);
    ~ClickerEngine() override;

    /// Sets the point sequence to be executed
    void setPoints(const QVector<ClickPoint>& pts);
    /// Configures when the automation should stop (Indefinite, TimeLimit, CycleCount)
    void setStopCondition(StopCondition cond, int value = 0);

    /// Returns true if automation is currently running
    bool isRunning() const { return m_running; }

    /// Begins the click sequence execution
    void start();
    /// Stops the click sequence immediately
    void stop();

signals:
    /// Emitted when automation successfully starts
    void started();
    /// Emitted when automation stops with a reason message
    void stopped(const QString& reason);
    /// Emitted immediately after a point is clicked with its index
    void currentPointChanged(int index);
    /// Emitted after completing one full cycle through all points
    void cycleCompleted(int cycleNum);
    /// Emitted for status updates (user feedback)
    void statusMessage(const QString& msg);

private slots:
    /// Timer callback that processes the next click in the sequence
    void tick();

private:
    /// Simulates a mouse click at the specified screen coordinates
    void doClick(int x, int y);
    /// Moves cursor smoothly from current position to target with randomization
    void humanLikeMovement(int targetX, int targetY);
    /// Generates a random value with a given variance for anti-detection
    int getRandomVariation(int variance);
    /// Simulates keyboard key presses (e.g., "A", "Space", "A, B, C")
    void simulateKeyPress(const QString& keyStr);

    QTimer* m_timer;               ///< Single-shot timer for inter-click delays
    QElapsedTimer m_elapsed;       ///< Tracks total runtime for TimeLimit condition
    int m_lastMouseX = 0;          ///< Last mouse X position (for movement)
    int m_lastMouseY = 0;          ///< Last mouse Y position (for movement)

    QVector<ClickPoint> m_points;  ///< The sequence of points to click
    int m_currentIndex = 0;        ///< Index of the next point to click
    int m_cycleCount = 0;          ///< Number of completed cycles

    QStringList m_pendingKeys;     ///< Queue of pending keyboard actions
    int m_pendingNextDelayMs = 0;  ///< Delay before executing pending keys
    double m_pendingKeyDelaySecs = 0.5;  ///< Delay between individual key presses

    StopCondition m_stopCond = StopCondition::Indefinite;  ///< When to stop
    int m_maxCycles = 0;           ///< Max cycles (if CycleCount condition)
    qint64 m_timeLimitMs = 0;      ///< Max runtime in ms (if TimeLimit condition)

    bool m_running = false;        ///< Current execution state
};
