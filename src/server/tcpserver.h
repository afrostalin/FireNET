// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

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
	void sendMessageToClient(QSslSocket* socket, QByteArray &data);
	void sendGlobalMessage(QByteArray &data);
public:
    virtual void setMaxThreads(int maximum);
    virtual bool listen(const QHostAddress &address, quint16 port);
	virtual void close();
public:
	void AddNewClient(SClient client);
	void RemoveClient(SClient client);
	void UpdateClient(SClient* client);
	QStringList GetPlayersList();
	int GetClientCount();
	QSslSocket* GetSocketByUid(int uid);
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
