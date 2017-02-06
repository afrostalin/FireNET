// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef REDISCONNECTOR_H
#define REDISCONNECTOR_H

#include <QObject>

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
	bool                                         IsConnected();
	void										 Disconnect();
public:
	bool                                         HEXISTS(const QString &key, const QString &field);
	bool										 HMSET(const QString &key, const std::vector<std::pair<std::string, std::string>>& field_val);
	QVector<std::pair<std::string, std::string>> HGETALL(const QString &key);
	bool										 SET(const QString &key, const QString &value);
	QString										 GET(const QString &key);	
	void										 BGSAVE();
public slots:
	void										 disconnected();	
private:
	cpp_redis::redis_client*					 pClient;
};

#endif // REDISCONNECTOR_H
