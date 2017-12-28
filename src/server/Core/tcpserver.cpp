// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "tcpserver.h"
#include "tcpthread.h"

#include "Workers/Databases/dbworker.h"
#include "Tools/console.h"

TcpServer::TcpServer(QObject *parent) 
	: QTcpServer(parent)
	, m_maxThreads(0)
	, m_maxConnections(0)
	, m_connectionTimeout(0)
	, m_Time(QTime::currentTime())
	, m_InputPacketsCount(0)
	, m_OutputPacketsCount(0)
	, bClosed(false)
{
}

TcpServer::~TcpServer()
{
}

void TcpServer::Clear()
{
	emit stop();
}

void TcpServer::Update()
{
	// Every one second - calculate server statisctic;
	if (m_Time.elapsed() >= 1000)
	{
		m_Time = QTime::currentTime();
		CalculateStatistic();
	}
}

void TcpServer::MessageReceived()
{
	m_InputPacketsCount++;
}

void TcpServer::MessageSended()
{
	m_OutputPacketsCount++;
}

void TcpServer::SetMaxThreads(const int maximum)
{
	m_maxThreads = maximum;
}

void TcpServer::SetMaxConnections(const int value)
{
	m_maxConnections = value;
}

void TcpServer::SetConnectionTimeout(const int value)
{
	m_connectionTimeout = value;
}

bool TcpServer::Listen(const QHostAddress & address, const quint16 port)
{
	if (m_maxThreads <= 0)
	{
		LogError("[TcpServer] Execute SetMaxThreads function before listen!");
		return false;
	}

	if (!listen(address, port))
	{
		LogError("[TcpServer]  Can't start listen on port <%d>. Error = <%s>", port, errorString().toStdString().c_str());
		return false;
	}

	LogInfo("[TcpServer] Start listing on port <%d>", port);

	Start();

	gEnv->m_ServerStatus.m_MainServerStatus = "Online";

	return true;
}

void TcpServer::Start()
{
	for (int i = 0; i < m_maxThreads; i++)
	{
		TcpThread *runnable = CreateRunnable();
		if (!runnable)
		{
			LogError("[TcpServer] Could not found runable!");
			return;
		}

		StartRunnable(runnable);
	}
}

void TcpServer::CalculateStatistic()
{
	// Update input packet count + speed
	gEnv->m_InputPacketsCount += m_InputPacketsCount;
	gEnv->m_InputSpeed = m_InputPacketsCount;

	// Update max input speed
	if (m_InputPacketsCount > gEnv->m_InputMaxSpeed)
		gEnv->m_InputMaxSpeed = m_InputPacketsCount;

	// Update output packet count + speed
	gEnv->m_OutputPacketsCount += m_OutputPacketsCount;
	gEnv->m_OutputSpeed = m_OutputPacketsCount;

	// Update max output speed
	if (m_OutputPacketsCount > gEnv->m_OutputMaxSpeed)
		gEnv->m_OutputMaxSpeed = m_OutputPacketsCount;

	// Update max clients count
	const int m_ClientCount = GetClientCount();
	if (m_ClientCount > gEnv->m_MaxClientCount)
		gEnv->m_MaxClientCount = m_ClientCount;

	// Refresh local counters
	m_InputPacketsCount = 0;
	m_OutputPacketsCount = 0;
}

TcpThread * TcpServer::CreateRunnable()
{
	LogDebug("[TcpServer] Creating runnable...");

	TcpThread *runnable = new TcpThread();
	runnable->setAutoDelete(false);

	return runnable;
}

void TcpServer::StartRunnable(TcpThread * runnable)
{
	if (runnable == nullptr)
	{
		LogError("[TcpServer] Can't start runnable - nullptr");
		return;
	}

	LogDebug("[TcpServer] Starting <%p> runnable", runnable);

	runnable->setAutoDelete(false);

	m_threads.append(runnable);

	connect(this, &TcpServer::closing, runnable, &TcpThread::closing, Qt::QueuedConnection);
	connect(runnable, &TcpThread::started, this, &TcpServer::started, Qt::QueuedConnection);
	connect(runnable, &TcpThread::finished, this, &TcpServer::finished, Qt::QueuedConnection);
	connect(this, &TcpServer::connecting, runnable, &TcpThread::connecting, Qt::QueuedConnection);

	QThreadPool::globalInstance()->start(runnable);
}

void TcpServer::incomingConnection(qintptr socketDescriptor)
{
	LogDebug("[TcpServer] New incoming connection, accepting...");

	if (GetClientCount() >= m_maxConnections)
	{
		LogError("[TcpServer] Can't accept new client, because server have limit <%d>. Rejecting...", m_maxConnections);
		Reject(socketDescriptor);
	}

	int previous = 0;
	TcpThread *runnable = m_threads.at(0);

	foreach(TcpThread *item, m_threads)
	{
		int count = item->Count();

		if (count == 0 || count < previous)
		{
			runnable = item;
			break;
		}

		previous = count;
	}

	if (runnable == nullptr)
	{
		LogWarning("[TcpServer] Could not find runable!");
		return;
	}

	Accept(socketDescriptor, runnable);
}

