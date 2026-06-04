#pragma once

#include "../parser/FrameTypes.h"
#include "../parser/SerialFrameExtractor.h"
#include <QByteArray>

/**
 * @brief Receives raw serial bytes, extracts framed packets, and decodes
 *        them from MsgPack into DecodedFrame structs for the rest of the app.
 */
class SerialParser : public QObject {
    Q_OBJECT

public:
    explicit SerialParser(QObject *parent = nullptr);

    /** @brief Clears any incomplete frame bytes waiting in the extractor. */
    void reset();

signals:
    /** @brief Emitted when a complete frame has been successfully decoded. */
    void frameDecoded(const DecodedFrame &frame);

    /**
     * @brief Emitted with the raw MsgPack bytes of each extracted frame,
     *        before decoding, e.g. for logging or replay.
     */
    void dataReceived(const QByteArray &data);

public slots:
    /**
     * @brief Feeds incoming raw serial bytes into the frame extractor.
     *        Any complete frames found are decoded and emitted via frameDecoded.
     * @param data Raw bytes received from the serial port.
     */
    void onRawDataReady(const QByteArray &data);

private:
    SerialFrameExtractor m_frameExtractor;

    /**
     * @brief Decodes a MsgPack payload and emits frameDecoded if valid.
     * @param data Raw MsgPack bytes representing a single frame.
     */
    void processMsgPackData(const QByteArray &data);

    /**
     * @brief Unpacks a MsgPack byte array into a DecodedFrame.
     *        Expected format: [sensors, vectors, timestamp].
     * @param data Raw MsgPack bytes to decode.
     * @return The decoded frame. Check isValid before using.
     */
    DecodedFrame decodeMsgPackFrame(const QByteArray &data) const;
};
