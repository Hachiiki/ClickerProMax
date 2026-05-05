QT += core widgets
CONFIG += c++17
TARGET = AutoClicker
TEMPLATE = app

SOURCES += \
    src/main.cpp \
    src/ui/mainwindow.cpp \
    src/ui/homewindow.cpp \
    src/ui/normalclickwindow.cpp \
    src/engine/normalclickerengine.cpp \
    src/ui/overlaywindow.cpp \
    src/engine/clickerengine.cpp \
    src/core/configmanager.cpp \
    src/core/appsettings.cpp

HEADERS += \
    src/ui/mainwindow.h \
    src/ui/homewindow.h \
    src/ui/normalclickwindow.h \
    src/engine/normalclickerengine.h \
    src/ui/overlaywindow.h \
    src/engine/clickerengine.h \
    src/core/configmanager.h \
    src/core/clickpoint.h \
    src/core/appsettings.h \
    src/util/hotkey_utils.h

INCLUDEPATH += src

win32 {
    LIBS += -luser32
    # Build as Windows application (no console window)
    CONFIG += windows
    RC_ICONS = src/logo/ClickerProMax.ico
}

# Output directory
DESTDIR = $$OUT_PWD/bin
