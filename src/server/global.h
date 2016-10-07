// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef GLOBAL_H
#define GLOBAL_H

class ClientQuerys;
class RedisConnector;
class TcpConnection;
class TcpServer;
class DBWorker;
#include <qsslsocket.h>
#include <qmutex.h>

struct SUser
{
	int uid;
	QString login;
	QString password;
	bool bBanStatus;
};

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
	bool isGameServer;
};

struct SGameServer
{
	QSslSocket* socket;
	QString name;
	QString ip;
	int port;
	QString map;
	QString gamerules;
	int online;
	int maxPlayers;
};

struct SGlobalEnvironment
{
	TcpServer* pServer;
	RedisConnector* pRedis;
	DBWorker* pDataBase;

	bool bUseMySql;
	QString serverIP;
	int serverPort;
	QString serverRootUser;
	QString serverRootPassword;
	int logLevel;
	int maxPlayers;
	int maxServers;
	int maxThreads;

	inline void Init()
	{
		bUseMySql = false;
		serverIP = "127.0.0.1";
		serverPort = 3322;
		serverRootUser = "admin";
		serverRootPassword = "qwerty";
		logLevel = 1;
		maxPlayers = 1000;
		maxServers = 100;
		maxThreads = 1;

		pServer = nullptr;
		pRedis = nullptr;
		pDataBase = nullptr;
	}

};

extern SGlobalEnvironment* gEnv;
extern QVector <SClient> vClients;
extern QVector <SGameServer> vServers;

#endif // GLOBAL_H
