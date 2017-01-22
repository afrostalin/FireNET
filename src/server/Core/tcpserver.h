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

enum EServerStatus
{
	EServer_Online,
	EServer_Offline,
};

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
	QSslSocket* GetSocketByUid(int uid);
	SProfile* GetProfileByUid(int uid);

	int GetClientCount();
	int GetMaxClientCount() { return m_maxConnections; }
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

	int m_maxThreads;
	int m_maxConnections;
	int m_connectionTimeout;

	QVector<SClient>  m_Clients;
	QList<TcpThread*> m_threads;
	QMutex            m_Mutex;
public:
	EServerStatus m_Status;
};
#endif // TCPSERVER_H
