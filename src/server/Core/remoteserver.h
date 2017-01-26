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
    explicit RemoteServer(QObject *parent = 0);
	~RemoteServer();
public:
	void Clear();
public slots:
	void Update();
	void CloseConnection();
signals:
	void close();
public:
	void run();

	void sendMessageToRemoteClient(QSslSocket* socket, NetPacket &packet);
	
	QStringList GetServerList();
	SGameServer* GetGameServer(QString name, QString map, QString gamerules);

	void AddNewClient(SRemoteClient client);
	void RemoveClient(SRemoteClient client);
	void UpdateClient(SRemoteClient* client);

	bool CheckGameServerExists(QString name, QString ip, int port);

	int GetClientCount();
	int GetMaxClientCount() { return m_MaxClinetCount; }

	bool IsHaveAdmin() { return bHaveAdmin; }
	void SetAdmin(bool bAmin) { bHaveAdmin = bAmin; }
private:
	bool CreateServer();
	void SetMaxClientCount(int max) { m_MaxClinetCount = max; }
    virtual void incomingConnection(qintptr socketDescriptor);
private:
	QTcpServer* m_Server;
	QVector<SRemoteClient> m_Clients;
	QList<RemoteConnection*> m_connections;

	int m_MaxClinetCount;
	bool bHaveAdmin;
};

#endif // REMOTESERVER_H