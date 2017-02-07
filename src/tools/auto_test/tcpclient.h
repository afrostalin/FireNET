// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QSslSocket>

#include <queue>

#include "Core/netpacket.h"

class tcpclient : public QObject
{
    Q_OBJECT
public:
    explicit tcpclient(QString ip = "127.0.0.1", int port = 3322, QObject *parent = nullptr);
public:
    void                  CreateClient();
    void                  CreateTestList();
private:
    void                  SendMsg(NetPacket &packet);
public slots:
	void                  onConnectedToServer();
	void                  onReadyRead();
	void                  onBytesWritten(qint64 bytes);
	void                  onDisconnected();
	void                  update();
private:
    QSslSocket*           m_socket;
    std::queue<NetPacket> m_packets;

    bool                  bConnected;
    bool                  bLastMsgSended;
private:
	QString               m_ip;
	int                   m_port;
};