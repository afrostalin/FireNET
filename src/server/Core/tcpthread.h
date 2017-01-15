// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QThread>
#include <QRunnable>
#include <QEventLoop>
#include <QDebug>

#include "tcpthread.h"
#include "tcpconnection.h"

#include "Workers/Databases/redisconnector.h"

class TcpThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = 0);
    ~TcpThread();

    virtual void run();
    virtual void accept(qint64 socketDescriptor,QThread *owner);
	int GetClientsCount();
    virtual QThread *runnableThread();

protected:
    QEventLoop *m_loop;
    QThread *m_thread;
    QList<TcpConnection*> m_connections;
signals:
    void close();

public slots:
    virtual void Update();
    virtual void stop();
    virtual void finished();
};

#endif // TCPTHREAD_H
