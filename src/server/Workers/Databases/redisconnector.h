// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include <QTimer>

namespace cpp_redis
{
	class redis_client;
}

class RedisConnector : public QObject
{
    Q_OBJECT
public:
    explicit RedisConnector(QObject *parent = nullptr);
	~RedisConnector();
public:
	void                                         run();
	bool                                         Connect();
	bool                                         IsConnected() const;
	void                                         Disconnect();
public:
	bool                                         HEXISTS(const QString &key, const QString &field);
	bool                                         HMSET(const QString &key, const std::vector<std::pair<std::string, std::string>>& field_val);
	QVector<std::pair<std::string, std::string>> HGETALL(const QString &key);
	bool                                         SET(const QString &key, const QString &value);
	QString                                      GET(const QString &key);	
	void                                         BGSAVE();
public slots:
	void                                         disconnected();
	void                                         update();
private:
	cpp_redis::redis_client*                     pClient;
	QTimer                                       m_Timer;
};