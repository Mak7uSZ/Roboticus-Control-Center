#include "SerialParser.h"
#include "SerialPortManager.h"
#include "UDPConnection.h"

#include <QElapsedTimer>
#include <QHostAddress>
#include <QSignalSpy>
#include <QVariantList>
#include <QUdpSocket>
#include <QtTest/QtTest>
#include <msgpack.h>

namespace {

QByteArray makeFrameDatagram(const QByteArray &payload) {
  QByteArray datagram;
  datagram.resize(3);
  datagram[0] = static_cast<char>(0xFD);
  datagram[1] = static_cast<char>(payload.size() & 0xFF);
  datagram[2] = static_cast<char>((payload.size() >> 8) & 0xFF);
  datagram.append(payload);
  return datagram;
}

QByteArray makeValidPayload() {
  QVariantList sensor;
  sensor << QStringLiteral("front-bumper") << 1.25 << true << 0.75
         << QStringLiteral("Layer 1") << 12.0 << 34.0;

  QVariantList sensors;
  sensors << QVariant(sensor);

  QVariantList vectors;

  QVariantList frame;
  frame << QVariant(sensors) << QVariant(vectors)
        << QVariant::fromValue<qlonglong>(1234);

  return MsgPack::pack(frame);
}

quint16 unusedUdpPort() {
  QUdpSocket socket;
  if (!socket.bind(QHostAddress::LocalHost, 0)) {
    return 0;
  }

  const quint16 port = socket.localPort();
  socket.close();
  return port;
}

bool sendToConnection(UDPConnection &connection, const QByteArray &datagram) {
  QUdpSocket sender;
  const qint64 written = sender.writeDatagram(
      datagram, QHostAddress::LocalHost, connection.port());
  return written == datagram.size();
}

} // namespace

class CommunicationRobustnessTest : public QObject {
  Q_OBJECT

private slots:
  void validFrameDatagram();
  void datagramNotStartingWithFrameByte();
  void datagramSizeDoesNotMatchHeader();
  void tooSmallDatagram();
  void oversizedDatagram();
  void corruptedMsgPackPayload();
  void comPortUnavailable();
};

void CommunicationRobustnessTest::validFrameDatagram() {
  UDPConnection connection;
  SerialParser parser;

  int decodedFrameCount = 0;
  DecodedFrame decodedFrame;
  connect(&connection, &UDPConnection::rawDataReceived, &parser,
          &SerialParser::processMsgPackData);
  connect(&parser, &SerialParser::frameDecoded, this,
          [&](const DecodedFrame &frame) {
            decodedFrame = frame;
            ++decodedFrameCount;
          });

  QSignalSpy payloadSpy(&connection, &UDPConnection::rawDataReceived);
  QVERIFY(payloadSpy.isValid());

  const quint16 port = unusedUdpPort();
  QVERIFY(port != 0);
  QVERIFY(connection.startListening(port));

  const QByteArray payload = makeValidPayload();
  QVERIFY(sendToConnection(connection, makeFrameDatagram(payload)));

  QTRY_COMPARE(payloadSpy.count(), 1);
  QTRY_COMPARE(decodedFrameCount, 1);
  QCOMPARE(payloadSpy.takeFirst().at(0).toByteArray(), payload);
  QVERIFY(decodedFrame.isValid);
  QCOMPARE(decodedFrame.sensors.size(), 1);
  QCOMPARE(decodedFrame.timestamp, qint64(1234));
}

void CommunicationRobustnessTest::datagramNotStartingWithFrameByte() {
  UDPConnection connection;
  QSignalSpy payloadSpy(&connection, &UDPConnection::rawDataReceived);
  QVERIFY(payloadSpy.isValid());

  const quint16 port = unusedUdpPort();
  QVERIFY(port != 0);
  QVERIFY(connection.startListening(port));

  const QByteArray datagram("\x00\x00\x00", 3);
  QVERIFY(sendToConnection(connection, datagram));

  QTRY_COMPARE(connection.packetsReceived(), quint64(1));
  QTest::qWait(25);
  QCOMPARE(payloadSpy.count(), 0);
}

