#include "networkmanager.h"

#include <QJsonDocument>
#include <QHostAddress>
#include <QDebug>

NetworkManager::NetworkManager(QObject* parent) : QObject(parent) {}

NetworkManager::~NetworkManager() { closeAll(); }

// ─────────────────────────────────────────────────────────────────────────────
bool NetworkManager::host(quint16 port)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &NetworkManager::onNewConnection);

    if (!server->listen(QHostAddress::Any, port)) {
        emit networkError("서버 시작 실패: " + server->errorString());
        server->deleteLater();
        server = nullptr;
        return false;
    }
    return true;
}

void NetworkManager::join(const QString& addr, quint16 port)
{
    sock = new QTcpSocket(this);
    connect(sock, &QTcpSocket::connected,    this, &NetworkManager::onConnected);
    connect(sock, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(sock, &QTcpSocket::readyRead,    this, &NetworkManager::onReadyRead);
    connect(sock, &QAbstractSocket::errorOccurred,
            this, &NetworkManager::onSocketError);
    sock->connectToHost(addr, port);
}

// ─────────────────────────────────────────────────────────────────────────────
void NetworkManager::onNewConnection()
{
    if (sock) return; // one client at a time
    sock = server->nextPendingConnection();
    connect(sock, &QTcpSocket::disconnected, this, &NetworkManager::onDisconnected);
    connect(sock, &QTcpSocket::readyRead,    this, &NetworkManager::onReadyRead);
    connect(sock, &QAbstractSocket::errorOccurred,
            this, &NetworkManager::onSocketError);
    emit peerConnected();
}

void NetworkManager::onConnected()    { emit peerConnected(); }
void NetworkManager::onDisconnected() { sock = nullptr; emit peerDisconnected(); }

void NetworkManager::onSocketError(QAbstractSocket::SocketError)
{
    if (sock) emit networkError(sock->errorString());
}

// ─────────────────────────────────────────────────────────────────────────────
// Newline-delimited JSON framing
void NetworkManager::onReadyRead()
{
    recvBuf += sock->readAll();
    while (recvBuf.contains('\n')) {
        int idx      = recvBuf.indexOf('\n');
        QByteArray ln = recvBuf.left(idx).trimmed();
        recvBuf      = recvBuf.mid(idx + 1);
        if (ln.isEmpty()) continue;
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(ln, &err);
        if (err.error == QJsonParseError::NoError && doc.isObject())
            emit messageReceived(doc.object());
    }
}

void NetworkManager::sendJson(const QJsonObject& obj)
{
    if (!sock || sock->state() != QAbstractSocket::ConnectedState) return;
    sock->write(QJsonDocument(obj).toJson(QJsonDocument::Compact) + "\n");
}

void NetworkManager::closeAll()
{
    if (sock)   { sock->disconnectFromHost();  sock->deleteLater();   sock   = nullptr; }
    if (server) { server->close();             server->deleteLater(); server = nullptr; }
}

bool NetworkManager::isConnected() const
{
    return sock && sock->state() == QAbstractSocket::ConnectedState;
}

bool NetworkManager::isHosting() const
{
    return server && server->isListening();
}
