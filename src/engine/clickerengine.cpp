#include "clickerengine.h"
#include <QDebug>

#include <cstdlib>
#include <ctime>

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

/**
 * @brief Constructor - initializes timer and random seed
 * @details Sets up a single-shot QTimer for controlling inter-click delays.
 * Seeds the random number generator for anti-detection humanization.
 */
ClickerEngine::ClickerEngine(QObject* parent)
{
    srand(static_cast<unsigned>(time(nullptr)));  // Seed random for anti-detection
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ClickerEngine::tick);
}

/**
 * @brief Destructor - ensures clicking is stopped before cleanup
 */
ClickerEngine::~ClickerEngine()
{
    stop();
}

/**
 * @brief Updates the point sequence to be executed
 */
void ClickerEngine::setPoints(const QVector<ClickPoint>& pts)
{
    m_points = pts;
}

/**
 * @brief Configures the stop condition and its parameters
 * @param cond Type of stop condition (Indefinite, TimeLimit, CycleCount)
 * @param value Associated value - seconds for TimeLimit, count for CycleCount
 */
void ClickerEngine::setStopCondition(StopCondition cond, int value)
{
    m_stopCond = cond;
    if (cond == StopCondition::TimeLimit)  m_timeLimitMs = static_cast<qint64>(value) * 1000;
    if (cond == StopCondition::CycleCount) m_maxCycles   = value;
}

/**
 * @brief Begins the automated clicking sequence
 * @details Initializes execution state, ensures window focus, and triggers
 * the first click immediately. Emits started() signal and begins the timer loop.
 */
void ClickerEngine::start()
{
    if (m_running || m_points.isEmpty()) {
        if (m_points.isEmpty())
            emit statusMessage("No points to click. Add points first.");
        return;
    }

#ifdef Q_OS_WIN
    // CRITICAL: Ensure Roblox window is focused before starting clicks
    POINT pt;
    GetCursorPos(&pt);
    HWND hwnd = WindowFromPoint(pt);
    if (hwnd) {
        // Set the window as foreground (active) window
        SetForegroundWindow(hwnd);
        Sleep(100);  // Give it time to activate
    }
#endif

    m_running      = true;
    m_currentIndex = 0;
    m_cycleCount   = 0;
    m_pendingKeys.clear();
    m_elapsed.start();

    emit started();
    emit statusMessage("Running...");

    // Fire immediately – click the first point without waiting
    tick();
}

/**
 * @brief Halts the clicking sequence immediately
 * @details Stops the timer, sets running flag to false, and emits stopped() signal.
 */
void ClickerEngine::stop()
{
    if (!m_running) return;
    m_running = false;
    m_timer->stop();
    emit stopped("User stopped");
    emit statusMessage("Stopped.");
}

/**
 * @brief Internal timer callback - executes one click iteration
 * @details Checks stop conditions, processes the current point click,
 * handles pending keyboard actions, and schedules the next iteration.
 * Emits currentPointChanged() and cycleCompleted() signals as appropriate.
 */
void ClickerEngine::tick()
{
    if (!m_running) return;

    // Check time-limit stop condition
    if (m_stopCond == StopCondition::TimeLimit &&
        m_elapsed.elapsed() >= m_timeLimitMs)
    {
        m_running = false;
        emit stopped("Time limit reached");
        emit statusMessage(QString("Stopped: time limit of %1 s reached.")
                           .arg(m_timeLimitMs / 1000));
        return;
    }

    if (m_points.isEmpty()) { stop(); return; }

    // Process pending keys if any
    if (!m_pendingKeys.isEmpty()) {
        QString key = m_pendingKeys.takeFirst().trimmed();
        if (!key.isEmpty()) {
            simulateKeyPress(key);
            emit statusMessage(QString("Pressed key: %1").arg(key));
        }

        if (m_pendingKeys.isEmpty()) {
            m_timer->start(m_pendingNextDelayMs);
        } else {
            m_timer->start(static_cast<int>(m_pendingKeyDelaySecs * 1000));
        }
        return;
    }

    // Click the current point
    const ClickPoint& pt = m_points[m_currentIndex];
    doClick(pt.position.x(), pt.position.y());
    emit currentPointChanged(m_currentIndex);
    emit statusMessage(QString("Clicked point %1 / %2   (cycle %3)")
                       .arg(m_currentIndex + 1)
                       .arg(m_points.size())
                       .arg(m_cycleCount + 1));

    // Advance index, detect cycle wrap
    const int delay     = pt.delayMs;   // delay belongs to the point we just clicked
    const int nextIndex = (m_currentIndex + 1) % m_points.size();

    if (nextIndex == 0) {
        m_cycleCount++;
        emit cycleCompleted(m_cycleCount);

        if (m_stopCond == StopCondition::CycleCount &&
            m_cycleCount >= m_maxCycles)
        {
            m_running = false;
            emit stopped(QString("Completed %1 cycles").arg(m_cycleCount));
            emit statusMessage(QString("Stopped: completed %1 cycle(s).").arg(m_cycleCount));
            return;
        }
    }

    m_currentIndex = nextIndex;

    // Schedule the next generic action
    if (!pt.actionKeys.isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        m_pendingKeys = pt.actionKeys.split(',', Qt::SkipEmptyParts);
#else
        m_pendingKeys = pt.actionKeys.split(',', QString::SkipEmptyParts);
#endif
        m_pendingKeyDelaySecs = pt.keyDelaySecs < 0.5 ? 0.5 : pt.keyDelaySecs;
        m_pendingNextDelayMs = delay;
        // Schedule first key press
        m_timer->start(static_cast<int>(m_pendingKeyDelaySecs * 1000));
    } else {
        m_timer->start(delay);
    }
}

