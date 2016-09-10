// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef REDISCONNECTOR_H
#define REDISCONNECTOR_H

#include <QObject>
#include <QThread>
#include <QEventLoop>
#include <QDebug>
#include <QMutex>
#include "qredisclient/redisclient.h"

class RedisConnector : public QObject
{
    Q_OBJECT
public:
    explicit RedisConnector(QObject *parent = 0);
public:
	void run();
    QString SendSyncQuery(QString command, QString key, QString value);
private:
	bool Connect();
public:
	bool connectStatus;
private:
	QEventLoop *m_loop;
	QThread *m_thread;
	RedisClient::Connection* connection;
};

#endif // REDISCONNECTOR_H
