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
#define SAFE_CLEAR(p) {if(p){p->Clear();}}

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
		m_InputPacketsCount = 0;
		m_InputSpeed = 0;
		m_InputMaxSpeed = 0;
		m_OutputPacketsCount = 0;
		m_OutputSpeed = 0;
		m_OutputMaxSpeed = 0;

		m_DebugsCount = 0;
		m_WarningsCount = 0;
		m_ErrorsCount = 0;

		m_MaxClientCount = 0;

		m_serverFullName = "FireNET";

		m_FileLogLevel = 0;
		m_UILogLevel = 0;
		
		isQuiting = false;
		isReadyToClose = false;
	}

	// Pointers to main server systems
	TcpServer*			 pServer;
	RemoteServer*		 pRemoteServer;
	DBWorker*			 pDBWorker;
	QTimer*				 pTimer;
	SettingsManager*	 pSettings;
	Scripts*			 pScripts;
	MainWindow*			 pUI;

	// Server statistic
	SServerStatus		 m_ServerStatus;	
	int                  m_InputPacketsCount;
	int                  m_InputSpeed;
	int                  m_InputMaxSpeed;
	int                  m_OutputPacketsCount;
	int                  m_OutputSpeed;
	int                  m_OutputMaxSpeed;

	int                  m_DebugsCount;
	int                  m_WarningsCount;
	int                  m_ErrorsCount;

	int                  m_MaxClientCount;

	// Log level for file and UI
	int					 m_FileLogLevel;
	int                  m_UILogLevel;

	// Server full name
	QString				 m_serverFullName;

	// Need for clearing server before quit
	bool			     isQuiting;
	bool			     isReadyToClose;
};

extern SGlobalEnvironment* gEnv;
#endif // GLOBAL_H