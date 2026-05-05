#ifndef HOTKEY_UTILS_H
#define HOTKEY_UTILS_H

#ifdef Q_OS_WIN
#include <windows.h>

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
