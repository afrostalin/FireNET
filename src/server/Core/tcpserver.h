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
public slots:
	void Update();
public:
	void Clear();
public:
	void sendMessageToClient(QSslSocket* socket, NetPacket &packet);
	void sendGlobalMessage(NetPacket &packet);
public:
    virtual void setMaxThreads(int maximum);
    virtual bool listen(const QHostAddress &address, quint16 port);
	virtual void close();
public:
	void AddNewClient(SClient client);
	void RemoveClient(SClient client);
	void UpdateClient(SClient* client);
	bool UpdateProfile(SProfile* profile);

	QStringList GetPlayersList();
	int GetClientCount();
	QSslSocket* GetSocketByUid(int uid);
	SProfile* GetProfileByUid(int uid);
private:
	QVector<SClient> m_Clients;
	QMutex m_Mutex;
protected:
    QList<TcpThread*> m_threads;
    QThreadPool *m_pool;
    virtual void incomingConnection(qintptr socketDescriptor);
    virtual void startThreads();
    virtual void startThread(TcpThread *cThread);
	virtual TcpThread *freeThread();
signals:
    void stop();
};
#endif // TCPSERVER_H
