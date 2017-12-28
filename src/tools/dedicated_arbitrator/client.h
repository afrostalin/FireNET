// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QSslSocket>
#include <QTimer>
#include <queue>

#include "Core/tcppacket.h"

class RemoteClient : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClient(QObject *parent = 0);
public:
    void                   ConnectToServer(const QString &ip, int port);
    void                   SendMessage(CTcpPacket &packet);
	bool                   IsConnected() const { return bConnected; }
public slots:
    void                   onConnectedToServer();
    void                   onReadyRead();
    void                   onBytesWritten(qint64 bytes);
    void                   onDisconnected();
	void                   Update();
public:
	void                   ReadResult(CTcpPacket &packet) const;
	void                   ReadError(CTcpPacket &packet) const;
private:
    QSslSocket*            m_socket;
	std::queue<CTcpPacket> m_packets;
	QTimer                 m_Timer;

	bool                   bLastMsgSended;
	bool                   bConnected;
};