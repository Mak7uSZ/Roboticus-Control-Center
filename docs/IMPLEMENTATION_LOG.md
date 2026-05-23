# Implementation Log

## 2026-05-23 11:11:34 +02:00

Goal of this step: Fix the `ConnectionBar` QML layout so mode selection and serial controls are separated into two rows.

Files inspected:

- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`
- `ui/components/ConnectionBar.qml`
- `ui/Main.qml`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `ui/components/Monitor/Monitor.qml`
- `ui/components/Graph/Graph.qml`
- `CMakeLists.txt`
- `build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug/CMakeCache.txt`

Files changed:

- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Increased the connection bar height to fit two rows.
- Moved Wired/Wireless buttons into a dedicated top row.
- Moved COM port, baud rate, and Start Monitor controls into a dedicated lower row.
- Kept the lower serial controls visible and enabled only when `connectionMode` is `"wired"`.
- Did not change C++ logic, parser logic, serial logic, graph, monitor, timeline, or UDP code.

Errors encountered:

- The previously documented `E:/QT/Tools/CMake_64/bin/cmake.exe` path was not available in this shell.
- The previously documented `build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug` directory did not exist. The current build directory is `build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug`.
- Fixed by inspecting the current `CMakeCache.txt`, then building with the cached Qt tool paths under `C:/Qt`.

Tests performed:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- Runtime UI behavior: Not tested. The app was built but not launched.

## 2026-05-21 20:05:49 +02:00

Goal of this step: Add a minimal, reversible wired/wireless UI mode switch without adding UDP transport yet.

Files inspected:

- `include/parser/SerialFrameExtractor.h`
- `src/parser/SerialFrameExtractor.cpp`
- `include/io/SerialParser.h`
- `src/io/SerialParser.cpp`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `ui/Main.qml`
- `ui/components/ConnectionBar.qml`
- `include/io/SerialPortManager.h`
- `src/io/SerialPortManager.cpp`
- `CMakeLists.txt`
- `README.md`
- `build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug/CMakeCache.txt`

Files changed:

- `include/parser/SerialFrameExtractor.h`
- `src/parser/SerialFrameExtractor.cpp`
- `include/io/SerialParser.h`
- `src/io/SerialParser.cpp`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `ui/Main.qml`
- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Added `reset()` to `SerialFrameExtractor` and `SerialParser`.
- Added `connectionMode`, `switchToWiredMode()`, and `switchToWirelessMode()` to `AppController`.
- Wireless mode disconnects `SerialPortManager` if connected and resets the parser buffer.
- Wired mode resets the parser buffer.
- Passed `appController` into `ConnectionBar`.
- Added Wired and Wireless buttons.
- Hid and disabled the old serial controls while wireless mode is active.
- Added project context documentation for future continuity.

Errors encountered:

- `git status --short` failed because Git reported dubious ownership for the repository. A one-command `-c safe.directory=...` override was used for inspection only.
- `git -c safe.directory=... status --short` printed a permission warning for `.qtcreator/qtc-cmake-presets-BdPpBqQx/`.
- `cmake --build build` failed because `cmake` was not on PATH.
- `E:/QT/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug` initially failed in qmsgpack AutoMoc because the MinGW compiler component could not run correctly from this shell.
- The build succeeded after prepending `E:\QT\Tools\mingw1310_64\bin` to PATH for the build command.

Tests performed:

```powershell
$env:Path = "E:\QT\Tools\mingw1310_64\bin;$env:Path"; & E:/QT/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- Runtime UI behavior: Not confirmed yet.

Next action:

Add `UDPConnection` later as a second raw byte input source that emits `rawDataReceived(QByteArray)` into the existing parser.
