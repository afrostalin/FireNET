// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QThread>

#include "global.h"
#include "redisconnector.h"
#include "dbworker.h"

#include "Tools/settings.h"

RedisConnector::RedisConnector(QObject *parent) : QObject(parent),
	connection(nullptr),
	connectStatus(false)
{
}

RedisConnector::~RedisConnector()
{
	qDebug() << "~RedisConnector";

	if (connectStatus)
		Disconnect();

	SAFE_RELEASE(connection);
}

void RedisConnector::run()
{
	gEnv->m_ServerStatus.m_DBStatus = "connecting";

	if (Connect())
	{
		qInfo() << "Redis connected. Work on" << QThread::currentThread();

		gEnv->m_ServerStatus.m_DBStatus = "online";

		connectStatus = true;
	}
	else
	{
        qCritical() << "Failed connect to Redis! Database functions not be work!";
		gEnv->m_ServerStatus.m_DBStatus = "offline";
		return;
	}
}

void RedisConnector::disconnected()
{
	qCritical() << "Redis server disconnected. Database functions not be work!";

	gEnv->m_ServerStatus.m_DBStatus = "offline";
	connectStatus = false;
}

void RedisConnector::Disconnect()
{
	gEnv->m_ServerStatus.m_DBStatus = "offline";

	if (connectStatus && connection)
	{
		connection->disconnect();
		connectStatus = false;
	}
}

bool RedisConnector::Connect()
{
	qInfo()<< "Init qredisclient...";
    initRedisClient();

	qInfo()<< "Create connection to Redis... ("<< gEnv->pSettings->GetVariable("redis_ip").toString() <<")";
    RedisClient::ConnectionConfig config(gEnv->pSettings->GetVariable("redis_ip").toString());
    connection = new RedisClient::Connection(config);
    connection->connect(false);

	connect(connection, &RedisClient::Connection::error, this, &RedisConnector::disconnected);

    return connection->isConnected();
}

QString RedisConnector::SendSyncQuery(QString command)
{
	if (connectStatus && connection != nullptr)
	{
		RedisClient::Response r = connection->commandSync(command);
		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

QString RedisConnector::SendSyncQuery(QString command, QString key)
{
	if (connectStatus && connection)
	{
		RedisClient::Response r = connection->commandSync(command, key);
		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

QString RedisConnector::SendSyncQuery(QString command, QString key, QString value)
{
	if (connectStatus && connection)
	{
		RedisClient::Response r = connection->commandSync(command, key, value);
		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

QString RedisConnector::SendSyncQuery(QList<QByteArray>& rawCmd)
{
	if (connectStatus && connection)
	{
		RedisClient::Response r = connection->commandSync(rawCmd);

		if (r.isArray())
			return r.toRawString();

		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

