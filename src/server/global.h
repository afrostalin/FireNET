// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QSslSocket>
#include <QHostAddress>
#include <QDebug>
#include <FireNetCore/IFireNetBase.h>

#include <stdarg.h>
#include <stdio.h>

#ifdef WIN32
#include "Windows.h"
#ifdef SendMessage
#undef SendMessage
#endif
#endif

class CServerThread;
class TcpServer;
class DBWorker;
class QTimer;
class RemoteServer; 
class RemoteClientQuerys;
class CConsole;
class Scripts;
class MainWindow;

#define SAFE_DELETE(p) {if(p){delete p; p = nullptr;}}
#define SAFE_RELEASE(p) {if(p){p->deleteLater(); p = nullptr;}}
#define SAFE_CLEAR(p) {if(p){p->Clear();}}
#define TODO(msg) __pragma(message("[TODO] " msg))
#define MAX_LOG_BUFFER_SIZE 1024

inline void LogDebug(const char* format, ...)
{
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qDebug() << buff;
	va_end(args);
}

inline void LogInfo(const char* format, ...)
{
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qInfo() << buff;
	va_end(args);
}

inline void LogWarning(const char* format, ...)
{
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qWarning() << buff;
	va_end(args);
}

inline void LogError(const char* format, ...)
{
	char buff[MAX_LOG_BUFFER_SIZE];
	memset(buff, 0, sizeof(buff));
	va_list args;
	va_start(args, format);
	vsnprintf_s(buff, _countof(buff), _TRUNCATE, format, args);
	qCritical() << buff;
	va_end(args);
}

// Create formated std::string
inline std::string _strFormat(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	std::vector<char> v(1024);
	while (true)
	{
		va_list args2;
		va_copy(args2, args);
		const int res = vsnprintf(v.data(), v.size(), fmt, args2);
		if ((res >= 0) && (res < static_cast<int>(v.size())))
		{
			va_end(args);
			va_end(args2);
			return std::string(v.data());
		}
		size_t size;
		if (res < 0)
			size = v.size() * 2;
		else
			size = static_cast<size_t>(res) + 1;
		v.clear();
		v.resize(size);
		va_end(args2);
	}
}

// Case-insensitive QString comparison
inline bool _qstricmp(const QString &str1, const QString &str2)
{
	if (_stricmp(str1.toStdString().c_str(), str2.toStdString().c_str()) == 0)
		return true;

	return false;
}

// Need for authorization system
struct SUser
{
	int uid = 0;
	QString login;
	QString password;
	bool bBanStatus = false;
};

// Default profile
struct SProfile
{
	int uid = 0;
	QString nickname;
	QString fileModel;
	int lvl = 0;
	int xp = 0;
	int money = 0;
	QString items;
#ifndef STEAM_SDK_ENABLED
	QString friends;
#endif
};

// Shop item structure
struct SShopItem
{
	QString name;
	int cost = 0;
	int minLvl = 0;
	bool canBuy = false;
};

// Trusted server structure
struct STrustedServer
{
	QString name;
	QString ip;
	int port = 0;
};

// Game server sturcture
struct SGameServer
{
	QSslSocket* socket = nullptr;
	QString name;
	QString ip;
	int port = 0;
	QString map;
	QString gamerules;
	int online = 0;
	int maxPlayers = 0;
	EFireNetGameServerStatus status = EGStatus_Unknown;
	int currentPID = 0;

	bool IsValid() const
	{
		return !name.isEmpty() && !ip.isEmpty() && port && !map.isEmpty() && !gamerules.isEmpty() && maxPlayers;
	}
};

// Ip : port
struct SIpPort
{
	SIpPort(std::string _ip, int _port)
		: ip(_ip), port(_port)
	{}

	std::string ip;
	int port = 0;
};

// Dedicated arbitrator
struct SDedicatedArbitrator
{
	QSslSocket* socket = nullptr;
	QString     name;
	int         m_GameServersMaxCount = 0;
	int         m_GameServersCount = 0;
};

// Client structure
struct SClient
{
	QSslSocket* socket = nullptr;
	SProfile* profile = nullptr;
	int status = 0;

	SIpPort GetAddress() const
	{
		if (socket != nullptr)
		{
			return SIpPort(socket->peerAddress().toString().toStdString(), socket->peerPort());
		}
		return SIpPort("unknown", 0);
	}
};

// Remote client structure
struct SRemoteClient
{
	QSslSocket*           socket = nullptr;
	SGameServer*          server = nullptr;
	SDedicatedArbitrator* pArbitrator = nullptr;
	RemoteClientQuerys*   pQuerys = nullptr;


	SIpPort GetAddress() const
	{
		if (socket != nullptr)
		{
			return SIpPort(socket->peerAddress().toString().toStdString(), socket->peerPort());
		}

		return SIpPort("unknown", 0);
	}

	bool isAdmin = false;
	bool isGameServer = false;
	bool isArbitrator = false;
};

// Server status structure
struct SServerStatus
{
	QString m_MainServerStatus = "Offline";
	QString m_RemoteServerStatus = "Offline";
	QString m_DBMode = "None";
	QString m_DBStatus = "None";
};

// Global environment instance
struct SGlobalEnvironment
{
	// Pointers to main server systems
	CServerThread*       pMainThread = nullptr;
	TcpServer*           pServer = nullptr;
	RemoteServer*        pRemoteServer = nullptr;
	DBWorker*            pDBWorker = nullptr;
	QTimer*              pTimer = nullptr;
	CConsole*            pConsole = nullptr;
	Scripts*             pScripts = nullptr;
	MainWindow*          pUI = nullptr;

	// Server statistic
	SServerStatus        m_ServerStatus;	
	int                  m_InputPacketsCount = 0;
	int                  m_InputSpeed = 0;
	int                  m_InputMaxSpeed = 0;
	int                  m_OutputPacketsCount = 0;
	int                  m_OutputSpeed = 0;
	int                  m_OutputMaxSpeed = 0;

	int                  m_DebugsCount = 0;
	int                  m_WarningsCount = 0;
	int                  m_ErrorsCount = 0;

	int                  m_MaxClientCount = 0;

	int                  m_ArbitratorsCount = 0;
	int                  m_GameServersRegistered = 0;

	// Log level for file and UI
	int                  m_FileLogLevel = 0;
	int                  m_UILogLevel = 0;

	// Server full name
	QString              m_serverFullName = "FireNET";
	int                  m_buildNumber = 0;
	QString              m_buildType = "Debug";
	QString              m_buildVersion = "v.2.1.6";

	// Need for clearing server before quit
	bool                 isQuiting = false;
	bool                 isReadyToClose = false;
};

struct SGameMap
{
	QString mapName;
	QString gamerules;
	int     mapID;
};

extern SGlobalEnvironment* gEnv;