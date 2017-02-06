// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <QObject>
#include <QThread>
#include <QTcpServer>
#include <QSslSocket>
#include <QMutex>

#include "global.h"
#include "remoteconnection.h"
#include "netpacket.h"

class RemoteServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit RemoteServer(QObject *parent = nullptr);
	~RemoteServer();
public:
	void                     Clear();
	void                     run();
	void                     sendMessageToRemoteClient(QSslSocket* socket, NetPacket &packet);
	void                     AddNewClient(SRemoteClient &client);
	void                     RemoveClient(SRemoteClient &client);
	void                     UpdateClient(SRemoteClient* client);
	bool                     CheckGameServerExists(const QString &name, const QString &ip, int port);
	void                     SetMaxClientCount(int count) { m_MaxClinetCount = count; }
	int                      GetClientCount();
	int                      GetMaxClientCount() { return m_MaxClinetCount; }
	bool                     IsHaveAdmin() { return bHaveAdmin; }
	void                     SetAdmin(bool bAmin) { bHaveAdmin = bAmin; }

	QStringList              GetServerList();
	SGameServer*             GetGameServer(const QString &name, const QString &map, const QString &gamerules);
	
private:
	bool                     CreateServer();
    virtual void             incomingConnection(qintptr socketDescriptor);
public slots:
	void                     Update();
	void                     CloseConnection();
signals:
	void                     close();
private:
	QTcpServer*              m_Server;
	QVector<SRemoteClient>   m_Clients;
	QList<RemoteConnection*> m_connections;
	QMutex                   m_Mutex;

	int                      m_MaxClinetCount;
	bool                     bHaveAdmin;
};

#endif // REMOTESERVER_H