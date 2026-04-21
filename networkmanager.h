#pragma once
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QByteArray>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();

    bool host(quint16 port);
    void join(const QString& addr, quint16 port);
    void sendJson(const QJsonObject& obj);
    void closeAll();

    bool isConnected() const;
    bool isHosting()   const;

signals:
    void peerConnected();
    void peerDisconnected();
    void messageReceived(QJsonObject);
    void networkError(QString);

private slots:
    void onNewConnection();
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError);

private:
    QTcpServer* server = nullptr;
    QTcpSocket* sock   = nullptr;
    QByteArray  recvBuf;
};
