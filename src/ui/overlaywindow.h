#pragma once
#include <QWidget>
#include <QVector>
#include "core/clickpoint.h"

/**
 * @class OverlayWindow
 * @brief Frameless, translucent, always-on-top visual overlay for screen positioning
 *
 * Renders numbered circles at each configured click point on the screen and provides
 * an interactive "capture mode" for users to select new point locations by clicking.
 *
 * Two Operating Modes:
 *   1. Display Mode (normal): Renders read-only circles at each point location,
 *      click-through (transparent to mouse), allows visualization of the click sequence
 *   2. Capture Mode: Becomes mouse-opaque, ready to receive a click, emits pointCaptured()
 *      at the clicked location, then returns to display mode
 *
 * Visual Features:
 *   - Numbered circles (1, 2, 3...) for each click point
 *   - Dashed lines connecting points to show sequence flow
 *   - Active/highlight state for current point during execution
 *   - Spans all virtual screens on multi-monitor setups
 *   - Always-on-top with frameless window
 */
class OverlayWindow : public QWidget
{
    Q_OBJECT
public:
    /// Constructs the overlay window (initially hidden)
    explicit OverlayWindow(QWidget* parent = nullptr);

    /// Sets the list of click points to be visualized
    void setPoints(const QVector<ClickPoint>& points);

    /// Highlights a specific point during execution (index -1 = no highlight)
    void setActivePoint(int index);

    /// Enables capture mode - window becomes opaque and ready to receive a click
    void enterCaptureMode();
    /// Disables capture mode - window returns to click-through display mode
    void exitCaptureMode();
    /// Checks if window is currently in capture mode
    bool isCaptureMode() const { return m_captureMode; }

signals:
    /// Emitted when user clicks during capture mode with the screen position
    void pointCaptured(QPoint screenPos);
    /// Emitted when user presses Escape during capture mode
    void captureAborted();

protected:
    /// Renders the visual elements (circles, numbers, lines)
    void paintEvent(QPaintEvent*) override;
    /// Handles mouse clicks (used in capture mode to emit pointCaptured)
    void mousePressEvent(QMouseEvent*) override;
    /// Handles Escape key press (used to abort capture mode)
    void keyPressEvent(QKeyEvent*) override;

private:
    /// Makes window click-through (clickThrough=true) or opaque (clickThrough=false)
    void applyClickThrough(bool clickThrough);
    /// Resizes and positions window to cover all screens on the system
    void coverAllScreens();

    QVector<ClickPoint> m_points;  ///< List of points to display
    int m_activePoint = -1;        ///< Index of the highlighted point (-1 = none)
    bool m_captureMode = false;    ///< Is the window in capture mode

    static constexpr int kRadius = 20;  ///< Radius of the drawn circles (pixels)
};
