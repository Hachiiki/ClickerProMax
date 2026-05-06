#pragma once
#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "core/clickpoint.h"
#include "core/configmanager.h"

/*  ClickerEngine
 *
 *  Drives the sequential auto-clicking loop:
 *
 *      click(pt[0])  --[pt[0].delay ms]--> click(pt[1]) --[pt[1].delay ms]--> ...
 *
 *  One QTimer is used (single-shot). After each click the timer is started
 *  with that point's delay; when it fires the next point is clicked.
 */
class ClickerEngine : public QObject
{
    Q_OBJECT
public:
    explicit ClickerEngine(QObject* parent = nullptr);
    ~ClickerEngine() override;

    void setPoints(const QVector<ClickPoint>& pts);
    void setStopCondition(StopCondition cond, int value = 0);

    bool isRunning() const { return m_running; }

    void start();
    void stop();

signals:
    void started();
    void stopped(const QString& reason);
    void currentPointChanged(int index);  // emitted just after a click
    void cycleCompleted(int cycleNum);
    void statusMessage(const QString& msg);

private slots:
    void tick();

private:
    void doClick(int x, int y);
    void humanLikeMovement(int targetX, int targetY);
    int  getRandomVariation(int variance);

    QTimer*        m_timer;
    QElapsedTimer  m_elapsed;
    int            m_lastMouseX = 0;  // Track last mouse position for movement
    int            m_lastMouseY = 0;

    QVector<ClickPoint> m_points;
    int  m_currentIndex = 0;
    int  m_cycleCount   = 0;

    QStringList m_pendingKeys;
    int         m_pendingNextDelayMs = 0;
    double      m_pendingKeyDelaySecs = 0.5;

    void simulateKeyPress(const QString& keyStr);

    StopCondition m_stopCond     = StopCondition::Indefinite;
    int           m_maxCycles    = 0;
    qint64        m_timeLimitMs  = 0;

    bool m_running = false;
};
