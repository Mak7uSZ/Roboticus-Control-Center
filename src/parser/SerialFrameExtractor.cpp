#include "parser/SerialFrameExtractor.h"


void SerialFrameExtractor::appendData(const QByteArray &chunk)
{
    m_buffer.append(chunk);

    constexpr int maxBufferSize = 65536;
    if (m_buffer.size() > maxBufferSize) {
        m_buffer = m_buffer.right(4096);
        const int syncPos = m_buffer.indexOf(static_cast<char>(0xFD));
        if (syncPos > 0) {
            m_buffer.remove(0, syncPos);
        } else if (syncPos < 0) {
            m_buffer.clear();
        }
    }
}


QList<QByteArray> SerialFrameExtractor::takeCompleteFrames()
{
    QList<QByteArray> frames;

    while (m_buffer.size() >= 3) {
        if (static_cast<uint8_t>(m_buffer.at(0)) != 0xFD) {
            m_buffer.remove(0, 1);
            continue;
        }

        const auto *ptr =
            reinterpret_cast<const unsigned char *>(m_buffer.constData());
        const quint16 payloadSize = ptr[1] | (ptr[2] << 8);

        if (payloadSize == 0 || payloadSize > 10000) {
            m_buffer.remove(0, 1);
            continue;
        }

        if (m_buffer.size() < (3 + payloadSize)) {
            break;
        }

        frames.append(m_buffer.mid(3, payloadSize));
        m_buffer.remove(0, 3 + payloadSize);
    }

    return frames;
}


void SerialFrameExtractor::reset()
{
    m_buffer.clear();
}