// Helper: convert screen pixel coords to absolute normalised (0 – 65535)

#ifdef Q_OS_WIN
static void screenToNorm(int x, int y, LONG& nx, LONG& ny)
{
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    nx = static_cast<LONG>((x * 65536 + sw / 2) / sw);
    ny = static_cast<LONG>((y * 65536 + sh / 2) / sh);
}

// Helper: build and send a single INPUT struct
static void sendMouseInput(DWORD flags, LONG nx, LONG ny)
{
    INPUT inp = {};
    inp.type           = INPUT_MOUSE;
    inp.mi.dx          = nx;
    inp.mi.dy          = ny;
    inp.mi.dwFlags     = flags;
    inp.mi.time        = 0;
    inp.mi.dwExtraInfo = 0;
    SendInput(1, &inp, sizeof(INPUT));
}
#endif

// Human-like mouse movement helper

int ClickerEngine::getRandomVariation(int variance)
{
    // Returns random value between -variance and +variance
    return (rand() % (variance * 2 + 1)) - variance;
}

void ClickerEngine::humanLikeMovement(int targetX, int targetY)
{
#ifdef Q_OS_WIN
    LONG nx, ny;

    // Small "jiggle" to force Roblox to fire hover events
    int jiggleX = targetX + 3 + getRandomVariation(2);
    int jiggleY = targetY + 3 + getRandomVariation(2);

    screenToNorm(jiggleX, jiggleY, nx, ny);
    sendMouseInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, nx, ny);
    Sleep(8 + getRandomVariation(5));

    // Move to exact target so Roblox updates hover state
    screenToNorm(targetX, targetY, nx, ny);
    sendMouseInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE, nx, ny);
    Sleep(15 + getRandomVariation(10));      // let Roblox process the move

    m_lastMouseX = targetX;
    m_lastMouseY = targetY;
#endif
}

// Platform mouse injection with anti-detection

void ClickerEngine::doClick(int x, int y)
{
#ifdef Q_OS_WIN
    // CRITICAL: Ensure the window at target is focused BEFORE each click
    POINT pt = { x, y };
    HWND hwnd = WindowFromPoint(pt);
    if (hwnd) {
        SetForegroundWindow(hwnd);
        Sleep(10);
    }

    // Move the cursor with a proper MOUSEMOVE event so Roblox registers hover
    humanLikeMovement(x, y);

    Sleep(20 + getRandomVariation(15));  // Random delay before click

    // Convert target to normalised absolute coords for SendInput
    LONG nx, ny;
    screenToNorm(x, y, nx, ny);

    // LEFT BUTTON DOWN
    sendMouseInput(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE, nx, ny);
    Sleep(15 + getRandomVariation(10));  // Variable hold time

    // LEFT BUTTON UP
    sendMouseInput(MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE, nx, ny);

    Sleep(10 + getRandomVariation(5));   // Recovery delay after click
#else
    // Linux / X11 (XTest extension)
    // Requires linking against Xtst:  LIBS += -lX11 -lXtst
    // and #include <X11/extensions/XTest.h>
    //
    // Display* dpy = XOpenDisplay(nullptr);
    // XTestFakeMotionEvent(dpy, -1, x, y, CurrentTime);
    // XFlush(dpy);
    // XTestFakeButtonEvent(dpy, 1, True,  CurrentTime);
    // XTestFakeButtonEvent(dpy, 1, False, CurrentTime);
    // XFlush(dpy);
    // XCloseDisplay(dpy);
    //
    // For now just log (remove when enabling XTest):
    qDebug() << "Click at" << x << y;
    Q_UNUSED(x); Q_UNUSED(y);
#endif
}

void ClickerEngine::simulateKeyPress(const QString& keyStr)
{
#ifdef Q_OS_WIN
    QString k = keyStr.trimmed().toUpper();
    WORD vk = 0;
    
    if (k == "SPACE") vk = VK_SPACE;
    else if (k == "ENTER") vk = VK_RETURN;
    else if (k == "TAB") vk = VK_TAB;
    else if (k == "UP") vk = VK_UP;
    else if (k == "DOWN") vk = VK_DOWN;
    else if (k == "LEFT") vk = VK_LEFT;
    else if (k == "RIGHT") vk = VK_RIGHT;
    else if (k == "SHIFT") vk = VK_SHIFT;
    else if (k == "CTRL") vk = VK_CONTROL;
    else if (k == "ALT") vk = VK_MENU;
    else if (k == "ESC" || k == "ESCAPE") vk = VK_ESCAPE;
    else if (k.length() == 1) {
        short scan = VkKeyScanA(k[0].toLatin1());
        vk = scan & 0xFF;
    }

    if (vk != 0) {
        INPUT inp = {};
        inp.type = INPUT_KEYBOARD;
        inp.ki.wVk = vk;
        SendInput(1, &inp, sizeof(INPUT));
        Sleep(20 + getRandomVariation(10));
        inp.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &inp, sizeof(INPUT));
    }
#else
    qDebug() << "Press key:" << keyStr;
#endif
}
