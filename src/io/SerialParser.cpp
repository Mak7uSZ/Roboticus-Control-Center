#include "SerialParser.h"

#include <QVariant>
#include <msgpack.h>

SerialParser::SerialParser(QObject *parent) : QObject(parent) {}

void SerialParser::reset() {
  m_frameExtractor.reset();
}

void SerialParser::onRawDataReady(const QByteArray &data) {
  m_frameExtractor.appendData(data);
  const QList<QByteArray> frames = m_frameExtractor.takeCompleteFrames();
  for (const QByteArray &msgpackData : frames) {
    emit dataReceived(msgpackData);
    processMsgPackData(msgpackData);
  }
}

void SerialParser::processMsgPackData(const QByteArray &data) {
  const DecodedFrame frame = decodeMsgPackFrame(data);
  if (!frame.isValid) {
    return;
  }
  emit frameDecoded(frame);
}


DecodedFrame SerialParser::decodeMsgPackFrame(const QByteArray &data) const {
    DecodedFrame decoded;

    const QVariant unpacked = MsgPack::unpack(data);
    if (!unpacked.isValid() || unpacked.isNull()) {
        return decoded;
    }

    const QVariantList frame = unpacked.toList();
    if (frame.size() < 3) { // sensors[1], vectors[1], timestamp[2]
        return decoded;
    }

    decoded.sensors = frame.at(0).toList();
    decoded.vectors = frame.at(1).toList();
    decoded.timestamp = frame.at(2).toLongLong();

    // A frame is valid if it has data and isn't just an empty structure
    decoded.isValid = !(decoded.sensors.isEmpty() && decoded.vectors.isEmpty());

    return decoded;
}
