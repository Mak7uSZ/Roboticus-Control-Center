#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QUrl>

#include "include/io/SerialParser.h"
#include "include/io/SerialPortManager.h"
#include "include/io/UDPConnection.h"
#include "include/models/SensorModel.h"
#include "include/models/VectorModel.h"
#include "include/parser/FrameTypes.h"
#include "include/parser/SnapshotLoader.h"
#include "include/parser/SnapshotStore.h"

/**
 * @brief Central controller that wires together the serial port, parser,
 *        data models, snapshot store, and file I/O.
 *
 *        Owns the SerialPortManager and SerialParser. Holds weak (non-owning)
 *        pointers to the SensorModel and VectorModel, which are set after
 *        construction via setModels().
 */
class AppController : public QObject {
    Q_OBJECT
    QML_ELEMENT

    /** @brief Exposes the port manager to QML so bindings like isConnected work. */
    Q_PROPERTY(SerialPortManager *portManager READ portManager CONSTANT)

    /** @brief Exposes the parser to QML. */
    Q_PROPERTY(SerialParser *parser READ parser CONSTANT)

    /** @brief Exposes the UDP transport status/control object to QML. */
    Q_PROPERTY(UDPConnection *udpConnection READ udpConnection CONSTANT)

    /** @brief Total number of stored snapshots; updates whenever a frame arrives or a file is loaded. */
    Q_PROPERTY(int snapshotCount READ snapshotCount NOTIFY snapshotsChanged)

    /** @brief Active input mode. Supported values are "wired" and "wireless". */
    Q_PROPERTY(QString connectionMode READ connectionMode NOTIFY connectionModeChanged)

public:
    explicit AppController(QObject *parent = nullptr);

    SerialPortManager *portManager() const { return m_portManager; }
    SerialParser *parser() const { return m_parser; }
    UDPConnection *udpConnection() const { return m_udpConnection; }
    int snapshotCount() const { return m_snapshotStore.count(); }
    QString connectionMode() const { return m_connectionMode; }

    /**
     * @brief Registers the active sensor and vector models with the controller.
     *        Must be called before any frames are processed.
     *        Does not take ownership of the models.
     */
    Q_INVOKABLE void setModels(SensorModel *sensorModel, VectorModel *vectorModel);

    /**
     * @brief Serializes all stored snapshots to a JSON file.
     * @param filePath Destination file as a QUrl.
     * @return True on success; emits errorOccurred on failure.
     */
    Q_INVOKABLE bool saveToFile(QUrl filePath);

    /**
     * @brief Loads snapshots from a JSON file, replacing any existing store.
     * @param filePath Source file as a QUrl.
     * @return True on success; emits errorOccurred on failure.
     */
    Q_INVOKABLE bool loadFromFile(QUrl filePath);

    /**
     * @brief Restores the sensor and vector models to the state captured at
     *        the given snapshot index.
     * @param index Zero-based index into the snapshot store.
     * @return True if the index was valid and models were restored.
     */
    Q_INVOKABLE bool restoreToIndex(int index);

    /**
     * @brief Returns the timestamp of the snapshot at the given index.
     * @return Timestamp in milliseconds, or -1 if out of range.
     */
    Q_INVOKABLE qint64 timestampAt(int index) const;

    /** @brief Returns all stored timestamps in order of arrival. */
    Q_INVOKABLE QList<qint64> availableTimestamps() const;

    /** @brief Switches to serial COM-port input mode. */
    Q_INVOKABLE void switchToWiredMode();

    /** @brief Switches to wireless input mode and closes any open serial port. */
    Q_INVOKABLE void switchToWirelessMode();

    /** @brief Starts UDP listening in wireless mode without parsing datagrams. */
    Q_INVOKABLE bool startWirelessMonitor(int port);

    /** @brief Stops UDP listening if active. */
    Q_INVOKABLE void stopWirelessMonitor();

    /** @brief Emits a user-facing connection error from QML validation. */
    Q_INVOKABLE void reportConnectionError(const QString &message);

signals:
    /** @brief Emitted whenever the snapshot store changes (frame received or file loaded). */
    void snapshotsChanged();

    /** @brief Emitted when connectionMode changes. */
    void connectionModeChanged();

    /** @brief Emitted with a human-readable message when any operation fails. */
    void errorOccurred(const QString &message);

private slots:
    /**
     * @brief Receives a decoded frame from the parser, updates the models,
     *        and appends a snapshot to the store.
     */
    void onFrameParsed(const DecodedFrame &frame);

    /** @brief Clears the snapshot store when a new port connection is established. */
    void onPortConnectionChanged();

private:
    SerialPortManager *m_portManager = nullptr;
    SerialParser *m_parser = nullptr;
    UDPConnection *m_udpConnection = nullptr;
    QString m_connectionMode = QStringLiteral("wired");
    SnapshotStore m_snapshotStore;
    SnapshotLoader m_snapshotLoader;

    // Non-owning pointers; lifetime managed by the QML engine
    SensorModel *m_sensorModel = nullptr;
    VectorModel *m_vectorModel = nullptr;

    /** @brief Dispatches frame data to the sensor and vector model updaters. */
    void updateModelsFromFrame(const DecodedFrame &frame);

    /**
     * @brief Parses the raw sensor list from a frame and updates or inserts
     *        entries in the sensor model.
     *        Expected element order: [name, input, isTriggered, threshold, layer, x, y]
     */
    void updateSensorsFromFrame(const QVariantList &sensors);

    /**
     * @brief Parses the raw vector list from a frame and updates or inserts
     *        entries in the vector model.
     *        Expected element order: [name, rotation, color, layer, x, y]
     */
    void updateVectorsFromFrame(const QVariantList &vectors);

    /** @brief Rounds a float to 2 decimal places to reduce noise in incoming data. */
    double roundTo2Decimals(float val) { return qRound(val * 100) / 100.0; }
};
