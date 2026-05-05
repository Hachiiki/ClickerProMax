# AutoClicker

A Qt/C++ desktop auto-clicker with numbered screen overlays, per-point delays, configurable stop conditions, and global hotkeys.

---

## Features

| Feature | Detail |
|---|---|
| **Numbered overlays** | Floating translucent circles on screen at each click point (always on top, click-through) |
| **Per-point delay** | Each point has its own "delay after click" (default 300 ms) |
| **Sequence arrows** | Dashed line drawn from each circle to the next |
| **Stop conditions** | Run indefinitely · Stop after N seconds · Stop after N cycles |
| **Global hotkey** | Press the configured key (default **F6**) anywhere to toggle start/stop |
| **Config persistence** | All settings & points auto-saved to JSON in AppData |

---

## Build with CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64"   # adjust to your Qt path
cmake --build . --config Release
```

## Build with qmake

```bash
qmake AutoClicker.pro
# Windows:
nmake          # or mingw32-make
# Linux/macOS:
make
```

---

## How to use

1. **Launch** – the overlay appears immediately (invisible until you add points).
2. **Add Point** – click the button; the overlay dims and shows an instruction banner. Click anywhere on screen to drop a numbered circle.
3. **Edit Point** – double-click a row in the list (or select + Edit) to change the delay or add a label.
4. **Reorder** – use ▲/▼ buttons to change the click sequence.
5. **Settings** – configure stop condition and the toggle hotkey.
6. **Start/Stop** – button in the main window, or press the hotkey (F6 by default) from any window.

### Click sequence

```
Click Point 1  →  wait Point1.delay ms  →  Click Point 2  →  wait Point2.delay ms  →  ...
```

When the last point is reached the sequence wraps back to Point 1 (one cycle complete).

---

## Platform notes

### Windows
Full support: overlay click-through via `WS_EX_TRANSPARENT`, mouse injection via `SendInput`, global hotkey via `RegisterHotKey`.

### Linux
The `ClickerEngine::doClick()` stub in `clickerengine.cpp` contains commented XTest code. To enable:
1. Install `libxtst-dev` (Debian/Ubuntu) or equivalent.
2. Add `LIBS += -lX11 -lXtst` to the `.pro` file.
3. Uncomment the XTest block in `clickerengine.cpp`.

---

## Config file location
`%APPDATA%\AutoClicker\AutoClicker\autoclicker_config.json`
