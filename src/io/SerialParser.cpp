#include "SerialParser.h"

#include <QDebug>
#include <QVariant>
#include <exception>
#include <msgpack.h>

namespace {

constexpr int MaxMsgPackNestingDepth = 64;

bool failMsgPackValidation(QString *error, const QString &message) {
  if (error) {
    *error = message;
  }
  return false;
}

bool ensureAvailable(const QByteArray &data, qsizetype offset, qsizetype count,
                     QString *error, const QString &what) {
  if (count < 0 || offset < 0 || count > data.size() - offset) {
    return failMsgPackValidation(
        error, QString("MsgPack %1 exceeds payload bounds.").arg(what));
  }
  return true;
}

bool readByte(const QByteArray &data, qsizetype &offset, quint8 *value,
              QString *error) {
  if (!ensureAvailable(data, offset, 1, error, "byte")) {
    return false;
  }
  *value = static_cast<quint8>(data.at(offset));
  ++offset;
  return true;
}

bool readUInt16(const QByteArray &data, qsizetype &offset, quint32 *value,
                QString *error) {
  if (!ensureAvailable(data, offset, 2, error, "uint16")) {
    return false;
  }
  const auto *bytes =
      reinterpret_cast<const unsigned char *>(data.constData() + offset);
  *value = (static_cast<quint32>(bytes[0]) << 8) |
           static_cast<quint32>(bytes[1]);
  offset += 2;
  return true;
}

bool readUInt32(const QByteArray &data, qsizetype &offset, quint32 *value,
                QString *error) {
  if (!ensureAvailable(data, offset, 4, error, "uint32")) {
    return false;
  }
  const auto *bytes =
      reinterpret_cast<const unsigned char *>(data.constData() + offset);
  *value = (static_cast<quint32>(bytes[0]) << 24) |
           (static_cast<quint32>(bytes[1]) << 16) |
           (static_cast<quint32>(bytes[2]) << 8) |
           static_cast<quint32>(bytes[3]);
  offset += 4;
  return true;
}

bool skipBytes(const QByteArray &data, qsizetype &offset, quint64 count,
               QString *error, const QString &what) {
  if (offset > data.size() ||
      count > static_cast<quint64>(data.size() - offset)) {
    return failMsgPackValidation(
        error, QString("MsgPack %1 of %2 byte(s) exceeds payload bounds.")
                   .arg(what)
                   .arg(count));
  }
  offset += count;
  return true;
}

bool skipMsgPackValue(const QByteArray &data, qsizetype &offset, int depth,
                      QString *error);

bool skipMsgPackArray(const QByteArray &data, qsizetype &offset, quint32 count,
                      int depth, QString *error) {
  for (quint32 i = 0; i < count; ++i) {
    if (!skipMsgPackValue(data, offset, depth + 1, error)) {
      return false;
    }
  }
  return true;
}

bool skipMsgPackMap(const QByteArray &data, qsizetype &offset, quint32 count,
                    int depth, QString *error) {
  for (quint32 i = 0; i < count; ++i) {
    if (!skipMsgPackValue(data, offset, depth + 1, error) ||
        !skipMsgPackValue(data, offset, depth + 1, error)) {
      return false;
    }
  }
  return true;
}

bool skipMsgPackValue(const QByteArray &data, qsizetype &offset, int depth,
                      QString *error) {
  if (depth > MaxMsgPackNestingDepth) {
    return failMsgPackValidation(error, "MsgPack nesting is too deep.");
  }

  quint8 marker = 0;
  if (!readByte(data, offset, &marker, error)) {
    return false;
  }

  if (marker <= 0x7f || marker >= 0xe0) {
    return true;
  }

  if (marker >= 0x80 && marker <= 0x8f) {
    return skipMsgPackMap(data, offset, marker & 0x0f, depth, error);
  }

  if (marker >= 0x90 && marker <= 0x9f) {
    return skipMsgPackArray(data, offset, marker & 0x0f, depth, error);
  }

  if (marker >= 0xa0 && marker <= 0xbf) {
    return skipBytes(data, offset, marker & 0x1f, error, "fixstr");
  }

  quint8 length8 = 0;
  quint32 length = 0;

  switch (marker) {
  case 0xc0:
  case 0xc2:
  case 0xc3:
    return true;
  case 0xc1:
    return failMsgPackValidation(error, "MsgPack marker 0xC1 is invalid.");
  case 0xc4:
    return readByte(data, offset, &length8, error) &&
           skipBytes(data, offset, length8, error, "bin8");
  case 0xc5:
    return readUInt16(data, offset, &length, error) &&
           skipBytes(data, offset, length, error, "bin16");
  case 0xc6:
    return readUInt32(data, offset, &length, error) &&
           skipBytes(data, offset, length, error, "bin32");
  case 0xc7:
    return readByte(data, offset, &length8, error) &&
           skipBytes(data, offset, static_cast<quint32>(length8) + 1, error,
                     "ext8");
  case 0xc8:
    return readUInt16(data, offset, &length, error) &&
           skipBytes(data, offset, static_cast<quint64>(length) + 1, error,
                     "ext16");
  case 0xc9:
    return readUInt32(data, offset, &length, error) &&
           skipBytes(data, offset, static_cast<quint64>(length) + 1, error,
                     "ext32");
  case 0xca:
    return skipBytes(data, offset, 4, error, "float32");
  case 0xcb:
    return skipBytes(data, offset, 8, error, "float64");
  case 0xcc:
  case 0xd0:
    return skipBytes(data, offset, 1, error, "integer");
  case 0xcd:
  case 0xd1:
    return skipBytes(data, offset, 2, error, "integer");
  case 0xce:
  case 0xd2:
    return skipBytes(data, offset, 4, error, "integer");
  case 0xcf:
  case 0xd3:
    return skipBytes(data, offset, 8, error, "integer");
  case 0xd4:
    return skipBytes(data, offset, 2, error, "fixext1");
  case 0xd5:
    return skipBytes(data, offset, 3, error, "fixext2");
  case 0xd6:
    return skipBytes(data, offset, 5, error, "fixext4");
  case 0xd7:
    return skipBytes(data, offset, 9, error, "fixext8");
  case 0xd8:
    return skipBytes(data, offset, 17, error, "fixext16");
  case 0xd9:
    return readByte(data, offset, &length8, error) &&
           skipBytes(data, offset, length8, error, "str8");
  case 0xda:
    return readUInt16(data, offset, &length, error) &&
           skipBytes(data, offset, length, error, "str16");
  case 0xdb:
    return readUInt32(data, offset, &length, error) &&
           skipBytes(data, offset, length, error, "str32");
  case 0xdc:
    return readUInt16(data, offset, &length, error) &&
           skipMsgPackArray(data, offset, length, depth, error);
  case 0xdd:
    return readUInt32(data, offset, &length, error) &&
           skipMsgPackArray(data, offset, length, depth, error);
  case 0xde:
    return readUInt16(data, offset, &length, error) &&
           skipMsgPackMap(data, offset, length, depth, error);
  case 0xdf:
    return readUInt32(data, offset, &length, error) &&
           skipMsgPackMap(data, offset, length, depth, error);
  default:
    return failMsgPackValidation(
        error, QString("MsgPack marker 0x%1 is not supported.")
                   .arg(marker, 2, 16, QLatin1Char('0'))
                   .toUpper());
  }
}

bool isMsgPackPayloadBounded(const QByteArray &data, QString *error) {
  if (data.isEmpty()) {
    return failMsgPackValidation(error, "MsgPack payload is empty.");
  }

  qsizetype offset = 0;
  while (offset < data.size()) {
    if (!skipMsgPackValue(data, offset, 0, error)) {
      return false;
    }
  }

  return offset == data.size();
}

} // namespace

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

    QString validationError;
    if (!isMsgPackPayloadBounded(data, &validationError)) {
        qWarning() << "Discarding malformed MsgPack payload:" << validationError;
        return decoded;
    }

    QVariant unpacked;
    try {
        unpacked = MsgPack::unpack(data);
    } catch (const std::exception &error) {
        qWarning() << "Discarding MsgPack payload after unpack error:" << error.what();
        return decoded;
    } catch (...) {
        qWarning() << "Discarding MsgPack payload after unknown unpack error.";
        return decoded;
    }

    if (!unpacked.isValid() || unpacked.isNull()) {
        qWarning() << "Discarding MsgPack payload: unpacked value is invalid.";
        return decoded;
    }

    const QVariantList frame = unpacked.toList();
    if (frame.size() < 3) { // sensors[1], vectors[1], timestamp[2]
        qWarning() << "Discarding MsgPack payload: decoded frame has"
                   << frame.size() << "element(s), expected at least 3.";
        return decoded;
    }

    decoded.sensors = frame.at(0).toList();
    decoded.vectors = frame.at(1).toList();
    decoded.timestamp = frame.at(2).toLongLong();

    // A frame is valid if it has data and isn't just an empty structure
    decoded.isValid = !(decoded.sensors.isEmpty() && decoded.vectors.isEmpty());

    return decoded;
}
