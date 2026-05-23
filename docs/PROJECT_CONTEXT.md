# Project Context

Last updated: 2026-05-23 13:32:27 +02:00

## Current project structure relevant to this task

- `CMakeLists.txt` defines one Qt 6 QML application target named `appRoboticus_Data_Visualiser` and links Qt Network for UDP support.
- `include/` contains C++ headers for controllers, models, IO, and parser logic.
- `src/` contains the matching C++ implementations.
- `ui/` contains QML UI files.
- `ui/components/ConnectionBar.qml` is the top connection control bar.
- `include/io/UDPConnection.h` and `src/io/UDPConnection.cpp` contain the UDP listening/status transport layer.
- `libs/qmsgpack/` is the bundled MsgPack dependency used by `SerialParser`.
- `build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug/` is the current Qt Creator generated build directory.

## Existing input data flow

Verified from `AppController`, `SerialParser`, `SerialFrameExtractor`, `SerialPortManager`, and QML files:

- Wired monitor active:
  `SerialPortManager -> SerialParser -> SerialFrameExtractor -> AppController -> models -> QML UI`
- Wireless monitor active:
  `UDPConnection -> SerialParser -> SerialFrameExtractor -> AppController -> models -> QML UI`

- `SerialPortManager` emits `rawDataReceived(QByteArray)` when serial bytes arrive.
- `UDPConnection` emits `rawDataReceived(QByteArray)` when UDP datagrams arrive.
- `AppController` owns `SerialPortManager`, `UDPConnection`, and `SerialParser`.
- `AppController` connects only the active input source to `SerialParser::onRawDataReady`.
- Wired mode keeps serial connected to the parser and stops/disconnects UDP parser input.
- Wireless monitor start disconnects serial parser input, starts UDP listening, and connects UDP datagrams to the parser only after the listener starts successfully.
- Wireless monitor stop disconnects UDP parser input and resets the parser.
- `SerialParser` appends raw bytes to `SerialFrameExtractor`, takes complete frames, emits raw MsgPack payloads through `dataReceived`, decodes MsgPack payloads, and emits `frameDecoded`.
- `AppController` connects `SerialParser::frameDecoded` to `AppController::onFrameParsed`.
- `AppController::onFrameParsed` updates the sensor and vector models and appends a snapshot.
- `ui/Main.qml` creates `SensorController`, `VectorController`, `AppController`, `ConnectionBar`, `Monitor`, `Timeline`, and `Graph`.
- `ui/Main.qml` passes `sensorController.model` and `vectorController.model` into `AppController::setModels`.
- QML graph updates are connected through `SensorController` and `VectorController` signals in `ui/Main.qml`.
- UDP datagrams must already contain the existing binary frame format: `0xFD + uint16 little-endian payload length + MsgPack payload`.
- UDP does not parse MsgPack inside `UDPConnection` and no JSON telemetry format has been added.

## Important files inspected

- `include/parser/SerialFrameExtractor.h`: declares the frame extractor and its internal `QByteArray` buffer.
- `src/parser/SerialFrameExtractor.cpp`: appends raw bytes, extracts complete framed payloads, resynchronizes on `0xFD`, and trims oversized buffers.
- `include/io/SerialParser.h`: declares the parser that receives raw bytes and emits decoded frames.
- `src/io/SerialParser.cpp`: uses `SerialFrameExtractor`, unpacks MsgPack payloads, validates decoded frames, and emits parser signals.
- `include/io/SerialPortManager.h`: declares serial connection state, port selection, baud selection, connect/disconnect methods, and `rawDataReceived`.
- `src/io/SerialPortManager.cpp`: refreshes available ports, opens/closes the serial port, reads raw data, and emits errors.
- `include/controllers/AppController.h`: declares the central controller exposed to QML.
- `src/controllers/AppController.cpp`: wires serial input to parser output, updates models from decoded frames, and manages snapshots.
- `ui/Main.qml`: creates the main QML object graph and passes dependencies between UI components.
- `ui/components/ConnectionBar.qml`: contains the COM port, baud rate, and Start Monitor serial controls.
- `CMakeLists.txt`: lists the Qt target, QML files, C++ sources, includes, and linked libraries.
- `include/io/UDPConnection.h`: declares the UDP transport/status QObject exposed through `AppController`.
- `src/io/UDPConnection.cpp`: binds a `QUdpSocket`, receives datagrams, and updates packet statistics.
- `src/controllers/AppController.cpp`: owns the mutually exclusive parser input routing for wired serial and wireless UDP.
- `README.md`: documents the high-level app purpose, serial frame protocol, and snapshot format.

