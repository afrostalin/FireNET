// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QThread>
#include <QEventLoop>
#include <cpp_redis/cpp_redis>

#include "global.h"
#include "redisconnector.h"

#include "Tools/console.h"

RedisConnector::RedisConnector(QObject *parent) 
	: QObject(parent)
	, pClient(nullptr)
{
	connect(&m_Timer, &QTimer::timeout, this, &RedisConnector::update);
}

RedisConnector::~RedisConnector()
{
	m_Timer.stop();

	if (IsConnected())
		Disconnect();

	SAFE_DELETE(pClient);
}

void RedisConnector::run()
{
	gEnv->m_ServerStatus.m_DBStatus = "connecting";

	if (Connect())
	{
		LogInfo("Redis connected. Work on <%p>", QThread::currentThread());
		gEnv->m_ServerStatus.m_DBStatus = "Online";
	}
	else
	{
		LogError("Failed connect to Redis! Database functions not be work!");
		gEnv->m_ServerStatus.m_DBStatus = "Offline";
		return;
	}
}

bool RedisConnector::Connect()
{
	pClient = new cpp_redis::redis_client();

	std::string ip = gEnv->pConsole->GetString("redis_ip");
	int port = gEnv->pConsole->GetInt("redis_port");

	try
	{
		LogDebug("Connecting to redis...");
		pClient->connect(ip, port);
	}
	catch (const cpp_redis::redis_error& error)
	{
		LogDebug("Redis error %s", error.what());
		return false;
	}

	LogDebug("Redis connected");
	m_Timer.start(1000);

	return true;
}

bool RedisConnector::IsConnected() const
{
	return pClient && pClient->is_connected() ? true : false;
}

void RedisConnector::Disconnect()
{
	gEnv->m_ServerStatus.m_DBStatus = "Offline";

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
				LogDebug("HEXISTS success. Result <%d>", reply.as_integer());			

				result = reply.as_integer() == 1 ? true : false;
			});		

			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			LogWarning("HEXISTS error - %s", error.what());
			result = false;
		}
	}
	else
	{
		LogError("Redis not connected");
	}

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
				LogDebug("HMSET success. Result <%s>" ,  reply.as_string().c_str());
				result = reply.as_string() == "OK" ? true : false;
			});

			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			LogWarning("HMSET error - %s", error.what());
		}
	}
	else
	{
		LogWarning("Redis not connected");
	}

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
					LogDebug("HGETALL success. Array size <%d>", replyArray.size());

					if (replyArray.size() > 0)
					{
						for (int i = 0; i < replyArray.size() - 1; i++)
						{
							std::pair<std::string, std::string> m_KeyValue;
							m_KeyValue.first = replyArray[i].as_string();
							m_KeyValue.second = replyArray[++i].as_string();

							LogDebug("Key (%s) value (%s)", m_KeyValue.first.c_str(), m_KeyValue.second.c_str());

							m_Result.push_back(m_KeyValue);
						}
					}
				}
			});

			pClient->sync_commit();

		}
		catch (const cpp_redis::redis_error& error)
		{
			LogWarning("HGETALL error - %s", error.what());
		}
	}
	else
	{
		LogWarning("Redis not connected");
	}

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
				LogDebug("SET success. Result <%s>", reply.as_string().c_str());
				result = reply.as_string() == "OK" ? true : false;
			});

			pClient->sync_commit();

		}
		catch (const cpp_redis::redis_error& error)
		{
			LogWarning("SET error - %s", error.what());
		}
	}
	else
	{
		LogWarning("Redis not connected");
	}

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
					LogDebug("GET success. Result <%s>", result.toStdString().c_str());
				}
				else if (reply.is_null())
				{
					result = QString();
					LogDebug("GET success. Result NULL");
				}
			});

			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			LogWarning("GET error - %s", error.what());
		}
	}
	else
	{
		LogWarning("Redis not connected");
	}

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
				LogDebug("BGSAVE success");
			});
			pClient->sync_commit();
		}
		catch (const cpp_redis::redis_error& error)
		{
			LogWarning("BGSAVE error - %s", error.what());
		}
	}
	else
	{
		LogWarning("Redis not connected");
	}
}

void RedisConnector::update()
{
	if (!IsConnected())
	{
		emit disconnected();
	}
}

void RedisConnector::disconnected()
{
	LogError("Redis server disconnected. Database functions not be work!");
	gEnv->m_ServerStatus.m_DBStatus = "Offline";
	m_Timer.stop();
}
