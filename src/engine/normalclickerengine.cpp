#include "normalclickerengine.h"
#include "core/configmanager.h"
#include <QDebug>
#ifdef Q_OS_WIN
#  include <windows.h>
#endif

/**
 * @brief Constructor - initializes timer for clicking
 * @details Sets up a single-shot QTimer for controlling click intervals
 */
NormalClickerEngine::NormalClickerEngine(QObject* parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &NormalClickerEngine::tick);
}

/**
 * @brief Destructor - ensures clicking is stopped before cleanup
 */
NormalClickerEngine::~NormalClickerEngine()
{
    stop();
}

/**
 * @brief Updates the clicking configuration
 * @param config New configuration with timing, button type, and position settings
 */
void NormalClickerEngine::setConfig(const NormalAppConfig& config)
{
    m_config = config;
}

/**
 * @brief Begins the clicking sequence
 * @details Validates that interval is non-zero, initializes state, emits started(),
 * and schedules the first click
 */
void NormalClickerEngine::start()
{
    if (m_running) return;
    
    // Check interval
    int intervalMs = m_config.ms + (m_config.secs * 1000) + (m_config.mins * 60000) + (m_config.hours * 3600000);
    if (intervalMs == 0) {
        emit statusMessage("Interval cannot be 0. Stopped.");
        return;
    }

    m_running = true;
    m_clickCount = 0;

    emit started();
    emit statusMessage("Running...");
    
    // Start first tick
    tick();
}

/**
 * @brief Halts the clicking sequence immediately
 * @details Stops the timer, sets running flag to false, and emits stopped() signal
 */
void NormalClickerEngine::stop()
{
    if (!m_running) return;
    m_running = false;
    m_timer->stop();
    emit stopped("User stopped");
    emit statusMessage("Stopped.");
}

#ifdef Q_OS_WIN
static void screenToNormPos(int x, int y, LONG& nx, LONG& ny)
{
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    nx = static_cast<LONG>((x * 65536 + sw / 2) / sw);
    ny = static_cast<LONG>((y * 65536 + sh / 2) / sh);
}

static void sendSingleInput(DWORD flags, LONG nx, LONG ny)
{
    INPUT inp = {};
    inp.type = INPUT_MOUSE;
    inp.mi.dx = nx;
    inp.mi.dy = ny;
    inp.mi.dwFlags = flags;
    SendInput(1, &inp, sizeof(INPUT));
}
#endif

/**
 * @brief Executes a single mouse click at the configured location
 * @details Determines target position (current cursor or pre-selected),
 * converts to normalized screen coordinates, and sends appropriate
 * mouse input events to the Windows API (left/right/middle, single/double)
 */
void NormalClickerEngine::doClick()
{
#ifdef Q_OS_WIN
    int targetX, targetY;
    if (m_config.positionType == CursorPositionType::Current) {
        POINT pt;
        GetCursorPos(&pt);
        targetX = pt.x;
        targetY = pt.y;
    } else {
        targetX = m_config.pickedX;
        targetY = m_config.pickedY;
    }

    LONG nx, ny;
    screenToNormPos(targetX, targetY, nx, ny);

    DWORD downFlag = 0;
    DWORD upFlag = 0;
    if (m_config.buttonType == MouseButtonType::Left) { downFlag = MOUSEEVENTF_LEFTDOWN; upFlag = MOUSEEVENTF_LEFTUP; }
    else if (m_config.buttonType == MouseButtonType::Right) { downFlag = MOUSEEVENTF_RIGHTDOWN; upFlag = MOUSEEVENTF_RIGHTUP; }
    else if (m_config.buttonType == MouseButtonType::Middle) { downFlag = MOUSEEVENTF_MIDDLEDOWN; upFlag = MOUSEEVENTF_MIDDLEUP; }

    HWND hwnd = WindowFromPoint({targetX, targetY});
    if (hwnd) { SetForegroundWindow(hwnd); Sleep(5); }

    int numClicks = (m_config.clickType == ClickType::Double) ? 2 : 1;
    for (int i = 0; i < numClicks; ++i) {
        sendSingleInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | downFlag, nx, ny);
        Sleep(10);
        sendSingleInput(MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | upFlag, nx, ny);
        if (i == 0 && numClicks == 2) Sleep(50);
    }
#endif
}

/**
 * @brief Timer callback - executes next click and schedules the following one
 * @details Checks repeat limit, performs a click, increments counter,
 * emits status signals, and schedules next iteration
 */
void NormalClickerEngine::tick()
{
    if (!m_running) return;

    if (!m_config.repeatUntilStopped && m_clickCount >= m_config.repeatTimes) {
        m_running = false;
        emit stopped("Repeat count reached");
        emit statusMessage(QString("Stopped: limit of %1 reached").arg(m_config.repeatTimes));
        return;
    }

    doClick();
    m_clickCount++;
    emit clicksUpdated(m_clickCount);

    if (m_config.repeatUntilStopped) {
        emit statusMessage(QString("Clicked %1 times").arg(m_clickCount));
    } else {
        emit statusMessage(QString("Clicked %1 / %2 times").arg(m_clickCount).arg(m_config.repeatTimes));
    }

    int intervalMs = m_config.ms + (m_config.secs * 1000) + (m_config.mins * 60000) + (m_config.hours * 3600000);
    m_timer->start(intervalMs);
}