## Existing framed data/parser architecture

Verified details only:

- A frame starts with byte `0xFD`.
- The next 2 bytes are a little-endian payload size.
- The payload is a MsgPack-encoded byte array.
- `SerialFrameExtractor::appendData` stores incoming bytes in an internal buffer.
- If the buffer exceeds 65536 bytes, the extractor keeps the last 4096 bytes and tries to resynchronize to the next `0xFD` byte.
- `SerialFrameExtractor::takeCompleteFrames` removes bytes until a start byte is found.
- Payload sizes of `0` or greater than `10000` are treated as invalid and the extractor drops one byte to continue resynchronizing.
- Incomplete frames remain in the internal buffer.
- `SerialParser::decodeMsgPackFrame` expects the unpacked MsgPack value to convert to a list with at least three entries: sensors, vectors, and timestamp.
- A decoded frame is considered valid when sensors and vectors are not both empty.

## What changed in this step

- Added `SerialFrameExtractor::reset()` to clear the extractor's internal byte buffer.
- Added `SerialParser::reset()` to call `SerialFrameExtractor::reset()`.
- Added `AppController::connectionMode` with values `"wired"` and `"wireless"`, defaulting to `"wired"`.
- Added `AppController::switchToWiredMode()` and `AppController::switchToWirelessMode()`.
- `switchToWiredMode()` resets the parser buffer.
- `switchToWirelessMode()` disconnects the serial port if connected and resets the parser buffer.
- Passed `appController` into `ConnectionBar.qml` from `Main.qml`.
- Added Wired and Wireless buttons to `ConnectionBar.qml`.
- Hid and disabled the existing COM port, baud rate, and Start Monitor controls while wireless mode is active.
- Added this context document and `docs/IMPLEMENTATION_LOG.md`.
- Layout-only update on 2026-05-23: `ConnectionBar.qml` now places Wired/Wireless mode buttons in a separate top row.
- The serial controls remain in a separate lower row and are still only visible/enabled in wired mode.
- No C++ logic, parser logic, serial logic, graph, monitor, timeline, or UDP code was changed in the layout-only update.
- UDP update on 2026-05-23: added `UDPConnection` as a Qt Network/`QUdpSocket` listener with QML-visible listening state, port, packet count, byte count, last sender, and error string.
- Added `AppController::udpConnection`, `startWirelessMonitor(quint16)`, and `stopWirelessMonitor()`.
- Wireless mode stops serial and does not auto-start UDP. Wired mode stops UDP.
- `ConnectionBar.qml` shows UDP controls in wireless mode with default port `45454`, start/stop button, listening/stopped status, and validation errors.
- Wireless UI cleanup on 2026-05-23: visible debug fields for packets, bytes, and last sender were removed from the main connection bar.
- UDP packet, byte, and last-sender statistics remain tracked internally in `UDPConnection`.
- Wireless telemetry update on 2026-05-23: `AppController::startWirelessMonitor()` now connects `UDPConnection::rawDataReceived(QByteArray)` to `SerialParser::onRawDataReady(QByteArray)` after UDP listening starts successfully.
- `AppController` disconnects the inactive parser input source during wired/wireless transitions so serial and UDP do not feed `SerialParser` at the same time.
- UDP datagrams are treated as existing framed binary data. `UDPConnection` still does not parse MsgPack or create a separate telemetry format.
- No UDP transport logic, parser logic, MsgPack-over-UDP, RoboticusDebugger telemetry-over-UDP, graph, monitor, model, or timeline behavior was changed in the wireless UI cleanup.
- UDP connection error update on 2026-05-23: invalid UDP port input now emits user-facing errors through `AppController`.
- `UDPConnection` now emits an error if no UDP packets are received within 10 seconds after listening starts or after the previous packet.
- UDP bind and socket errors are still exposed through `UDPConnection::errorString` and `errorOccurred`.
- UDP packet, byte, and last-sender statistics remain internal.
- Added minimal debug logs for UDP datagram size, UDP datagram first byte, wireless bytes reaching `SerialParser`, and successful `SerialParser::frameDecoded` emission.
- ConnectionBar UI fix on 2026-05-23: active Wired/Wireless buttons now use green background with dark text, inactive mode buttons keep dark background with white text.
- Wireless controls now share the same row margins, height, and spacing constants as the wired controls; the UDP field, Start/Stop Wireless Monitor button, and status text align across the full row.
- ConnectionBar no longer displays `UDPConnection::errorString` as an inline wireless error; inline wireless errors are now limited to the validation/start errors assigned by the bar itself.
- Main UI suppresses the no-datagram UDP timeout popup message so the previous 10 second "No UDP packets..." warning is no longer visible, while other connection errors remain visible.

