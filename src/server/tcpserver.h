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
#include "tcpthread.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    ~TcpServer();

public:
	void sendMessageToClient(QSslSocket* socket, QByteArray data);
	void sendGlobalMessage(QByteArray data);
    virtual void setMaxThreads(int maximum);
    virtual bool listen(const QHostAddress &address, quint16 port);
	virtual void close();

protected:
    QList<TcpThread*> m_threads;
    QThreadPool *m_pool;
    virtual void incomingConnection(qintptr socketDescriptor);
    virtual void startThreads();
    virtual void startThread(TcpThread *cThread);
	virtual TcpThread *freeThread();
signals:
    void stop();
public:
	QString serverIP;
	int serverPort;
	QString serverRootUser;
	QString serverRootPassword;
	int logLevel;
	int maxPlayers;
	int maxServers;
	int maxThreads;
	int tickRate;
	bool bGlobalChatEnable;
};
#endif // TCPSERVER_H
