// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef GLOBAL_H
#define GLOBAL_H

class TcpServer;
class DBWorker;
class QTimer;
class RemoteServer;

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

struct SRemoteClient
{
	QSslSocket *socket;
	bool isAdmin;
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
	DBWorker* pDataBases;
	QTimer* pTimer;
	RemoteServer* pRemoteServer;
};

extern SGlobalEnvironment* gEnv;
extern QVector <SClient> vClients;
extern QVector<SRemoteClient> vRemoteClients;
extern QVector <SGameServer> vServers;

#endif // GLOBAL_H
