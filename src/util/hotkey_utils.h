#ifndef HOTKEY_UTILS_H
#define HOTKEY_UTILS_H

#ifdef Q_OS_WIN
#include <windows.h>

/**
 * @brief Converts Qt hotkey codes to Windows virtual key codes
 *
 * Maps Qt's key enumeration and modifier flags to the Windows API format
 * required by RegisterHotKey(). Supports function keys (F1-F24) and
 * alphanumeric keys (A-Z, 0-9) with optional modifiers (Ctrl, Alt, Shift, Win).
 *
 * @param qtKey Qt::Key enum value (e.g., Qt::Key_F6, Qt::Key_A)
 * @param qtMods Qt modifier flags (Qt::ControlModifier, Qt::AltModifier, etc.)
 * @param[out] vk Windows virtual key code (VK_* constant)
 * @param[out] mods Windows modifier flags (MOD_CONTROL, MOD_ALT, etc.)
 * @return True if conversion successful, false for unsupported keys
 *
 * @example
 *   UINT vk, mods;
 *   if (qtKeyToWin(Qt::Key_F6, 0, vk, mods)) {
 *       RegisterHotKey(hwnd, id, mods, vk);
 *   }
 */
inline bool qtKeyToWin(int qtKey, int qtMods, UINT& vk, UINT& mods)
{
    mods = 0;
    if (qtMods & Qt::ControlModifier) mods |= MOD_CONTROL;
    if (qtMods & Qt::AltModifier)     mods |= MOD_ALT;
    if (qtMods & Qt::ShiftModifier)   mods |= MOD_SHIFT;
    if (qtMods & Qt::MetaModifier)    mods |= MOD_WIN;

    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F24) {
        vk = VK_F1 + (qtKey - Qt::Key_F1);
        return true;
    }
    if ((qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) ||
        (qtKey >= Qt::Key_0 && qtKey <= Qt::Key_9)) {
        vk = static_cast<UINT>(qtKey);
        return true;
    }
    return false;
}
#endif

#endif // HOTKEY_UTILS_H
