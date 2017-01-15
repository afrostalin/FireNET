// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef REDISCONNECTOR_H
#define REDISCONNECTOR_H

#include <QObject>
#include <QThread>
#include "qredisclient/redisclient.h"

class RedisConnector : public QObject
{
    Q_OBJECT
public:
    explicit RedisConnector(QObject *parent = 0);
	~RedisConnector();
public:
	void run();
	void Disconnect();
	QString SendSyncQuery(QString command);
	QString SendSyncQuery(QString command, QString key);
    QString SendSyncQuery(QString command, QString key, QString value);
	QString SendSyncQuery(QList<QByteArray> &rawCmd);
private:
	bool Connect();
public:
	bool connectStatus;
private:
	QThread *m_thread;
	RedisClient::Connection* connection;
};

#endif // REDISCONNECTOR_H
