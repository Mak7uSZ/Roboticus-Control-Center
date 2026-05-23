#include "UDPConnection.h"

#include <QHostAddress>
#include <QNetworkDatagram>
#include <QUdpSocket>

UDPConnection::UDPConnection(QObject *parent) : QObject(parent) {
  m_noDatagramsTimer.setSingleShot(true);
  m_noDatagramsTimer.setInterval(NoDatagramsTimeoutMs);

  connect(&m_noDatagramsTimer, &QTimer::timeout, this,
          &UDPConnection::handleNoDatagramsTimeout);
}

bool UDPConnection::startListening(quint16 port) {
  if (port == 0) {
    setError("UDP port must be between 1 and 65535.");
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
    setError(QString("Failed to listen on UDP port %1: %2")
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
  clearError();
  m_noDatagramsTimer.start();

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
  m_noDatagramsTimer.stop();

  if (m_socket) {
    disconnect(m_socket, &QUdpSocket::readyRead, this,
               &UDPConnection::readPendingDatagrams);
    m_socket->close();
  }

  m_listening = false;
  clearError();

  if (wasListening) {
    emit listeningChanged();
  }
}

void UDPConnection::clearStatistics() {
  m_packetsReceived = 0;
  m_bytesReceived = 0;
  m_lastSenderAddress.clear();
  m_lastSenderPort = 0;

  emit statisticsChanged();
  emit lastSenderChanged();
}

void UDPConnection::readPendingDatagrams() {
  if (!m_socket) {
    return;
  }

  bool receivedAnyDatagram = false;

  while (m_socket->hasPendingDatagrams()) {
    const QNetworkDatagram datagram = m_socket->receiveDatagram();
    if (!datagram.isValid()) {
      setError("Received an invalid UDP datagram.");
      continue;
    }

    const QByteArray data = datagram.data();

    ++m_packetsReceived;
    m_bytesReceived += static_cast<quint64>(data.size());
    m_lastSenderAddress = datagram.senderAddress().toString();
    m_lastSenderPort = datagram.senderPort();

    receivedAnyDatagram = true;
    emit rawDataReceived(data);
  }

  if (receivedAnyDatagram) {
    clearError();
    m_noDatagramsTimer.start();
    emit statisticsChanged();
    emit lastSenderChanged();
  }
}

void UDPConnection::handleNoDatagramsTimeout() {
  if (!m_listening) {
    return;
  }

  setError(QString("No UDP packets were received on port %1 within 10 seconds.")
               .arg(m_port));
}

void UDPConnection::handleSocketError(QAbstractSocket::SocketError error) {
  if (error == QAbstractSocket::UnknownSocketError || !m_socket) {
    return;
  }

  setError(QString("UDP socket error: %1").arg(m_socket->errorString()));
}

void UDPConnection::setError(const QString &message) {
  if (message.isEmpty()) {
    return;
  }

  if (m_errorString != message) {
    m_errorString = message;
    emit errorChanged();
  }

  emit errorOccurred(message);
}

void UDPConnection::clearError() {
  if (m_errorString.isEmpty()) {
    return;
  }

  m_errorString.clear();
  emit errorChanged();
}
