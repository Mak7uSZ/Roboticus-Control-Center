# Implementation Log

## 2026-05-23 13:32:27 +02:00

Goal of this step: Fix the ConnectionBar wired/wireless layout and visible timeout/error behavior without changing backend telemetry logic.

Files inspected:

- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`
- `ui/components/ConnectionBar.qml`
- `ui/Main.qml`
- `ui/components/StyledComboBox.qml`
- `ui/components/Error.qml`
- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`

Files changed:

- `ui/components/ConnectionBar.qml`
- `ui/Main.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Added a shared mode-button style in `ConnectionBar.qml` so selected Wired/Wireless buttons use green background with dark text, while inactive buttons keep dark background with white text.
- Aligned the Wireless controls row with the Wired controls row by using shared row margins, height, spacing, and UDP field width.
- Let the Wireless status text fill the remaining row width instead of leaving a trailing filler item, so the row uses available space more cleanly.
- Stopped showing `udpConnection.errorString` directly as inline ConnectionBar text; inline wireless errors now come from local validation/start handling only.
- Suppressed the no-datagram UDP timeout popup in `Main.qml` while keeping other error popups visible.
- Did not change C++ telemetry routing, UDP listening, parser logic, SerialParser, SerialPortManager, AppController backend behavior, graph, monitor, timeline, or models.

Errors encountered:

- The first build reached the link step but failed because `appRoboticus_Data_Visualiser.exe` was still running and locked.
- Stopped the running app process and reran the same build successfully.

Tests performed:

- Build command:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- Runtime visual inspection was not performed during this step.

## 2026-05-23 12:28:10 +02:00

Goal of this step: Implement app-side wireless telemetry by feeding UDP datagrams into the existing binary frame parser.

Files inspected:

- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `include/io/SerialParser.h`
- `src/io/SerialParser.cpp`
- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`

Files changed:

- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `src/io/UDPConnection.cpp`
- `src/io/SerialParser.cpp`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Added explicit parser input routing in `AppController`.
- Wired mode connects `SerialPortManager::rawDataReceived` to `SerialParser::onRawDataReady` and disconnects/stops UDP parser input.
- Wireless monitor start disconnects serial parser input, resets `SerialParser`, starts UDP listening, then connects `UDPConnection::rawDataReceived` to the parser after the listener starts successfully.
- Wireless monitor stop disconnects UDP parser input, stops UDP listening, and resets `SerialParser`.
- Preserved the existing frame protocol: UDP datagrams are expected to contain `0xFD + uint16 little-endian payload length + MsgPack payload`.
- Did not parse MsgPack in `UDPConnection` and did not add JSON telemetry.
- Added minimal debug logs for UDP datagram size, first byte, wireless bytes reaching `SerialParser`, and successful `frameDecoded` emission.
- Did not change graph, model, monitor, timeline, or snapshot behavior.

Errors encountered:

- No build errors were encountered.

Tests performed:

- Build command:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- Runtime UDP telemetry with an ESP32 fake generator was not manually tested during this step.

## 2026-05-23 12:19:34 +02:00

Goal of this step: Add user-facing UDP connection errors matching the existing serial error flow.

Files inspected:

- `src/io/SerialPortManager.cpp`
- `ui/components/Error.qml`
- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Files changed:

- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Added a 10 second no-datagram timeout in `UDPConnection`; if listening is active and no UDP packets arrive, it sets `errorString` and emits `errorOccurred`.
- Added clearer UDP bind/socket errors through `UDPConnection`.
- Added AppController forwarding for QML validation errors via `reportConnectionError`.
- Updated Wireless UI validation so empty port input, letters/symbols, and out-of-range ports report through the same error popup path as serial errors.
- Kept UDP packet/byte/last-sender tracking internally.
- Did not connect UDP datagrams to `SerialParser` and did not change MsgPack parsing, serial, graph, model, monitor, or timeline logic.

Errors encountered:

- The first build compiled but failed at link because `appRoboticus_Data_Visualiser.exe` was still running and locked.
- Stopped the running app process and reran the same build successfully.

Tests performed:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- Runtime validation and 10 second UDP timeout behavior were not manually tested in the UI during this step.

## 2026-05-23 11:58:30 +02:00

Goal of this step: Clean up the Wireless connection bar UI by hiding internal UDP debug statistics.

Files inspected:

- `ui/components/ConnectionBar.qml`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`
- `ui/components/StyledComboBox.qml`

Files changed:

- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Removed visible Wireless UI debug fields for packets, bytes, and last sender from the main connection bar.
- Kept the UDP port field, Start/Stop Wireless Monitor button, Listening/Stopped status, and validation/error text.
- Restyled Wireless controls as a single lower row matching the existing Wired row spacing, height, green accent, dark input background, and monitor button treatment.
- Kept UDP packet, byte, and last-sender statistics in `UDPConnection`; only their visible connection-bar labels were removed.
- Did not change UDP transport logic, AppController logic, SerialParser, MsgPack parsing, SerialPortManager, graph, model, monitor, or timeline logic.
- UDP datagrams remain disconnected from `SerialParser`.

Errors encountered:

- No code or build errors were encountered during this cleanup.

Tests performed:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- The app was launched with Qt and MinGW paths on `PATH`; it was still running after 5 seconds and was then stopped.
- Visual UI inspection was not performed from this environment.

## 2026-05-23 11:36:15 +02:00

Goal of this step: Add a UDP wireless listener/status layer without feeding UDP datagrams into telemetry parsing yet.

Files inspected:

- `CMakeLists.txt`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `ui/Main.qml`
- `ui/components/ConnectionBar.qml`
- `include/io/SerialPortManager.h`
- `src/io/SerialPortManager.cpp`
- `include/io/SerialParser.h`
- `src/io/SerialParser.cpp`
- `include/parser/SerialFrameExtractor.h`
- `src/parser/SerialFrameExtractor.cpp`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Files changed:

- `CMakeLists.txt`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`
- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

Summary of changes:

- Added `UDPConnection`, a QObject/`QUdpSocket` transport layer that can bind to an IPv4 UDP port, receive datagrams, track packet/byte counts, track the last sender, expose errors, and emit `rawDataReceived(QByteArray)`.
- Added Qt Network to CMake package requirements and linked `Qt6::Network`.
- Exposed `udpConnection` through `AppController`.
- Added `AppController::startWirelessMonitor(quint16)` and `AppController::stopWirelessMonitor()`.
- Wireless start switches to wireless mode if needed, disconnects serial if active, resets the parser buffer, clears UDP statistics, and starts UDP listening.
- Wired mode stops UDP listening. Wireless mode still stops serial and does not auto-start UDP.
- Added wireless-mode controls in `ConnectionBar.qml`: UDP port field defaulting to `45454`, Start/Stop Wireless Monitor buttons, listening status, packet count, byte count, last sender, and validation/error text.
- UDP datagrams are not connected to `SerialParser`; no MsgPack-over-UDP or RoboticusDebugger-over-UDP telemetry parsing was added.
- Graph, monitor, timeline, model, serial parser, and serial port manager logic were not changed.

Errors encountered:

- Initial build exited during automatic QML type registration. The visible output only showed the target failure, with no specific qmltyperegistrar diagnostic.
- A verbose rebuild got past QML type registration but hit the 120 second command timeout while compiling QML cache/object files.
- Re-running the same build with a 300 second timeout completed successfully.

Tests performed:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

Result:

- Build completed successfully: `[100%] Built target appRoboticus_Data_Visualiser`.
- Runtime UI behavior: Not tested. The app was built but not launched.

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
