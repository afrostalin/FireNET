// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef GLOBAL_H
#define GLOBAL_H

class TcpServer;
class DBWorker;
class QTimer;
class RemoteServer;
class SettingsManager;
class Scripts;

#include <QSslSocket>
#include <QDebug>

// Need for authorization system
struct SUser
{
	int uid;
	QString login;
	QString password;
	bool bBanStatus;
};

// Default profile
struct SProfile
{
	int uid;
	QString nickname;
	QString fileModel;
	int lvl;
	int xp;
	int money;
	QString items;
	QString friends;
};

// Client structure
struct SClient
{
	QSslSocket* socket;
	SProfile* profile;
	int status;
};

// Shop item structure
struct SShopItem
{
	QString name;
	int cost;
	int minLnl;
	bool canBuy;
};

// Trusted server structure
struct STrustedServer
{
	QString name;
	QString ip;
	int port;
};

// Game server sturcture
struct SGameServer
{
	QString name;
	QString ip;
	int port;
	QString map;
	QString gamerules;
	int online;
	int maxPlayers;
};

// Remote client sturcture
struct SRemoteClient
{
	QSslSocket* socket;
	SGameServer* server;

	bool isAdmin;
	bool isGameServer;
};

// Global environment instance
struct SGlobalEnvironment
{
	TcpServer* pServer;
	DBWorker* pDBWorker;
	QTimer* pTimer;
	RemoteServer* pRemoteServer;
	SettingsManager* pSettings;
	Scripts* pScripts;
};

extern SGlobalEnvironment* gEnv;
#endif // GLOBAL_H
