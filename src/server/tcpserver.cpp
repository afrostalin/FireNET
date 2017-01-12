// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "tcpserver.h"
#include "settings.h"
#include "tcpthread.h"
#include "dbworker.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    m_pool = QThreadPool::globalInstance();
    setMaxThreads(m_pool->maxThreadCount());
}


void TcpServer::Clear()
{
	qInfo() << "~TcpServer";
	close();
}

void TcpServer::Update()
{
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

	if (pThread == nullptr)
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
//	QMutexLocker locker(&m_Mutex);

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
//	QMutexLocker locker(&m_Mutex);

	if (!client.socket)
	{
		qWarning() << "Can't remove client. Client socket = nullptr";
		return;
	}

	if (m_Clients.size() > 0)
	{
		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->socket == client.socket)
			{
				qDebug() << "Removing client" << client.socket;

				m_Clients.erase(it);
				return;
			}
		}
	}

	qWarning() << "Can't remove client. Client not found";
}

void TcpServer::UpdateClient(SClient* client)
{
//	QMutexLocker locker(&m_Mutex);

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

bool TcpServer::UpdateProfile(SProfile * profile)
{
	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (it->profile->uid == profile->uid)
			{
				// First update profile in DB
				if (gEnv->pDBWorker->UpdateProfile(profile))
				{
					it->profile = profile;
					qDebug() << "Profile" << profile->nickname << "updated";
					return true;
				}
				else
				{
					qWarning() << "Failed update" << profile->nickname << "profile in DB!";
					return false;
				}
			}
		}
	}

	qDebug() << "Profile" << profile->nickname << "not found.";
	return false;
}

QStringList TcpServer::GetPlayersList()
{
//	QMutexLocker locker(&m_Mutex);

	QStringList playerList;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (!it->profile->nickname.isEmpty())
			{
				playerList.push_back("Uid: " + QString::number(it->profile->uid) +
					" Nickname: " + it->profile->nickname +
					" Level: " + QString::number(it->profile->lvl) +
					" XP: " + QString::number(it->profile->xp));
			}
		}
	}

	return playerList;
}

int TcpServer::GetClientCount()
{
	QMutexLocker locker(&m_Mutex);
	return m_Clients.size();
}

QSslSocket * TcpServer::GetSocketByUid(int uid)
{
	if (uid <= 0)
		return nullptr;

	//QMutexLocker locker(&m_Mutex);
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

SProfile * TcpServer::GetProfileByUid(int uid)
{
	if (uid <= 0)
		return nullptr;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (it->profile->uid == uid)
			{
				qDebug() << "Profile finded. Return";
				return it->profile;
			}
		}
	}

	qDebug() << "Profile not finded.";
	return nullptr;
}

void TcpServer::sendMessageToClient(QSslSocket * socket, NetPacket &packet)
{
	if (socket != nullptr)
	{
		socket->write(packet.toString());
		socket->waitForBytesWritten(10);
	}
}

void TcpServer::sendGlobalMessage(NetPacket &packet)
{
//	QMutexLocker locker(&m_Mutex);
	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		// Send message only for authorizated clients
		if (it->profile != nullptr && it->socket != nullptr)
		{
			it->socket->write(packet.toString());
			it->socket->waitForBytesWritten(10);
		}
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
	connect(gEnv->pTimer, &QTimer::timeout, pThread, &TcpThread::Update);
	//connect(this,&TcpServer::stop, pThread, &TcpThread::stop);

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

	int prevClientsCount = 0;

	for (int i = 0; i < m_pool->maxThreadCount(); i++)
	{
		TcpThread* pThread = m_threads.at(i);
		int clientsCount = pThread->GetClientsCount();

		if (prevClientsCount > clientsCount)	
			return pThread;
		else
			prevClientsCount = clientsCount;
	}

	// Return first thread
	TcpThread* pThread = m_threads.at(0);
    return pThread;
}

void TcpServer::close()
{
	emit stop();

	m_threads.clear();
	m_pool->clear();
}