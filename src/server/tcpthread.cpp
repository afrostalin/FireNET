// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "tcpthread.h"
#include "global.h"

TcpThread::TcpThread(QObject *parent) : QObject(parent)
{
	m_thread = 0;
	m_loop = 0;
	m_clients = 0;
}

TcpThread::~TcpThread()
{
}

void TcpThread::run()
{
	m_thread = QThread::currentThread();
	qDebug() << "Starting" << m_thread;

	m_loop = new QEventLoop();
	m_loop->exec();
}

void TcpThread::accept(qint64 socketDescriptor, QThread *owner)
{
	qDebug() << "Accepting new connection in " << m_thread;

	TcpConnection *connection = new TcpConnection();

	connect(this, &TcpThread::close, connection, &TcpConnection::close);
	connect(connection, &TcpConnection::finished, this, &TcpThread::finished);

	m_connections.append(connection);
	m_clients++;
	connection->accept(socketDescriptor);
}

int TcpThread::count()
{
	return m_connections.count();
}

QThread *TcpThread::runnableThread()
{
	return m_thread;
}

void TcpThread::stop()
{
	emit close();
	m_loop->quit();
}

void TcpThread::finished()
{
	if (!QObject::sender())
	{
		qCritical() << "Sender is not a QObject*";
		return;
	}

	TcpConnection *connection = qobject_cast<TcpConnection*>(QObject::sender());
	if (!connection)
	{
		qCritical() << "Sender is not a TcpConnection*";
		return;
	}

	m_connections.removeOne(connection);
	m_clients--;
	connection->deleteLater();
}

