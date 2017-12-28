// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QRunnable>
#include <QEventLoop>
#include <QReadLocker>

#include "tcpthread.h"
#include "tcpconnection.h"

class TcpThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit TcpThread(QObject *parent = nullptr);
    ~TcpThread();
public:
	void                  run();
	int                   Count();
	void                  SendGlobalMessage(CTcpPacket &packet);
private:
	static TcpConnection* CreateConnection();
	void                  AddSignals(TcpConnection* connection) const;
public slots:
	void                  connecting(qintptr handle, TcpThread *runnable, TcpConnection* connection);
	void                  closing();
	void                  opened() const;
	void                  closed();
signals:
	void                  started();
	void                  finished();
	void                  quit();
private:
	QEventLoop*           m_loop;
	QReadWriteLock        m_lock;
	QList<TcpConnection*> m_connections;
};
