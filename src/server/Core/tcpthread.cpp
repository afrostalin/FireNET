// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QTimer>

#include "global.h"
#include "tcpthread.h"
#include "tcpserver.h"
#include "Tools/console.h"

TcpThread::TcpThread(QObject *parent) 
	: QObject(parent)
	, m_loop(nullptr)
{
	Q_UNUSED(parent);
}

TcpThread::~TcpThread()
{
	m_connections.clear();
	SAFE_DELETE(m_loop);
}

void TcpThread::run()
{
	//Make an event loop to keep this alive on the thread
	m_loop = new QEventLoop();
	m_loop->exec();

	emit finished();
}

int TcpThread::Count()
{
	QReadLocker locker(&m_lock);
	return m_connections.count();
}

void TcpThread::SendGlobalMessage(CTcpPacket & packet)
{
	for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
	{
		(*it)->SendMessage(packet);
	}
}

void TcpThread::connecting(const qintptr handle, TcpThread *runnable, TcpConnection* connection)
{
	if (runnable != this) 
		return;

	connection->moveToThread(QThread::currentThread());

	m_connections.append(connection);
	AddSignals(connection);
	connection->accept(handle);
}

void TcpThread::closing()
{
	emit quit();
	m_loop->exit();
}

void TcpThread::opened() const
{
	TcpConnection *connection = static_cast<TcpConnection*>(sender());
	if (!connection)
		return;

	// Block very fast connection
	if (!gEnv->pConsole->GetBool("bUseStressMode") && gEnv->pServer->GetClientCount() > gEnv->pConsole->GetInt("sv_max_players"))
	{
		connection->quit();
	}
}

void TcpThread::closed()
{
	TcpConnection* connection = static_cast<TcpConnection*>(sender());
	if (connection == nullptr)
		return;

	m_connections.removeAll(connection);
	connection->deleteLater();
}

TcpConnection* TcpThread::CreateConnection()
{
	TcpConnection* connection = new TcpConnection();
	return connection;
}

void TcpThread::AddSignals(TcpConnection * connection) const
{
	connect(connection, &TcpConnection::opened, this, &TcpThread::opened, Qt::QueuedConnection);
	connect(connection, &TcpConnection::closed, this, &TcpThread::closed, Qt::QueuedConnection);
	connect(this, &TcpThread::quit, connection, &TcpConnection::quit, Qt::QueuedConnection);

	connect(connection, &TcpConnection::received, gEnv->pServer, &TcpServer::MessageReceived, Qt::QueuedConnection);
	connect(connection, &TcpConnection::sended, gEnv->pServer, &TcpServer::MessageSended, Qt::QueuedConnection);

	connect(gEnv->pTimer, &QTimer::timeout, connection, &TcpConnection::Update, Qt::QueuedConnection);
}
