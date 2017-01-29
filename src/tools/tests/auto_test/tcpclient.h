#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QSslSocket>

#include <queue>

#include "netpacket.h"

class tcpclient : public QObject
{
    Q_OBJECT
public:
    explicit tcpclient(QObject *parent = 0);
public:
    void CreateClient();
    void CreateTestList();
public slots:
    void onConnectedToServer();
    void onReadyRead();
    void onBytesWritten(qint64 bytes);
    void onDisconnected();

    void update();
private:
    void SendMsg(NetPacket &packet);
private:
    QSslSocket* m_socket;
    std::queue<NetPacket> m_packets;

    bool bConnected;
    bool bLastMsgSended;
};

#endif // TCPCLIENT_H