## Why the change was made

The change prepares the UI for a future wireless input path while preserving the existing wired serial mode. Resetting the parser buffer during mode switches prevents incomplete bytes from one input mode from being reused after a mode change.

The latest wireless telemetry change allows an ESP32 fake telemetry generator to send valid existing binary frames over UDP before real robot firmware is ready. It intentionally reuses the existing parser/model/UI path instead of adding new parsing in `UDPConnection`.

## Files modified

- `include/parser/SerialFrameExtractor.h`
- `src/parser/SerialFrameExtractor.cpp`
- `include/io/SerialParser.h`
- `src/io/SerialParser.cpp`
- `include/controllers/AppController.h`
- `src/controllers/AppController.cpp`
- `include/io/UDPConnection.h`
- `src/io/UDPConnection.cpp`
- `src/io/SerialParser.cpp`
- `CMakeLists.txt`
- `ui/Main.qml`
- `ui/components/ConnectionBar.qml`
- `docs/PROJECT_CONTEXT.md`
- `docs/IMPLEMENTATION_LOG.md`

## Files intentionally not modified

- `include/io/SerialPortManager.h`
- `src/io/SerialPortManager.cpp`
- MsgPack parsing behavior in `src/io/SerialParser.cpp`, except for adding `reset()` and a debug log before `frameDecoded`.
- Model files under `include/models/` and `src/models/`.
- Graph QML files under `ui/components/Graph/`.
- Monitor QML files under `ui/components/Monitor/`.
- Timeline QML files under `ui/components/Timeline/`.

## Build errors encountered and how they were fixed

- The earlier documented `E:/QT/Tools/CMake_64/bin/cmake.exe` path was not available in the 2026-05-23 shell.
- The earlier documented `build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug` build directory was not present.
- The current build cache is under `build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug` and points to Qt tools under `C:/Qt`.
- During the UDP update, an initial build stopped at automatic QML type registration without a useful diagnostic. A verbose rerun passed that step but hit the 120 second command timeout while compiling. A final build with a longer timeout completed successfully.
- During the wireless UI cleanup, the same build command completed successfully and regenerated the `ConnectionBar.qml` QML cache.
- During the UDP connection error update, the first build reached the link step but failed because `appRoboticus_Data_Visualiser.exe` was still running. After stopping that process, the same build command completed successfully.
- The successful build command was:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

## Runtime/UI errors encountered and how they were fixed

The app was launched briefly after the wireless UI cleanup and was still running after 5 seconds, then it was stopped. Visual UI inspection was not performed from this environment.

## Current status after the change

- The requested wired/wireless UI mode switch exists.
- Wireless mode disconnects serial input and shows UDP listener controls.
- UDP listening now feeds existing binary frames into telemetry parsing only while the wireless monitor is active.
- Wireless mode no longer shows packets, bytes, or last sender in the main connection bar.
- Wireless UDP validation errors are shown for empty ports, letters/symbols, and ports outside `1..65535`.
- Wireless UDP listening no longer shows the old 10 second no-packets timeout warning in the UI.
- Wireless UDP datagrams now feed the existing parser only while the wireless monitor is active.
- Wired mode keeps serial behavior intact and disconnects/stops UDP parser input.
- Wired serial behavior is preserved and wired mode stops UDP listening if active.
- The project builds successfully with the 2026-05-23 command listed in `docs/IMPLEMENTATION_LOG.md`.

## Next planned step

Test with an ESP32 fake telemetry generator that sends valid existing binary frames over UDP. Add real robot UART bridge firmware later.
