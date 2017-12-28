// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QMutex>

#include "global.h"
#include "remoteconnection.h"

class CTcpPacket;

class RemoteServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit RemoteServer(QObject *parent = nullptr);
	~RemoteServer();
public:
	void                       Clear();
	void                       run();
	void                       sendMessageToRemoteClient(QSslSocket* socket, CTcpPacket &packet) const;
	void                       SendMessageToAllRemoteClients(CTcpPacket &packet);
	void                       AddNewClient(SRemoteClient &client);
	void                       RemoveClient(SRemoteClient &client);
	void                       UpdateClient(SRemoteClient* client);
	bool                       CheckGameServerExists(const QString &name, const QString &ip, int port);
	void                       SetMaxClientCount(const int count) { m_MaxClinetCount = count; }
	int                        GetClientCount();
	int                        GetMaxClientCount() const { return m_MaxClinetCount; }
	bool                       IsHaveAdmin() const { return bHaveAdmin; }
	void                       SetAdmin(const bool bAmin) { bHaveAdmin = bAmin; }

	std::vector<std::string>   DumpServerList();
	SGameServer*               GetGameServer(const QString &name, const QString &map, const QString &gamerules, bool onlyEmpty = false);
	void                       GetAllGameServers(std::vector<SGameServer> &list);
private:
	bool                       CreateServer();
	void                       incomingConnection(qintptr socketDescriptor) override;
public slots:
	void                       Update() const;
	void                       CloseConnection();
signals:
	void                       close();
private:
	QTcpServer*                m_Server;
	std::vector<SRemoteClient> m_Clients;
	QList<RemoteConnection*>   m_connections;
	QMutex                     m_Mutex;

	int                        m_MaxClinetCount;
	bool                       bHaveAdmin;
};