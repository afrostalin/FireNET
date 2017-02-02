#ifndef REMOTEADMIN_H
#define REMOTEADMIN_H

#include <QObject>
#include <QDebug>
#include <QSslSocket>

#include "Core/netpacket.h"

class RemoteClient : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClient(QObject *parent = 0);
public:
    void ConnectToServer(QString ip, int port);
    void SendMessage(NetPacket &packet);
public slots:
    void onConnectedToServer();
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onDisconnected();
public:
    bool bConnected;
private:
    QSslSocket* m_socket;
};

#endif // REMOTEADMIN_H
