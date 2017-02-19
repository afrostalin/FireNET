// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef REMOTECLIENTQUERYS_H
#define REMOTECLIENTQUERYS_H

#include <QObject>

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
	void              SetConnection(RemoteConnection* connection) { this->m_connection = connection; }
public:
	// Administration functional
	void              onAdminLogining(CTcpPacket &packet);
	void              onConsoleCommandRecived(CTcpPacket &packet);

	// Game server functionality
	void              onGameServerRegister(CTcpPacket &packet);
	void              onGameServerUpdateInfo(CTcpPacket &packet);
	void              onGameServerGetOnlineProfile(CTcpPacket &packet);
	void              onGameServerUpdateOnlineProfile(CTcpPacket &packet);
private:
	bool              CheckInTrustedList(const QString &name, const QString &ip, int port);
private:
	QSslSocket*       m_socket;
	SRemoteClient*	  m_client;
	RemoteConnection* m_connection;
};

#endif // REMOTECLIENTQUERYS_H