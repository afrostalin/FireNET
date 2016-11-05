// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "tcpserver.h"
#include "settings.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    m_pool = QThreadPool::globalInstance();
    setMaxThreads(m_pool->maxThreadCount());
}

TcpServer::~TcpServer()
{
    close();
}

void TcpServer::setMaxThreads(int maximum)
{
    if(isListening())
    {
        qWarning() << "Max threads not changed, call setMaxThreads() before listen()";
        return;
    }

    m_pool->setMaxThreadCount(maximum);
}

bool TcpServer::listen(const QHostAddress &address, quint16 port)
{
	qInfo() << "Start listening on port " << port;
    startThreads();
    return QTcpServer::listen(address,port);
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	qDebug() << "Incomming connection...";

	TcpThread *pThread = freeThread();

	if (!pThread)
	{
		qCritical() << "No free thread!";
		return;
	}

	if (m_Clients.size() != gEnv->pSettings->GetVariable("sv_max_players").toInt())
		pThread->accept(socketDescriptor, pThread->runnableThread());
	else
	{
		qWarning() << "Connection limit exceeded! Max players = " << gEnv->pSettings->GetVariable("sv_max_players").toInt();
	}
}

void TcpServer::AddNewClient(SClient client)
{
	QMutexLocker locker(&m_Mutex);

	if (client.socket == nullptr)
	{
		qWarning() << "Can't add client. Client socket = nullptr";
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client.socket)
		{
			qWarning() << "Can't add client" << client.socket << ". Client alredy added";
			return;
		}
	}

	qDebug() << "Adding new client" << client.socket;
	m_Clients.push_back(client);
}

void TcpServer::RemoveClient(SClient client)
{
	QMutexLocker locker(&m_Mutex);

	if (client.socket == nullptr)
	{
		qWarning() << "Can't remove client. Client socket = nullptr";
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client.socket)
		{
			qDebug() << "Removing client" << client.socket;
			m_Clients.erase(it);
			return;
		}
	}

	qWarning() << "Can't remove client. Client" << client.socket << "not found";
}

void TcpServer::UpdateClient(SClient* client)
{
	QMutexLocker locker(&m_Mutex);

	if (client->socket == nullptr)
	{
		qWarning() << "Can't update client. Client socket = nullptr";
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client->socket)
		{
			if (client->profile != nullptr)
			{
				it->profile = client->profile;
				it->status = client->status;

				qDebug() << "Client" << it->socket << "updated.";
				return;
			}
			else
			{
				qWarning() << "Can't update client" << it->socket << ". Profile = nullptr";
				return;
			}
		}
	}

	qWarning() << "Can't update client. Client" << client->socket << "not found";
}

int TcpServer::GetClientCount()
{
	QMutexLocker locker(&m_Mutex);
	return m_Clients.size();
}

QSslSocket * TcpServer::GetSocketByUid(int uid)
{
	QMutexLocker locker(&m_Mutex);
	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (it->profile->uid == uid)
			{
				qDebug() << "Socket finded. Return";
				return it->socket;
			}
		}
	}

	qDebug() << "Socket not finded.";
	return nullptr;
}

void TcpServer::sendMessageToClient(QSslSocket * socket, QByteArray data)
{
	qDebug() << "Send message to client. Original size = " << data.size();
	socket->write(data);
	socket->waitForBytesWritten(3);
}

void TcpServer::sendGlobalMessage(QByteArray data)
{
	qDebug() << "Send message to all clients. Original size = " << data.size();

	QVector<SClient>::iterator it;
	for (it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		it->socket->write(data);
		it->socket->waitForBytesWritten(3);
	}
}

void TcpServer::startThreads()
{
	qInfo() << "Starting threads :" << m_pool->maxThreadCount();

    for(int i = 0; i < m_pool->maxThreadCount(); i++)
    {
        TcpThread *pThread = new TcpThread(this);
        startThread(pThread);
    }
}

void TcpServer::startThread(TcpThread *pThread)
{
	connect(gEnv->pTimer, SIGNAL(timeout()), pThread, SLOT(Update()));
	connect(this,&TcpServer::stop, pThread, &TcpThread::stop);

    m_threads.append(pThread);
    pThread->setAutoDelete(true);
    m_pool->start(pThread);
}

TcpThread *TcpServer::freeThread()
{
    if(m_threads.count() < 1)
    {
        qCritical() << "No threads to run connection on!";
        return nullptr;
    }

	TcpThread* pThread;

	int threadsCount = 0;
	int maxClientsInThread = 0;

    // Detect maximum clients in thread
	for (threadsCount; threadsCount != m_pool->maxThreadCount(); threadsCount++)
	{
		pThread = m_threads.at(threadsCount);
		int clientsInThread = pThread->GetClientsCount();

		if (clientsInThread > maxClientsInThread)
			maxClientsInThread = clientsInThread;
	}

	threadsCount = 0;

    // Return thread with minimal clients count
	for (threadsCount; threadsCount != m_pool->maxThreadCount(); threadsCount++)
	{
		pThread = m_threads.at(threadsCount);
		int clientsInThread = pThread->GetClientsCount();
		if (clientsInThread < maxClientsInThread)
		{
			return pThread;
		}
	}


    return pThread;
}

void TcpServer::close()
{
	qInfo() << "Closing server...";
	emit stop();

	m_threads.clear();
	m_pool->clear();
}