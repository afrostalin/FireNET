// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QDebug>
#include <QSslSocket>
#include <QTimer>
#include <queue>

#include "Core/netpacket.h"

class RemoteClient : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClient(QObject *parent = 0);
public:
    void                  ConnectToServer(const QString &ip, int port);
    void                  SendMessage(NetPacket &packet);
	bool                  IsConnected() { return bConnected; }
public slots:
    void                  onConnectedToServer();
    void                  onReadyRead();
    void                  onBytesWritten(qint64 bytes);
    void                  onDisconnected();
	void                  Update();
private:
    QSslSocket*           m_socket;
	std::queue<NetPacket> m_packets;
	QTimer                m_Timer;

	bool                  bLastMsgSended;
	bool                  bConnected;
};