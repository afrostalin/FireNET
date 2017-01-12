// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "tcpthread.h"
#include "global.h"

TcpThread::TcpThread(QObject *parent) : QObject(parent)
{
	m_thread = 0;
	m_loop = 0;
}

TcpThread::~TcpThread()
{
	qInfo() << "~TcpThread";
}

void TcpThread::Update()
{
	//qDebug() << "Update from" << m_thread;
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
	qDebug() << "Accepting new connection in" << owner;

	TcpConnection *connection = new TcpConnection();

	connect(this, &TcpThread::close, connection, &TcpConnection::close);
	connect(this, &TcpThread::close, m_loop,  &QEventLoop::quit);
	connect(connection, &TcpConnection::finished, this, &TcpThread::finished);

	m_connections.append(connection);
	connection->accept(socketDescriptor);
}

int TcpThread::GetClientsCount()
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
	connection->deleteLater();
}