// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSslSocket>
#include <QThread>
#include <QThreadPool>
#include <QEventLoop>
#include <QDebug>
#include <QMutex>

#include "tcpthread.h"
#include "netpacket.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
	~TcpServer();
public:
    void SetMaxThreads(int maximum);
	void SetMaxConnections(int value);
	void SetConnectionTimeout(int value);
	bool Listen(const QHostAddress &address, quint16 port);
	void Clear();
public:
	void sendMessageToClient(QSslSocket* socket, NetPacket &packet);
	void sendGlobalMessage(NetPacket &packet);

	void AddNewClient(SClient client);
	void RemoveClient(SClient client);
	void UpdateClient(SClient* client);
	bool UpdateProfile(SProfile* profile);

	QStringList GetPlayersList();
	int GetClientCount();
	QSslSocket* GetSocketByUid(int uid);
	SProfile* GetProfileByUid(int uid);
public slots:
	void started();
	void finished();
	void stop();
	void Update();
signals:
	void connecting(qintptr handle, TcpThread *runnable, TcpConnection* connection);
	void closing();
	void idle(int value);

private:
	virtual void incomingConnection(qintptr socketDescriptor);
	virtual TcpThread* CreateRunnable();
	virtual void StartRunnable(TcpThread *runnable);
	virtual void Reject(qintptr handle);
	virtual void Accept(qintptr handle, TcpThread *runnable);
	virtual void Start();

	int maxThreads = 0;
	int maxConnections = 0;
	int connectionTimeout = 0;

	QVector<SClient>  m_Clients;
	QList<TcpThread*> m_threads;
};
#endif // TCPSERVER_H
