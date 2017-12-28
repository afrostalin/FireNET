// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QSslSocket>
#include <queue>

#include "global.h"
#include "tcppacket.h"

class RemoteClientQuerys;

class RemoteConnection : public QObject
{
    Q_OBJECT
public:
    explicit RemoteConnection(QObject *parent = nullptr);
	~RemoteConnection();
public:
	void                  SendMessage(CTcpPacket &packet);
	SIpPort               GetPeerAddress() const;
private:
	void                  CalculateStatistic();
public slots:
	void                  accept(qint64 socketDescriptor);
	void                  close() const;
	void                  connected();
	void                  disconnected();
	void                  readyRead();
	void                  bytesWritten(qint64 bytes);
	void                  socketError(QAbstractSocket::SocketError error) const;
	void                  Update();
signals:
	void                  finished();
	void                  received();
	void                  sended();
private:
	QSslSocket*            m_socket;
	std::string            m_PeerIP;
	quint16                m_PeerPort;

	RemoteClientQuerys*    pQuerys;
	SRemoteClient          m_Client;
	std::queue<CTcpPacket> m_Packets;
private:
	int                   m_maxPacketSize;
	int                   m_maxBadPacketsCount;
	int                   m_BadPacketsCount;

	QTime                 m_Time;
	int                   m_InputPacketsCount;
	int                   m_PacketsSpeed;
	int                   m_maxPacketSpeed;

	bool                  bConnected;
	bool                  bLastMsgSended;
};