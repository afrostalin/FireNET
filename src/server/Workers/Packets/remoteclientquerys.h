// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QTimer>

#include "global.h"

class CTcpPacket;
class RemoteConnection;

class RemoteClientQuerys : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClientQuerys(QObject *parent = 0);
	~RemoteClientQuerys();
public:
	void              SetSocket(QSslSocket* socket) { this->m_socket = socket; }
	void              SetClient(SRemoteClient* client);
	void              SetConnection(RemoteConnection* connection);
public:
	bool              ReadPacket(CTcpPacket &packet);
	bool              ReadQuery(CTcpPacket &packet);

	// Administration functional
	void              onAdminLogining(CTcpPacket &packet) const;
	void              onConsoleCommandRecived(CTcpPacket &packet);

	// Game server functionality
	void              onGameServerRegister(CTcpPacket &packet);
	void              onGameServerUpdateInfo(CTcpPacket &packet);
	void              onGameServerGetOnlineProfile(CTcpPacket &packet);
	void              onGameServerUpdateOnlineProfile(CTcpPacket &packet);
	void              onPingPong(CTcpPacket &packet);

	// Dedicated arbitrator
	void              onArbitratorRegister(CTcpPacket &packet);
	void              onArbitratorUpdate(CTcpPacket &packet);
private:
	bool              CheckInTrustedList(const QString &name, const QString &ip, int port);
public:
	const char*       GetStatusString(EFireNetGameServerStatus status) const;
public slots:
	void              OnUpdate();
	void              OnConnectionClosed();
private:	
	QSslSocket*       m_socket;
	SRemoteClient*	  m_client;
	RemoteConnection* m_connection;
	bool              bReloadRequestSended;
private:
	QTimer            m_Timer;
	bool              m_ServerAlive;
};