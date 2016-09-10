// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef GLOBAL_H
#define GLOBAL_H

class ClientQuerys;
class RedisConnector;
class TcpConnection;
class TcpServer;
#include <qsslsocket.h>
#include <qmutex.h>

struct SProfile
{
	int uid;
	QString nickname;
	QString model;
	int lvl;
	int xp;
	int money;
	QString items;
	QString friends;
	QString achievements;
	QString stats;
};

struct SClient
{
	QSslSocket* socket;
	SProfile* profile;
	int status;
};

extern TcpServer* pServer;
extern RedisConnector* pRedis;
extern QVector <SClient> vClients;

#endif // GLOBAL_H
