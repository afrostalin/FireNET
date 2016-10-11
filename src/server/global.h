// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef GLOBAL_H
#define GLOBAL_H

class ClientQuerys;
class RedisConnector;
class MySqlConnector;
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
	MySqlConnector* pMySql;
	DBWorker* pDataBase;

	// Redis settings
	QString redisHost;
	bool bRedisBackgroundSave;

	// MySql settings
	bool bUseMySql;
	QString mySqlHost;
	int mySqlPort;
	QString mySqlDbName;
	QString mySqlUsername;
	QString mySqlPassword;

	// MySql "users" table settings (can use old table without creating new table)
	QString mySqlUsersTableName;
	QString mySqlUsersUidName;
	QString mySqlUsersLoginName;
	QString mySqlUsersPasswordName;
	QString mySqlUsersBanName;

	// Server settings
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
		// Redis settings
		redisHost = "127.0.0.1";
		bRedisBackgroundSave = false;

		// MySql settings
		bUseMySql = false;
		mySqlHost = "127.0.0.1";
		mySqlDbName = "FireNET";
		mySqlPort = 3306;
		mySqlUsername = "admin";
		mySqlPassword = "qwerty";

		mySqlUsersTableName = "users";
		mySqlUsersUidName = "uid";
		mySqlUsersLoginName = "login";
		mySqlUsersPasswordName = "password";
		mySqlUsersBanName = "ban";

		// Server settings
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
		pMySql = nullptr;
		pDataBase = nullptr;
	}

};

extern SGlobalEnvironment* gEnv;
extern QVector <SClient> vClients;
extern QVector <SGameServer> vServers;

#endif // GLOBAL_H