void CommunicationRobustnessTest::datagramSizeDoesNotMatchHeader() {
  UDPConnection connection;
  QSignalSpy payloadSpy(&connection, &UDPConnection::rawDataReceived);
  QVERIFY(payloadSpy.isValid());

  const quint16 port = unusedUdpPort();
  QVERIFY(port != 0);
  QVERIFY(connection.startListening(port));

  QByteArray datagram;
  datagram.resize(4);
  datagram[0] = static_cast<char>(0xFD);
  datagram[1] = static_cast<char>(10);
  datagram[2] = static_cast<char>(0);
  datagram[3] = static_cast<char>(0x90);
  QVERIFY(sendToConnection(connection, datagram));

  QTRY_COMPARE(connection.packetsReceived(), quint64(1));
  QTest::qWait(25);
  QCOMPARE(payloadSpy.count(), 0);
}

void CommunicationRobustnessTest::tooSmallDatagram() {
  UDPConnection connection;
  QSignalSpy payloadSpy(&connection, &UDPConnection::rawDataReceived);
  QVERIFY(payloadSpy.isValid());

  const quint16 port = unusedUdpPort();
  QVERIFY(port != 0);
  QVERIFY(connection.startListening(port));

  const QByteArray datagram("\xFD\x01", 2);
  QVERIFY(sendToConnection(connection, datagram));

  QTRY_COMPARE(connection.packetsReceived(), quint64(1));
  QTest::qWait(25);
  QCOMPARE(payloadSpy.count(), 0);
}

void CommunicationRobustnessTest::oversizedDatagram() {
  UDPConnection connection;
  QSignalSpy payloadSpy(&connection, &UDPConnection::rawDataReceived);
  QVERIFY(payloadSpy.isValid());

  const quint16 port = unusedUdpPort();
  QVERIFY(port != 0);
  QVERIFY(connection.startListening(port));

  QByteArray payload;
  payload.resize(UDPConnection::MaxAcceptedDatagramSize - 2);
  payload.fill(static_cast<char>(0));

  QVERIFY(sendToConnection(connection, makeFrameDatagram(payload)));

  QTRY_COMPARE(connection.packetsReceived(), quint64(1));
  QTest::qWait(25);
  QCOMPARE(payloadSpy.count(), 0);
}

void CommunicationRobustnessTest::corruptedMsgPackPayload() {
  UDPConnection connection;
  SerialParser parser;

  int decodedFrameCount = 0;
  connect(&connection, &UDPConnection::rawDataReceived, &parser,
          &SerialParser::processMsgPackData);
  connect(&parser, &SerialParser::frameDecoded, this,
          [&](const DecodedFrame &) { ++decodedFrameCount; });

  QSignalSpy payloadSpy(&connection, &UDPConnection::rawDataReceived);
  QVERIFY(payloadSpy.isValid());

  const quint16 port = unusedUdpPort();
  QVERIFY(port != 0);
  QVERIFY(connection.startListening(port));

  const QByteArray corruptedPayload("\xDB\x00\x00\x00\x10", 5);
  QVERIFY(sendToConnection(connection, makeFrameDatagram(corruptedPayload)));

  QTRY_COMPARE(payloadSpy.count(), 1);
  QTest::qWait(25);
  QCOMPARE(decodedFrameCount, 0);
}

void CommunicationRobustnessTest::comPortUnavailable() {
  SerialPortManager manager;
  QSignalSpy errorSpy(&manager, &SerialPortManager::errorOccurred);
  QVERIFY(errorSpy.isValid());

  QVERIFY(manager.setBaudRate(115200));
  QVERIFY(manager.setComPort(QStringLiteral("COM_DOES_NOT_EXIST_9999")));

  QElapsedTimer timer;
  timer.start();
  QVERIFY(!manager.connectToPort());
  QVERIFY(timer.elapsed() < 2000);
  QVERIFY(!manager.isConnected());
  QVERIFY(errorSpy.count() > 0);

  const int errorsAfterFirstAttempt = errorSpy.count();
  QVERIFY(!manager.connectToPort());
  QVERIFY(!manager.isConnected());
  QVERIFY(errorSpy.count() > errorsAfterFirstAttempt);
}

QTEST_MAIN(CommunicationRobustnessTest)

#include "CommunicationRobustnessTest.moc"
