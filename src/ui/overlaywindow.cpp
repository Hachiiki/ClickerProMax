#include "overlaywindow.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QApplication>
#include <QFont>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────

OverlayWindow::OverlayWindow(QWidget* parent)
    : QWidget(parent)
{
    // Frameless + always on top + tool (no taskbar entry)
    setWindowFlags(Qt::FramelessWindowHint
                   | Qt::WindowStaysOnTopHint
                   | Qt::Tool);

    // Translucent background so unpainted areas are fully transparent
    setAttribute(Qt::WA_TranslucentBackground);

    // Never steal focus from other windows
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFocusPolicy(Qt::StrongFocus); // needed to receive key events in capture mode

    coverAllScreens();
    applyClickThrough(true);
}

// ── Public interface ──────────────────────────────────────────────────────────

void OverlayWindow::setPoints(const QVector<ClickPoint>& points)
{
    m_points = points;
    update();
}

void OverlayWindow::setActivePoint(int index)
{
    if (m_activePoint == index) return;
    m_activePoint = index;
    update();
}

void OverlayWindow::enterCaptureMode()
{
    m_captureMode = true;
    show();
    applyClickThrough(false);  // receive mouse events
    setCursor(Qt::CrossCursor);
    raise();
    activateWindow();          // take focus so ESC works
    update();
}

void OverlayWindow::exitCaptureMode()
{
    m_captureMode = false;
    applyClickThrough(true);
    unsetCursor();
    update();
}

// ── Painting ─────────────────────────────────────────────────────────────────

void OverlayWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // ── Capture-mode overlay ─────────────────────────────────────────────────
    if (m_captureMode) {
        p.fillRect(rect(), QColor(0, 0, 0, 55));   // subtle dim

        // Instruction banner
        QFont bannerFont("Segoe UI", 13, QFont::Bold);
        p.setFont(bannerFont);
        QRect banner(0, 0, width(), 40);
        p.fillRect(banner, QColor(20, 20, 20, 160));
        p.setPen(QColor(255, 220, 60));
        p.drawText(banner, Qt::AlignCenter,
                   "Click anywhere to place a point   |   ESC to cancel");
    }

    // ── Click-point circles ───────────────────────────────────────────────────
    const int R = kRadius;
    QFont numFont("Segoe UI", 10, QFont::Bold);
    QFont delayFont("Segoe UI", 7);

    for (int i = 0; i < m_points.size(); ++i)
    {
        const QPoint& pos = m_points[i].position;
        const bool active = (i == m_activePoint);

        // Colours
        const QColor fill   = active ? QColor(255, 120, 30, 210)
                                     : QColor(40, 130, 255, 185);
        const QColor border = active ? QColor(255, 230, 80)
                                     : QColor(220, 240, 255);
        const QColor shadow(0, 0, 0, 90);

        QRect circ(pos.x() - R, pos.y() - R, R * 2, R * 2);

        // Drop shadow (paint a slightly larger dark circle behind)
        p.setBrush(shadow);
        p.setPen(Qt::NoPen);
        p.drawEllipse(circ.adjusted(2, 3, 2, 3));

        // Main circle
        p.setBrush(fill);
        p.setPen(QPen(border, active ? 2.5 : 1.5));
        p.drawEllipse(circ);

        // Number
        p.setFont(numFont);
        p.setPen(Qt::white);
        p.drawText(circ, Qt::AlignCenter, QString::number(i + 1));

        // Delay label just below the circle
        p.setFont(delayFont);
        p.setPen(QColor(200, 220, 255, 200));
        QRect delayRect(pos.x() - 28, pos.y() + R + 2, 56, 13);
        p.drawText(delayRect, Qt::AlignCenter,
                   QString("%1 ms").arg(m_points[i].delayMs));

        // Arrow from this circle to the next (thin dashed line)
        if (i + 1 < m_points.size())
        {
            const QPoint& next = m_points[i + 1].position;
            QPen arrowPen(QColor(180, 200, 255, 100), 1, Qt::DashLine);
            p.setPen(arrowPen);
            p.setBrush(Qt::NoBrush);
            p.drawLine(pos, next);
        }
    }
}

// ── Mouse / keyboard events ───────────────────────────────────────────────────

void OverlayWindow::mousePressEvent(QMouseEvent* event)
{
    if (!m_captureMode) return;

    if (event->button() == Qt::LeftButton)
    {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        emit pointCaptured(event->globalPosition().toPoint());
#else
        emit pointCaptured(event->globalPos());
#endif
        exitCaptureMode();
    }
    else if (event->button() == Qt::RightButton)
    {
        exitCaptureMode();
        emit captureAborted();
    }
}

void OverlayWindow::keyPressEvent(QKeyEvent* event)
{
    if (m_captureMode && event->key() == Qt::Key_Escape)
    {
        exitCaptureMode();
        emit captureAborted();
    }
}

// ── Helpers ───────────────────────────────────────────────────────────────────

void OverlayWindow::coverAllScreens()
{
    QRect total;
    for (QScreen* s : QApplication::screens())
        total = total.united(s->geometry());
    setGeometry(total);
}

void OverlayWindow::applyClickThrough(bool clickThrough)
{
#ifdef Q_OS_WIN
    HWND hwnd    = reinterpret_cast<HWND>(winId());
    LONG exStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
    if (clickThrough)
        exStyle |= (WS_EX_TRANSPARENT | WS_EX_LAYERED);
    else
        exStyle &= ~WS_EX_TRANSPARENT;
    SetWindowLong(hwnd, GWL_EXSTYLE, exStyle);
#else
    // On X11 you would use XShapeCombineRectangles with ShapeInput.
    // For now, we just make the window invisible to input by lowering it
    // (capture mode raises it back).
    Q_UNUSED(clickThrough);
#endif
}
