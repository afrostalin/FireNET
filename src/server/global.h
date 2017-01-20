// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef GLOBAL_H
#define GLOBAL_H

class TcpServer;
class DBWorker;
class QTimer;
class RemoteServer;
class SettingsManager;
class Scripts;
class MainWindow;

#include <QSslSocket>
#include <QDebug>

// Safe deleting
#define SAFE_DELETE(p) {if(p){delete p; p = nullptr;}}
#define SAFE_RELEASE(p) {if(p){p->deleteLater(); p = nullptr;}}
#define SAFE_CLEAR(p) {if(p) {p->Clear();}}

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
	SGlobalEnvironment()
	{
		pServer = nullptr;
		pDBWorker = nullptr;
		pTimer = nullptr;
		pRemoteServer = nullptr;
		pSettings = nullptr;
		pScripts = nullptr;
		pUI = nullptr;

		isQuiting = false;
		isReadyToClose = false;
	}

	TcpServer* pServer;
	DBWorker* pDBWorker;
	QTimer* pTimer;
	RemoteServer* pRemoteServer;
	SettingsManager* pSettings;
	Scripts* pScripts;
	MainWindow* pUI;


	bool isQuiting;
	bool isReadyToClose;
};

extern SGlobalEnvironment* gEnv;

#endif // GLOBAL_H