void TcpServer::Accept(qintptr handle, TcpThread * runnable)
{
	TcpConnection *connection = new TcpConnection;
	emit connecting(handle, runnable, connection);
}

void TcpServer::Reject(qintptr handle)
{
	QSslSocket *socket = new QSslSocket(this);
	socket->setSocketDescriptor(handle);
	socket->close();
	socket->deleteLater();
}

void TcpServer::finished()
{
	TcpThread *runnable = static_cast<TcpThread*>(sender());

	if (runnable == nullptr)
		return;

	m_threads.removeAll(runnable);

	SAFE_DELETE(runnable);

	if (m_threads.size() <= 0)
	{
		bClosed = true;
	}
}

void TcpServer::stop()
{
	emit closing();
}

void TcpServer::started()
{
	TcpThread *runnable = static_cast<TcpThread*>(sender());
	if (runnable == nullptr) 
		return;

	LogDebug("[TcpServer] Runnable <%p> has started");
}

void TcpServer::AddNewClient(SClient &client)
{
	QMutexLocker locker(&m_Mutex);

	if (client.socket == nullptr)
	{
		LogWarning("[TcpServer] Can't add client. Client socket = nullptr");
		return;
	}

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->socket == client.socket)
		{
			LogWarning("[TcpServer] Can' add client with socket <%p> (%s:%d). Client alredy added", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);
			return;
		}
	}

	LogDebug("[TcpServer] Adding new client with socket <%p> (%s:%d)", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);
	m_Clients.push_back(client);
}

void TcpServer::RemoveClient(SClient &client)
{
	QMutexLocker locker(&m_Mutex);

	if (!client.socket)
	{
		LogWarning("[TcpServer] Can't remove client. Client socket = nullptr");
		return;
	}

	if (m_Clients.size() > 0)
	{
		for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
		{
			if (it->socket == client.socket)
			{
				LogDebug("[TcpServer] Removing client with socket <%p> (%s:%d)", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);

				m_Clients.erase(it);
				return;
			}
		}
	}
	
	LogWarning("[TcpServer] Can't remove client with socket <%p> (%s:%d). Client not found", client.socket, client.GetAddress().ip.c_str(), client.GetAddress().port);
}

void TcpServer::UpdateClient(SClient* client)
{
	QMutexLocker locker(&m_Mutex);

	if (client->socket == nullptr)
	{
		LogWarning("[TcpServer] Can't update client. Client socket = nullptr");
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

				LogDebug("[TcpServer] Client with socket <%p> (%s:%d) updated.", client->socket, client->GetAddress().ip.c_str(), client->GetAddress().port);
				return;
			}
			LogWarning("[TcpServer] Can't update client with socket <%p> (%s:%d). Profile = nullptr", it->socket, it->GetAddress().ip.c_str(), it->GetAddress().port);
			return;
		}
	}

	LogWarning("[TcpServer] Can't update client with socket <%p> (%s:%d). Client not found", client->socket, client->GetAddress().ip.c_str(), client->GetAddress().port);
}

bool TcpServer::UpdateProfile(SProfile * profile)
{
	QMutexLocker locker(&m_Mutex);

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
					LogDebug("[TcpServer] Profile <%s> updated.", profile->nickname.toStdString().c_str());
					return true;
				}
				LogWarning("[TcpServer] Failed update profile <%s> in DB!", profile->nickname.toStdString().c_str());
				return false;
			}
		}
	}

	LogDebug("[TcpServer] Profile <%s> not found.", profile->nickname.toStdString().c_str());
	return false;
}

std::vector<std::string> TcpServer::DumpPlayerList()
{
	QMutexLocker locker(&m_Mutex);

	std::vector<std::string> playerList;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (!it->profile->nickname.isEmpty())
			{
				playerList.push_back(_strFormat("[%d] - [%s] - [%d] - [%d]"
					, it->profile->uid
					, it->profile->nickname.toStdString().c_str()
					, it->profile->lvl
					, it->profile->xp));
			}
		}
	}

	return playerList;
}

int TcpServer::GetClientCount()
{
	QMutexLocker locker(&m_Mutex);
	return static_cast<int>(m_Clients.size());
}

QSslSocket * TcpServer::GetSocketByUid(int uid)
{
	QMutexLocker locker(&m_Mutex);

	if (uid <= 0)
		return nullptr;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (it->profile->uid == uid)
			{
				return it->socket;
			}
		}
	}

	return nullptr;
}

SProfile * TcpServer::GetProfileByUid(int uid)
{
	QMutexLocker locker(&m_Mutex);

	if (uid <= 0)
		return nullptr;

	for (auto it = m_Clients.begin(); it != m_Clients.end(); ++it)
	{
		if (it->profile != nullptr)
		{
			if (it->profile->uid == uid)
			{
				return it->profile;
			}
		}
	}

	return nullptr;
}

void TcpServer::sendMessageToClient(QSslSocket* socket, CTcpPacket &packet) const
{
	if (socket)
		socket->write(packet.toString());
}

void TcpServer::sendGlobalMessage(CTcpPacket &packet)
{
	for (auto it = m_threads.begin(); it != m_threads.end(); ++it)
	{
		(*it)->SendGlobalMessage(packet);
	}
}