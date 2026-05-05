cmake_minimum_required(VERSION 3.16)
project(AutoClicker VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Try Qt6 first, fall back to Qt5
find_package(Qt6 QUIET COMPONENTS Core Widgets)
if(Qt6_FOUND)
    set(QT_LIBS Qt6::Core Qt6::Widgets)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    set(QT_LIBS Qt5::Core Qt5::Widgets)
    message(STATUS "Using Qt5")
endif()

set(SOURCES
    src/main.cpp
    src/ui/mainwindow.cpp
    src/ui/homewindow.cpp
    src/ui/normalclickwindow.cpp
    src/engine/normalclickerengine.cpp
    src/ui/overlaywindow.cpp
    src/engine/clickerengine.cpp
    src/core/configmanager.cpp
    src/core/appsettings.cpp
)

set(HEADERS
    src/ui/mainwindow.h
    src/ui/homewindow.h
    src/ui/normalclickwindow.h
    src/engine/normalclickerengine.h
    src/ui/overlaywindow.h
    src/engine/clickerengine.h
    src/core/configmanager.h
    src/core/clickpoint.h
    src/core/appsettings.h
    src/util/hotkey_utils.h
)

if(WIN32)
    add_executable(AutoClicker WIN32 ${SOURCES} ${HEADERS})
else()
    add_executable(AutoClicker ${SOURCES} ${HEADERS})
endif()

target_include_directories(AutoClicker PRIVATE src)
target_link_libraries(AutoClicker ${QT_LIBS})

if(WIN32)
    target_link_libraries(AutoClicker user32)
endif()

# Copy executable to build root for convenience
set_target_properties(AutoClicker PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
