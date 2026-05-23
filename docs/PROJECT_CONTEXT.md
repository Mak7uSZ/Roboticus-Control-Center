# Project Context

Last updated: 2026-05-23 11:11:34 +02:00

## Current project structure relevant to this task

- `CMakeLists.txt` defines one Qt 6 QML application target named `appRoboticus_Data_Visualiser`.
- `include/` contains C++ headers for controllers, models, IO, and parser logic.
- `src/` contains the matching C++ implementations.
- `ui/` contains QML UI files.
- `ui/components/ConnectionBar.qml` is the top connection control bar.
- `libs/qmsgpack/` is the bundled MsgPack dependency used by `SerialParser`.
- `build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug/` is the current Qt Creator generated build directory.

## Existing serial data flow

Verified from `AppController`, `SerialParser`, `SerialFrameExtractor`, `SerialPortManager`, and QML files:

`SerialPortManager -> SerialParser -> SerialFrameExtractor -> AppController -> models -> QML UI`

- `SerialPortManager` emits `rawDataReceived(QByteArray)` when serial bytes arrive.
- `AppController` owns `SerialPortManager` and `SerialParser`.
- `AppController` connects `SerialPortManager::rawDataReceived` to `SerialParser::onRawDataReady`.
- `SerialParser` appends raw bytes to `SerialFrameExtractor`, takes complete frames, emits raw MsgPack payloads through `dataReceived`, decodes MsgPack payloads, and emits `frameDecoded`.
- `AppController` connects `SerialParser::frameDecoded` to `AppController::onFrameParsed`.
- `AppController::onFrameParsed` updates the sensor and vector models and appends a snapshot.
- `ui/Main.qml` creates `SensorController`, `VectorController`, `AppController`, `ConnectionBar`, `Monitor`, `Timeline`, and `Graph`.
- `ui/Main.qml` passes `sensorController.model` and `vectorController.model` into `AppController::setModels`.
- QML graph updates are connected through `SensorController` and `VectorController` signals in `ui/Main.qml`.

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

## Why the change was made

The change prepares the UI for a future wireless input path while preserving the existing wired serial mode. Resetting the parser buffer during mode switches prevents incomplete bytes from one input mode from being reused after a mode change.

## Files modified

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

## Files intentionally not modified

- `include/io/SerialPortManager.h`
- `src/io/SerialPortManager.cpp`
- MsgPack parsing behavior in `src/io/SerialParser.cpp`, except for adding `reset()`.
- Model files under `include/models/` and `src/models/`.
- Graph QML files under `ui/components/Graph/`.
- Monitor QML files under `ui/components/Monitor/`.
- Timeline QML files under `ui/components/Timeline/`.
- `CMakeLists.txt`

## Build errors encountered and how they were fixed

- The earlier documented `E:/QT/Tools/CMake_64/bin/cmake.exe` path was not available in the 2026-05-23 shell.
- The earlier documented `build/Desktop_Qt_6_11_1_MinGW_64_bit_Debug` build directory was not present.
- The current build cache is under `build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug` and points to Qt tools under `C:/Qt`.
- The successful build command was:

```powershell
$env:Path = "C:\Qt\Tools\mingw1310_64\bin;$env:Path"; & C:/Qt/Tools/CMake_64/bin/cmake.exe --build build/Desktop_Qt_6_11_1_MinGW_64_bit-Debug
```

## Runtime/UI errors encountered and how they were fixed

Not confirmed yet. The desktop app was not launched during this step. QML compilation completed as part of the successful build.

## Current status after the change

- The requested wired/wireless UI mode switch exists.
- Wireless mode currently only disconnects serial input and hides/disables serial controls.
- No UDP or wireless input implementation has been added.
- The project builds successfully with the 2026-05-23 command listed in `docs/IMPLEMENTATION_LOG.md`.

## Next planned step

Add `UDPConnection` later as a second raw byte input source that emits `rawDataReceived(QByteArray)` into the existing parser.
