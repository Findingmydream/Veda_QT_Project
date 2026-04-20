#pragma once

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QByteArray>

// ─────────────────────────────────────────────────────────────────────────────
// NetworkManager – thin wrapper around QTcpServer / QTcpSocket
//
// Protocol: newline-delimited compact JSON  (each message ends with '\n')
//
// Lobby messages (MainWindow handles):
//   {"t":"hello","name":"..."}   – name exchange after connect
//   {"t":"start","difficulty":N} – host signals game begin
//
// In-game messages (GameWidget handles):
//   {"t":"pos",  "x":N}          – my ship x position
//   {"t":"bullet", "x":N,"y":N}  – my bullet spawn position
//   {"t":"spawn","id":N,"x":N,"lv":N} – host spawns enemy
//   {"t":"enemyFire","id":N,"x":N,"y":N} – host spawns enemy bullet
//   {"t":"kill", "id":N,"score":N}    – I killed enemy id N
//   {"t":"level","l":N}          – host broadcasts new level
//   {"t":"hit"}                  – I lost a life (not dead yet)
//   {"t":"dead","score":N}       – I died (game over for me)
// ─────────────────────────────────────────────────────────────────────────────

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    explicit NetworkManager(QObject* parent = nullptr);
    ~NetworkManager();

    // Host mode: open a TCP server on `port`
    bool host(quint16 port);

    // Client mode: connect to `addr:port`
    void join(const QString& addr, quint16 port);

    // Send a JSON message to the peer
    void sendJson(const QJsonObject& obj);

    // Tear down everything
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
