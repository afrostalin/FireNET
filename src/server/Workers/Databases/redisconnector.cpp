// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QThread>
#include <QEventLoop>
#include <QTimer>
#include <cpp_redis/cpp_redis>

#include "global.h"
#include "redisconnector.h"
#include "dbworker.h"

#include "Tools/settings.h"

RedisConnector::RedisConnector(QObject *parent) : QObject(parent),
	pClient(nullptr)
{
}

RedisConnector::~RedisConnector()
{
	qDebug() << "~RedisConnector";

	if (IsConnected())
		Disconnect();

	SAFE_DELETE(pClient);
}

void RedisConnector::run()
{
	gEnv->m_ServerStatus.m_DBStatus = "connecting";

	if (Connect())
	{
		qInfo() << "Redis connected. Work on" << QThread::currentThread();
		gEnv->m_ServerStatus.m_DBStatus = "online";
	}
	else
	{
        qCritical() << "Failed connect to Redis! Database functions not be work!";
		gEnv->m_ServerStatus.m_DBStatus = "offline";
		return;
	}
}

bool RedisConnector::Connect()
{
	pClient = new cpp_redis::redis_client();

	std::string ip = gEnv->pSettings->GetVariable("redis_ip").toString().toStdString();
	int port = gEnv->pSettings->GetVariable("redis_port").toInt();

	try
	{
		qDebug() << "Connecting to redis...";

		pClient->connect(ip, port);
	}
	catch (const cpp_redis::redis_error& error)
	{
		qDebug() << "ERROR" << error.what();
		return false;
	}

	qDebug() << "Redis connected";

	return true;
}

bool RedisConnector::IsConnected()
{
	if (pClient && pClient->is_connected())
		return true;
	else
		return false;
}

void RedisConnector::Disconnect()
{
	gEnv->m_ServerStatus.m_DBStatus = "offline";

	if (IsConnected())
		pClient->disconnect();
}

bool RedisConnector::HEXISTS(const QString & key, const QString & field)
{
	bool result = false;

	if (pClient->is_connected())
	{
		try
		{			
			pClient->hexists(key.toStdString(), field.toStdString(), [&](cpp_redis::reply& reply)
			{
				qDebug() << "HEXISTS success. Result" << reply.as_integer();			

				result = reply.as_integer() == 1 ? true : false;
			});		

			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			qWarning() << "HEXISTS error - " << error.what();
			result = false;
		}
	}
	else
		qCritical() << "Redis not connected";

	return result;
}

bool RedisConnector::HMSET(const QString & key, const std::vector<std::pair<std::string, std::string>>& field_val)
{
	bool result = false;

	if (pClient->is_connected())
	{
		try
		{
			pClient->hmset(key.toStdString(), field_val, [&](cpp_redis::reply & reply)
			{
				qDebug() << "HMSET success. Result" << reply.as_string().c_str();

				result = reply.as_string() == "OK" ? true : false;
			});

			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			qWarning() << "HMSET error - " << error.what();
		}
	}
	else
		qWarning() << "Redis not connected";

	return result;
}

QVector<std::pair<std::string, std::string>> RedisConnector::HGETALL(const QString & key)
{
	QVector<std::pair<std::string, std::string>> m_Result;

	if (pClient->is_connected())
	{
		try
		{
			pClient->hgetall(key.toStdString(), [&](cpp_redis::reply & reply)
			{
				if (reply.is_array())
				{
					std::vector<cpp_redis::reply> replyArray = reply.as_array();
					qDebug() << "HGETALL success. Array size" << replyArray.size();

					if (replyArray.size() > 0)
					{
						for (int i = 0; i < replyArray.size() - 1; i++)
						{
							std::pair<std::string, std::string> m_KeyValue;
							m_KeyValue.first = replyArray[i].as_string();
							m_KeyValue.second = replyArray[++i].as_string();

							qDebug() << "Key (" << m_KeyValue.first.c_str() << ") Value (" << m_KeyValue.second.c_str() << ")";

							m_Result.push_back(m_KeyValue);
						}
					}
				}
			});

			pClient->sync_commit();

		}
		catch (const cpp_redis::redis_error& error)
		{
			qWarning() << "HGETALL error - " << error.what();
		}
	}
	else
		qWarning() << "Redis not connected";

	return m_Result;
}

bool RedisConnector::SET(const QString & key, const QString & value)
{
	bool result = false;

	if (pClient->is_connected())
	{
		try
		{
			pClient->set(key.toStdString(), value.toStdString(), [&](cpp_redis::reply & reply)
			{
				qDebug() << "SET success. Result" << reply.as_string().c_str();

				result = reply.as_string() == "OK" ? true : false;
			});

			pClient->sync_commit();

		}
		catch (const cpp_redis::redis_error& error)
		{
			qWarning() << "SET error - " << error.what();
		}
	}
	else
		qWarning() << "Redis not connected";

	return result;
}

QString RedisConnector::GET(const QString & key)
{
	QString result;

	if (pClient->is_connected())
	{
		try
		{
			pClient->get (key.toStdString(), [&](cpp_redis::reply & reply)
			{
				if (reply.is_bulk_string())
				{
					result = reply.as_string().c_str();
					qDebug() << "GET success. Result" << result;
				}
				else if (reply.is_null())
				{
					result = QString();
					qDebug() << "GET success. Result NULL";
				}
			});

			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			qWarning() << "GET error - " << error.what();
		}
	}
	else
		qWarning() << "Redis not connected";

	return result;
}

void RedisConnector::BGSAVE()
{
	if (pClient->is_connected())
	{
		try
		{
			pClient->bgsave([](cpp_redis::reply & reply)
			{
				qDebug() << "BGSAVE success";
			});
			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			qWarning() << "BGSAVE error - " << error.what();
		}
	}
	else
		qWarning() << "Redis not connected";
}

// TODO - DEPRICATED
void RedisConnector::disconnected()
{
	qCritical() << "Redis server disconnected. Database functions not be work!";
	gEnv->m_ServerStatus.m_DBStatus = "offline";
}
