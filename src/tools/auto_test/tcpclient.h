// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QSslSocket>

#include <queue>

#include "Core/tcppacket.h"

class tcpclient : public QObject
{
    Q_OBJECT
public:
    explicit tcpclient(QString ip = "127.0.0.1", int port = 3322, QObject *parent = nullptr);
public:
    void                   CreateClient();
    void                   CreateTestList();
private:
    void                   SendMsg(CTcpPacket &packet);
public slots:
	void                   onConnectedToServer();
	void                   onReadyRead() const;
	void                   onBytesWritten(qint64 bytes);
	void                   onDisconnected();
	void                   update();
private:
    QSslSocket*            m_socket;
    std::queue<CTcpPacket> m_packets;

    bool                   bConnected;
    bool                   bLastMsgSended;
private:
	QString                m_ip;
	int                    m_port;
};