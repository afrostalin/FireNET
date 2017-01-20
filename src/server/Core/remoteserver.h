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

class RemoteConnection;
class NetPacket;

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
	int GetClientCount();
	QStringList GetServerList();
	SGameServer* GetGameServer(QString name, QString map, QString gamerules);

	void AddNewClient(SRemoteClient client);
	void RemoveClient(SRemoteClient client);
	void UpdateClient(SRemoteClient* client);

	bool CheckGameServerExists(QString name, QString ip, int port);
private:
	bool CreateServer();
public:
    virtual void incomingConnection(qintptr socketDescriptor);
	void sendMessageToRemoteClient(QSslSocket* socket, NetPacket &packet);
public:
	bool bHaveAdmin;
private:
	QThread* m_Thread;
	QTcpServer* m_Server;
	QList<RemoteConnection*> m_connections;
private:
	QVector<SRemoteClient> m_Clients;
	QMutex m_Mutex;
};

#endif // REMOTESERVER_H