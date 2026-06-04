#pragma once

#include <QByteArray>
#include <QList>

class SerialFrameExtractor
{
public:

    /**
     * @brief Appends incoming data to the internal buffer and trims it if it
     *        exceeds the maximum size, re-syncing to the next 0xFD start byte.
     * @param chunk Raw bytes received from the serial port.
     */
    void appendData(const QByteArray &chunk);


    /**
     * @brief Parses the buffer and extracts all complete frames.
     *        A frame starts with 0xFD, followed by a 2-byte little-endian payload
     *        size, then the payload bytes. Incomplete frames remain in the buffer.
     * @return List of complete payload byte arrays, in order of arrival.
     */
    QList<QByteArray> takeCompleteFrames();

    /** @brief Clears any incomplete bytes currently buffered. */
    void reset();

private:
    QByteArray m_buffer;
};
