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
	if (Connect())
	{
		qInfo() << "Redis connected. Work on" << QThread::currentThread();
	}
	else
	{
        qCritical() << "Failed connect to Redis! Database functions not be work!";
		return;
	}
}

void RedisConnector::Disconnect()
{
	if (connectStatus && connection)
	{
		connection->disconnect();
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

    connectStatus = connection->isConnected();

    return connection->isConnected();
}

QString RedisConnector::SendSyncQuery(QString command)
{
	RedisClient::Response r;

	if (connectStatus && connection != nullptr)
	{
		r = connection->commandSync(command);
		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

QString RedisConnector::SendSyncQuery(QString command, QString key)
{
	RedisClient::Response r;

	if (connectStatus && connection != nullptr)
	{
		r = connection->commandSync(command, key);
		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

QString RedisConnector::SendSyncQuery(QString command, QString key, QString value)
{
	RedisClient::Response r;

	if (connectStatus && connection != nullptr)
	{
		r = connection->commandSync(command, key, value);
		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

QString RedisConnector::SendSyncQuery(QList<QByteArray>& rawCmd)
{
	RedisClient::Response r;

	if (connectStatus && connection != nullptr)
	{
		r = connection->commandSync(rawCmd);

		if (r.isArray())
			return r.toRawString();

		return r.getValue().toString();
	}
	else
		qCritical() << "Error send sync query to Redis DB";

	return QString();
}

