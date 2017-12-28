// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QSslSocket>
#include <QTime>

#include <queue>

#include "global.h"
#include "tcppacket.h"

class ClientQuerys;

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = nullptr);
    ~TcpConnection();
public:
	void                  SendMessage(CTcpPacket &packet);
	SIpPort               GetPeerAddress() const;
private:
	QSslSocket*            CreateSocket();
	void                   CalculateStatistic();
public slots:
	void                   quit();
	void                   accept(qint64 socketDescriptor);
	void                   connected();
	void                   disconnected();
	void                   readyRead();
	void                   bytesWritten(qint64 bytes);
	void                   stateChanged(QAbstractSocket::SocketState socketState) const;
	void                   socketError(QAbstractSocket::SocketError error);
	void                   Update();
signals:
	void                   opened();
	void                   closed();

	void                   received();
	void                   sended();
private:	
	QSslSocket*            m_Socket;
	std::string            m_PeerIP;
	quint16                m_PeerPort;

	ClientQuerys*          pQuery;
	SClient                m_Client;
	std::queue<CTcpPacket> m_Packets;
private:
	int                    m_maxPacketSize;
	int                    m_maxBadPacketsCount;
	int                    m_BadPacketsCount;

	QTime                  m_Time;
	int                    m_InputPacketsCount;
	int                    m_PacketsSpeed;
	int                    m_maxPacketSpeed;

	bool                   bConnected;
	bool                   bIsQuiting;
	bool                   bLastMsgSended;
};