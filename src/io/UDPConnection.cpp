#include "UDPConnection.h"

#include <QDebug>
#include <QHostAddress>
#include <QNetworkDatagram>
#include <QUdpSocket>

UDPConnection::UDPConnection(QObject *parent) : QObject(parent) {
}

bool UDPConnection::extractPayloadFromDatagram(const QByteArray &datagram,
                                               QByteArray *payload,
                                               QString *errorMessage) {
  if (payload) {
    payload->clear();
  }

  const int datagramSize = datagram.size();

  auto reject = [errorMessage](const QString &message) {
    if (errorMessage) {
      *errorMessage = message;
    }
    return false;
  };

  if (datagramSize < 3) {
    return reject(QString("UDP datagram is too small for a frame header: %1 byte(s).")
                      .arg(datagramSize));
  }

  const auto *bytes =
      reinterpret_cast<const unsigned char *>(datagram.constData());

  if (bytes[0] != FrameStartByte) {
    const QString actualStartByte =
        QString("0x%1").arg(bytes[0], 2, 16, QLatin1Char('0')).toUpper();
    return reject(QString("UDP datagram does not start with frame byte 0xFD "
                          "(got %1).")
                      .arg(actualStartByte));
  }

  const int payloadLength = bytes[1] | (bytes[2] << 8);
  const int expectedDatagramSize = 3 + payloadLength;

  if (datagramSize != expectedDatagramSize) {
    return reject(QString("UDP frame size mismatch: datagram has %1 byte(s), "
                          "header declares %2 payload byte(s), expected %3 byte(s).")
                      .arg(datagramSize)
                      .arg(payloadLength)
                      .arg(expectedDatagramSize));
  }

  if (datagramSize > MaxAcceptedDatagramSize) {
    return reject(QString("UDP frame is too large: %1 byte(s), maximum accepted "
                          "datagram size is %2 byte(s).")
                      .arg(datagramSize)
                      .arg(MaxAcceptedDatagramSize));
  }

  if (payload) {
    *payload = datagram.mid(3, payloadLength);
  }
  return true;
}

bool UDPConnection::startListening(quint16 port) {
  if (port < 1 || port > 65535 ) {
    emit errorOccurred("UDP port must be between 1 and 65535.");
    return false;
  }

  if (m_listening && m_port == port) {
    return true;
  }

  if (m_listening) {
    stopListening();
  }

  if (!m_socket) {
    m_socket = new QUdpSocket(this);
  }

  const bool bound =
      m_socket->bind(QHostAddress::AnyIPv4, port,
                     QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

  if (!bound) {
      emit errorOccurred(QString("Failed to listen on UDP port %1: %2")
                             .arg(port)
                             .arg(m_socket->errorString()));
    return false;
  }

  connect(m_socket, &QUdpSocket::readyRead, this,
          &UDPConnection::readPendingDatagrams, Qt::UniqueConnection);
  connect(m_socket, &QUdpSocket::errorOccurred, this,
          &UDPConnection::handleSocketError, Qt::UniqueConnection);

  const bool listeningChangedNeeded = !m_listening;
  const bool portChangedNeeded = m_port != port;

  m_port = port;
  m_listening = true;

  if (portChangedNeeded) {
    emit portChanged();
  }
  if (listeningChangedNeeded) {
    emit listeningChanged();
  }

  return true;
}

void UDPConnection::stopListening() {
  const bool wasListening = m_listening;

  if (m_socket) {
    disconnect(m_socket, &QUdpSocket::readyRead, this,
               &UDPConnection::readPendingDatagrams);
    m_socket->close();
  }

  m_listening = false;

  if (wasListening) {
    emit listeningChanged();
  }
}

void UDPConnection::clearStatistics() {
  m_packetsReceived = 0;
  m_bytesReceived = 0;
  m_lastSenderAddress.clear();
  m_lastSenderPort = 0;
}

void UDPConnection::readPendingDatagrams() {
  if (!m_socket) {
    return;
  }

  while (m_socket->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = m_socket->receiveDatagram();
    if (!datagram.isValid()) {
      qWarning() << "Received an invalid UDP datagram.";
      continue;
    }

    const QByteArray data = datagram.data();

    ++m_packetsReceived;
    m_bytesReceived += static_cast<quint64>(data.size());
    m_lastSenderAddress = datagram.senderAddress().toString();
    m_lastSenderPort = datagram.senderPort();

    QByteArray payload;
    QString validationError;
    if (!extractPayloadFromDatagram(data, &payload, &validationError)) {
      qWarning().noquote()
          << QString("Discarding UDP datagram from %1:%2: %3")
                 .arg(m_lastSenderAddress)
                 .arg(m_lastSenderPort)
                 .arg(validationError);
      continue;
    }

    qDebug() << "Accepted UDP frame datagram size" << data.size()
             << "payload size" << payload.size();
    emit rawDataReceived(payload);
  }
}

void UDPConnection::handleNoDatagramsTimeout() {
  if (!m_listening) {
    return;
  }

  emit errorOccurred(
      QString("No UDP packets were received on port %1 within 10 seconds.")
          .arg(m_port));
}

void UDPConnection::handleSocketError(QAbstractSocket::SocketError error) {
  if (error == QAbstractSocket::UnknownSocketError || !m_socket) {
    return;
  }

  emit errorOccurred(
      QString("UDP socket error: %1").arg(m_socket->errorString()));
}
