#pragma once

#include <QAbstractSocket>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <QTimer>

class QUdpSocket;

class UDPConnection : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(quint16 port READ port NOTIFY portChanged)
    Q_PROPERTY(quint64 packetsReceived READ packetsReceived NOTIFY statisticsChanged)
    Q_PROPERTY(quint64 bytesReceived READ bytesReceived NOTIFY statisticsChanged)
    Q_PROPERTY(QString lastSenderAddress READ lastSenderAddress NOTIFY lastSenderChanged)
    Q_PROPERTY(quint16 lastSenderPort READ lastSenderPort NOTIFY lastSenderChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

public:
    explicit UDPConnection(QObject *parent = nullptr);

    bool isListening() const { return m_listening; }
    quint16 port() const { return m_port; }
    quint64 packetsReceived() const { return m_packetsReceived; }
    quint64 bytesReceived() const { return m_bytesReceived; }
    QString lastSenderAddress() const { return m_lastSenderAddress; }
    quint16 lastSenderPort() const { return m_lastSenderPort; }
    QString errorString() const { return m_errorString; }

    Q_INVOKABLE bool startListening(quint16 port);
    Q_INVOKABLE void stopListening();
    Q_INVOKABLE void clearStatistics();

signals:
    void rawDataReceived(const QByteArray &data);
    void listeningChanged();
    void portChanged();
    void statisticsChanged();
    void lastSenderChanged();
    void errorChanged();
    void errorOccurred(const QString &message);

private slots:
    void readPendingDatagrams();
    void handleNoDatagramsTimeout();
    void handleSocketError(QAbstractSocket::SocketError error);

private:
    void setError(const QString &message);
    void clearError();

    static constexpr int NoDatagramsTimeoutMs = 10000;

    QUdpSocket *m_socket = nullptr;
    QTimer m_noDatagramsTimer;
    bool m_listening = false;
    quint16 m_port = 0;
    quint64 m_packetsReceived = 0;
    quint64 m_bytesReceived = 0;
    QString m_lastSenderAddress;
    quint16 m_lastSenderPort = 0;
    QString m_errorString;
};
