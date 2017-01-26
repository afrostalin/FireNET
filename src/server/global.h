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

// Server status structure
struct SServerStatus
{
	QString m_MainServerStatus;
	QString m_RemoteServerStatus;
	QString m_DBMode;
	QString m_DBStatus;
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

		// Server statisctic
		m_ServerStatus.m_DBMode = "none";
		m_ServerStatus.m_DBStatus = "none";
		m_ServerStatus.m_MainServerStatus = "offline";
		m_ServerStatus.m_RemoteServerStatus = "offline";
		//

		m_LogLevel = 0;

		isQuiting = false;
		isReadyToClose = false;

		m_serverFullName = "FireNET";
	}

	TcpServer*			 pServer;
	DBWorker*			 pDBWorker;
	QTimer*				 pTimer;
	RemoteServer*		 pRemoteServer;
	SettingsManager*	 pSettings;
	Scripts*			 pScripts;
	MainWindow*			 pUI;

	// Server statistic
	SServerStatus		 m_ServerStatus;
	int					 m_LogLevel;
	QString				 m_serverFullName;

	// Need for clearing server before quit
	bool			     isQuiting;
	bool			     isReadyToClose;
};

extern SGlobalEnvironment* gEnv;
#endif // GLOBAL_H