#pragma once

#include <QAbstractSocket>
#include <QByteArray>
#include <QObject>
#include <QString>

class QUdpSocket;

class UDPConnection : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(quint16 port READ port NOTIFY portChanged)

public:
    explicit UDPConnection(QObject *parent = nullptr);

    static constexpr int MaxAcceptedDatagramSize = 1400;

    bool isListening() const { return m_listening; }
    quint16 port() const { return m_port; }
    quint64 packetsReceived() const { return m_packetsReceived; }
    quint64 bytesReceived() const { return m_bytesReceived; }
    QString lastSenderAddress() const { return m_lastSenderAddress; }
    quint16 lastSenderPort() const { return m_lastSenderPort; }

    Q_INVOKABLE bool startListening(quint16 port);
    Q_INVOKABLE void stopListening();
    Q_INVOKABLE void clearStatistics();

    static bool extractPayloadFromDatagram(const QByteArray &datagram,
                                           QByteArray *payload,
                                           QString *errorMessage = nullptr);

signals:
    /**
     * @brief Emitted with the MsgPack payload from one validated UDP frame
     *        datagram. UDP input is datagram/frame based and is never buffered
     *        together with other datagrams.
     */
    void rawDataReceived(const QByteArray &data);
    void listeningChanged();
    void portChanged();
    void errorOccurred(const QString &message);

private slots:
    void readPendingDatagrams();
    void handleNoDatagramsTimeout();
    void handleSocketError(QAbstractSocket::SocketError error);

private:
    static constexpr int NoDatagramsTimeoutMs = 10000;
    static constexpr quint8 FrameStartByte = 0xFD;

    QUdpSocket *m_socket = nullptr;
    bool m_listening = false;
    quint16 m_port = 0;
    quint64 m_packetsReceived = 0;
    quint64 m_bytesReceived = 0;
    QString m_lastSenderAddress;
    quint16 m_lastSenderPort = 0;
    QString m_errorString;
};
