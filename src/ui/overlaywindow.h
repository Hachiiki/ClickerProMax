#pragma once
#include <QWidget>
#include <QVector>
#include "core/clickpoint.h"

/*  OverlayWindow
 *
 *  A frameless, translucent, always-on-top window that spans the entire
 *  virtual desktop.  It does two jobs:
 *
 *  1. Renders floating numbered circles at each click-point position at all
 *     times (click-through when the app is idle or running).
 *
 *  2. Enters "capture mode" when the user wants to add a new point: the
 *     window temporarily becomes mouse-opaque so it can receive the click,
 *     emits pointCaptured(), then returns to click-through.
 */
class OverlayWindow : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWindow(QWidget* parent = nullptr);

    // ── Point management ─────────────────────────────────────────────────────
    void setPoints(const QVector<ClickPoint>& points);

    // Highlight the currently active point (-1 = none)
    void setActivePoint(int index);

    // ── Capture mode ─────────────────────────────────────────────────────────
    void enterCaptureMode();
    void exitCaptureMode();
    bool isCaptureMode() const { return m_captureMode; }

signals:
    void pointCaptured(QPoint screenPos);
    void captureAborted();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;

private:
    void applyClickThrough(bool clickThrough);
    void coverAllScreens();

    QVector<ClickPoint> m_points;
    int  m_activePoint  = -1;
    bool m_captureMode  = false;

    static constexpr int kRadius = 20;
};
