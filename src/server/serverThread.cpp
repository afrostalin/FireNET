// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QThread>
#include <QSettings>
#include <QDir>
#include <QFile>
#include <Logger.h>
#include <FileAppender.h>
#include <QMetaObject>
#include <QProcess>

#include "global.h"
#include "serverThread.h"

#include "Core/tcpserver.h"
#include "Core/remoteserver.h"

#include "Workers/Databases/dbworker.h"

#include "Tools/console.h"
#include "Tools/scripts.h"

#include "UI/UILogger.h"
#include "UI/mainwindow.h"

#define LOGS_FOLDER "Logs/"
#define FIRENET_CONFIG "FireNET.cfg"

FileAppender* pLogFileAppender;
UILogger*     pLogUIAppender;

CServerThread::CServerThread(QObject *parent) 
	: QObject(parent)
	, m_loop(nullptr)
{
}

CServerThread::~CServerThread()
{
	SAFE_RELEASE(m_loop);
}

// Console variable callbacks

void UpdateFileLogLevel(QVariant variable)
{
	Logger::LogLevel logLevel;

	const int lvl = variable.toInt();

	switch (lvl)
	{
	case 0:
	{
		logLevel = Logger::Info;
		pLogFileAppender->setFormat(QLatin1String("%{time}{<dd.MM.yy - HH:mm:ss.zzz>} [%{type:-7}] %{message}\n"));
		break;
	}
	case 1:
	{
		logLevel = Logger::Debug;
		pLogFileAppender->setFormat(QLatin1String("%{time}{<dd.MM.yy - HH:mm:ss.zzz>} [%{type:-7}] %{message}\n"));
		break;
	}
	default:
	{
		logLevel = Logger::Debug;
		pLogFileAppender->setFormat(QLatin1String("%{time}{<dd.MM.yy - HH:mm:ss.zzz>} [%{type:-7}] %{message}\n"));
		break;
	}
	}

	pLogFileAppender->setDetailsLevel(logLevel);
	gEnv->m_FileLogLevel = lvl;
}

void UpdateUILogLevel(QVariant variable)
{
	Logger::LogLevel logLevel;

	const int lvl = variable.toInt();

	switch (lvl)
	{
	case 0 :
	{
		logLevel = Logger::Info;
		pLogUIAppender->setFormat(QLatin1String("%{message}"));
		break;
	}
	case 1:
	{
		logLevel = Logger::Info;
		pLogUIAppender->setFormat(QLatin1String("[%{type:-7}] %{message}"));
		break;
	}
	case 2:
	{
		logLevel = Logger::Info;
		pLogUIAppender->setFormat(QLatin1String("%{time}{<HH:mm:ss.zzz>} [%{type:-7}] %{message}"));
		break;
	}
	case 3:
	{
		logLevel = Logger::Debug;
		pLogUIAppender->setFormat(QLatin1String("%{time}{<HH:mm:ss.zzz>} [%{type:-7}] %{message}"));
		break;
	}
	default:
	{
		logLevel = Logger::Debug;
		pLogUIAppender->setFormat(QLatin1String("%{time}{<HH:mm:ss.zzz>} [%{type:-7}] %{message}"));
		break;
	}
	}

	pLogUIAppender->setDetailsLevel(logLevel);
	gEnv->m_UILogLevel = lvl;
}

void UpdateMaxClientCount(QVariant variable)
{
	if (gEnv->pServer)
		gEnv->pServer->SetMaxConnections(variable.toInt());
}

void UpdateMaxRemoteClientCount(QVariant variable)
{
	if (gEnv->pRemoteServer)
	{
		// Game servers + admin + arbitrators
		gEnv->pRemoteServer->SetMaxClientCount(variable.toInt() + 1 + gEnv->pConsole->GetInt("sv_max_arbitrators"));
	}
}

void UpdateStressMode(QVariant variable)
{
	if (variable.toBool())
	{
		gEnv->pMainThread->EnableStressMode();
	}
}

// ~Console variable callbacks

// Console commands
void CmdStatus(QStringList& args)
{
	CConsole* pConsole = gEnv->pConsole;
	if (!pConsole)
	{
		LogError("Can't get settings manager!");
		return;
	}

	std::vector<std::string> m_Stats = gEnv->pMainThread->DumpStatistic();

	for (const auto &it : m_Stats)
	{
		LogWarning("%s", it.c_str());
	}
}

void CmdSendMessage(QStringList& args)
{
	if (args.size() > 1)
	{
		QString message = args[1];

		LogInfo("Try send message to all clients <%s>", message.toStdString().c_str());

		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerMessage);
		msg.WriteStdString(message.toStdString());
		gEnv->pServer->sendGlobalMessage(msg);
	}
}

void CmdSendCommand(QStringList& args)
{
	if (args.size() > 1)
	{
		QString command = args[1];

		LogInfo("Try send command to all clients <%s>", command.toStdString().c_str());

		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerCommand);
		msg.WriteStdString(command.toStdString());
		gEnv->pServer->sendGlobalMessage(msg);
	}
}

void CmdSendRemoteCommand(QStringList& args)
{
	if (args.size() > 1)
	{
		QString command = args[1];

		LogInfo("Try send command to all remote clients <%s>", command.toStdString().c_str());

		CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
		msg.WriteServerMessage(EFireNetTcpSMessage::ServerCommand);
		msg.WriteStdString(command.toStdString());
		gEnv->pRemoteServer->SendMessageToAllRemoteClients(msg);
	}
}

void CmdPrintPlayers(QStringList& args)
{
	std::vector<std::string> players = gEnv->pServer->DumpPlayerList();

	if (players.size() <= 0)
	{
		LogWarning("No available  players for display");
		return;
	}

	for (int i = 0; i < players.size(); i++)
	{
		LogInfo("[UID] - [NICKNAME] - [LEVEL] - [XP]");
		LogInfo("%s", players[i].c_str());
	}
}

void CmdPrintGameServer(QStringList& args)
{
	std::vector<std::string> servers = gEnv->pRemoteServer->DumpServerList();

	if (servers.size() <= 0)
	{
		LogWarning("No available  server for display");
		return;
	}

	LogInfo("[ SERVER IP : PORT] - [GAME MAP : GAMERULES] - [CURRENT PID] - [PLAYERS] - [SERVER STATUS]");

	for (int i = 0; i < servers.size(); i++)
	{
		LogInfo("%s", servers[i].c_str());
	}
}

void CmdClearLog(QStringList& args)
{
	if (gEnv->pUI)
	{
		gEnv->pUI->ClearOutput();
	}
}

void CmdQuit(QStringList& args)
{
	if (gEnv->pUI)
	{
		gEnv->pUI->CleanUp();
	}
}

void CmdList(QStringList& args)
{
	gEnv->pConsole->PrintAllCommands();
}

void CmdKillByPID(QStringList& args)
{
	if (args.size() > 1)
	{
		QString pid = args[1];
		int processID = pid.toInt();

		if (processID > 0)
		{
			LogWarning("Killing process by PID = %d", processID);
			HANDLE hnd;
			hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, static_cast<DWORD>(processID));
			TerminateProcess(hnd, 0);
		}
	}
}

void CmdSpawnGameServer(QStringList& args)
{
	QString exeArgs;
	QString pathToExe = gEnv->pConsole->GetString("sv_gameserver_path").c_str();	

	if (args.size() > 1)
	{
		exeArgs = args[1];
	}

	if (!pathToExe.isEmpty())
	{
		LogInfo("Spawn game server <%s> with params <%s>", pathToExe.toStdString().c_str(), exeArgs.toStdString().c_str());

		if (!QProcess::startDetached(pathToExe + " " + exeArgs))
		{
			LogError("Can't spawn game server!");
		}
	}
}

void CmdRunPool(QStringList& args)
{
	QString pathToExe = gEnv->pConsole->GetString("sv_gameserver_path").c_str();
	int count = gEnv->pConsole->GetInt("sv_gameservers_pool_count");
	if (count && !pathToExe.isEmpty())
	{
		LogInfo("Spawning game server pool with count <%d>", count);

		std::vector<SGameMap> maps = gEnv->pScripts->GetGameMaps();

		for (const auto &it : maps)
		{
			QString exeArgs = _strFormat("-dedicated -simple_console +sv_gamerules %s +map %s", it.gamerules.toStdString().c_str(), it.mapName.toStdString().c_str()).c_str();

			for (int i = 0; i < count / maps.size(); i++)
			{
				LogInfo("Spawn game server with params <%s>", exeArgs.toStdString().c_str());

				if (!QProcess::startDetached(pathToExe + " " + exeArgs))
				{
					LogError("Can't spawn game server!");
				}
			}
		}
	}
}

void CmdKillAll(QStringList &args)
{
	if (gEnv->pRemoteServer)
	{
		std::vector<SGameServer> m_Servers;
		gEnv->pRemoteServer->GetAllGameServers(m_Servers);

		for (const auto &it : m_Servers)
		{
			if (it.currentPID > 0)
			{
				HANDLE hnd;
				hnd = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, TRUE, static_cast<DWORD>(it.currentPID));
				TerminateProcess(hnd, 0);
			}
		}
	}
}

void CmdScriptsReload(QStringList &args)
{
	if (args.size() > 1)
	{
		if (!args[1].isEmpty())
		{
			if (args[1] == "all")
			{
				gEnv->pScripts->ReloadAll();
			}
			else
			{
				LogWarning("Can't reload script by name <%s> now - TODO", args[1].toStdString().c_str());
				TODO("Make reload_script command for reloading scripts by name");
			}
		}
		else
		{
			LogWarning("Can't reload script - empty args");
		}	
	}
	else
	{
		LogWarning("Can't reload script - empty args");
	}
}

void CmdRuntimeStatus(QStringList &args)
{
	pLogUIAppender->EnableRuntimeStatus();
}

void CmdConfigReload(QStringList &args)
{
	gEnv->pMainThread->ReadServerCFG(true);
}

// ~Console commands

bool CServerThread::Init() const
{
	if (!QFile::exists("key.key") || !QFile::exists("key.pem"))
	{
		LogError("Server can't start - SSL key files not found. First create SSL keys for starting server!");
		return false;
	}

	if (!QFile::exists(FIRENET_CONFIG))
	{
		LogWarning("Not found FireNET.cfg! Using default settings...");
	}

	if (!QFile::exists("scripts/shop.xml"))
	{
		LogWarning("Not found shop.xml! Shop system not be work!");
	}

	return true;
}

void CServerThread::StartLogging()
{
	// Check exists logs folder
	if (!QDir(LOGS_FOLDER).exists())
		QDir().mkdir(LOGS_FOLDER);

	QDateTime* pDate = new QDateTime();
	QString m_Date = pDate->currentDateTime().toString("dd MMMM yy (hh mm ss)");
	SAFE_DELETE(pDate);

	QString m_LogFileName = _strFormat(LOGS_FOLDER "FireNet %s %s Build(%d) %s.log", gEnv->m_buildVersion.toStdString().c_str(), gEnv->m_buildType.toStdString().c_str(), gEnv->m_buildNumber, m_Date.toStdString().c_str()).c_str();


#ifdef NDEBUG
	int m_LogLevel = 0;
#else
	int m_LogLevel = 3;
#endif

	// Init logging tool
	pLogFileAppender = new FileAppender(m_LogFileName);
	pLogUIAppender = new UILogger();

	UpdateFileLogLevel(m_LogLevel);
	UpdateUILogLevel(m_LogLevel);
	
	logger->registerAppender((AbstractAppender*)pLogFileAppender);
	logger->registerAppender((AbstractAppender*)pLogUIAppender);
}

void CServerThread::RegisterVariables()
{
	// Server commands
	gEnv->pConsole->RegisterCommand("status", &CmdStatus, "Get full server status");
	gEnv->pConsole->RegisterCommand("send_message", &CmdSendMessage, "Send global message to all connected clients");
	gEnv->pConsole->RegisterCommand("send_command", &CmdSendCommand, "Send console command to all connected remote clients");
	gEnv->pConsole->RegisterCommand("send_remote_command", &CmdSendRemoteCommand, "Send console command to all connected clients");
	gEnv->pConsole->RegisterCommand("players", &CmdPrintPlayers, "Print all connected players");
	gEnv->pConsole->RegisterCommand("servers", &CmdPrintGameServer, "Print all registered game servers");
	gEnv->pConsole->RegisterCommand("clear", &CmdClearLog, "Clear UI output");
	gEnv->pConsole->RegisterCommand("list", &CmdList, "List all console commands");
	gEnv->pConsole->RegisterCommand("quit", &CmdQuit, "Close FireNet");
	gEnv->pConsole->RegisterCommand("killPID", &CmdKillByPID, "Kill process by PID (Only need if FireNet not using dedicated arbitrator)");
	gEnv->pConsole->RegisterCommand("killAll", &CmdKillAll, "Kill all game servers (Only need if FireNet not using dedicated arbitrator)");
	gEnv->pConsole->RegisterCommand("spawnGameServer", &CmdSpawnGameServer, "Spawn game server with params (Only need if FireNet not using dedicated arbitrator)");
	gEnv->pConsole->RegisterCommand("runPool", &CmdRunPool, "Spawn game servers pool (Only need if FireNet not using dedicated arbitrator)");
	gEnv->pConsole->RegisterCommand("reload_script", &CmdScriptsReload, "Reload script by name (or 'all')");
	gEnv->pConsole->RegisterCommand("runtimeStatus", &CmdRuntimeStatus, "Enable/Disable runtime status (disabled all logs and show runtime server status)");
	gEnv->pConsole->RegisterCommand("reload_config", &CmdConfigReload, "Reload config");

	// Server vars
	gEnv->pConsole->RegisterVariable("sv_role", "Master", "Server role [Master/Slave]", false);
	gEnv->pConsole->RegisterVariable("sv_ip", "127.0.0.1", "Main server ip address", false);
	gEnv->pConsole->RegisterVariable("sv_port", 3322, "Main server port", false);
	gEnv->pConsole->RegisterVariable("sv_ui_width_text_size", 152, "Maximum message lenght in UI window", true);
#ifdef QT_NO_DEBUG
	gEnv->pConsole->RegisterVariable("sv_file_log_level", 0, "Log level for control debugging messages in log file [0-1]", true, &UpdateFileLogLevel);
	gEnv->pConsole->RegisterVariable("sv_ui_log_level", 0, "Log level for control debugging messages in UI [0-3]", true, &UpdateUILogLevel);
#else
	gEnv->pConsole->RegisterVariable("sv_file_log_level", 1, "Log level for control debugging messages in log file [0-1]", true, &UpdateFileLogLevel);
	gEnv->pConsole->RegisterVariable("sv_ui_log_level", 3, "Log level for control debugging messages in UI [0-3]", true, &UpdateUILogLevel);
#endif
	gEnv->pConsole->RegisterVariable("sv_thread_count", 1, "Main server thread count for thread pooling", false);
	gEnv->pConsole->RegisterVariable("sv_max_players", 1000, "Maximum players count for connection", true, &UpdateMaxClientCount);
	gEnv->pConsole->RegisterVariable("sv_max_servers", 100, "Maximum game servers count for connection", true, &UpdateMaxRemoteClientCount);
	gEnv->pConsole->RegisterVariable("sv_max_arbitrators", 10, "Maximum dedicated arbitrators for registering", true, &UpdateMaxRemoteClientCount);
	gEnv->pConsole->RegisterVariable("sv_tickrate", 30, "Main server tick rate speed (30 by Default)", false);

	// Game servers
	gEnv->pConsole->RegisterVariable("sv_gameserver_path", "", "Path to game server executeble for running and managing it (Only need if FireNet not using dedicated arbitrator)", true);
	gEnv->pConsole->RegisterVariable("sv_gameservers_pool_count", 10, "How many game servers need spawn (Only need if FireNet not using dedicated arbitrator)", true);

	// Remote server vars
	gEnv->pConsole->RegisterVariable("remote_root_user", "administrator", "Remote admin login", true);
	gEnv->pConsole->RegisterVariable("remote_root_password", "qwerty", "Remote admin password", true);
	gEnv->pConsole->RegisterVariable("remote_server_port", 64000, "Remote server port", false);
	gEnv->pConsole->RegisterVariable("remote_ping_timeout", 5000, "Remote server ping timeout (ms)", false);

	// Database vars
	gEnv->pConsole->RegisterVariable("db_mode", "Redis", "Database mode [Redis, MySql, Redis+MySql, None]", false);

	// Redis vars
	gEnv->pConsole->RegisterVariable("redis_ip", "127.0.0.1", "Redis database ip address", false);
	gEnv->pConsole->RegisterVariable("redis_port", 6379, "Redis database port", false);
	gEnv->pConsole->RegisterVariable("redis_bg_saving", false, "Use redis background saving", true);

	// MySQL vars
	gEnv->pConsole->RegisterVariable("mysql_host", "127.0.0.1", "MySql database ip address", false);
	gEnv->pConsole->RegisterVariable("mysql_port", 3306, "MySql database port", false);
	gEnv->pConsole->RegisterVariable("mysql_db_name", "FireNET", "MySql database name", false);
	gEnv->pConsole->RegisterVariable("mysql_username", "admin", "MySql username", false);
	gEnv->pConsole->RegisterVariable("mysql_password", "password", "MySql password", false);

	// Network vars
	gEnv->pConsole->RegisterVariable("net_encryption_timeout", 3, "Network timeout for new connection", true);
	gEnv->pConsole->RegisterVariable("net_version", "1.0.0.1", "Network version for check packets validations", true);
	gEnv->pConsole->RegisterVariable("net_max_packet_size", 512 , "Maximum packet size for reading", true);
	gEnv->pConsole->RegisterVariable("net_max_bad_packets_count", 10, "Maximum bad packets count from client", true);
	gEnv->pConsole->RegisterVariable("net_max_packets_speed", 4, "Maximum packets per second count by client", true);
	gEnv->pConsole->RegisterVariable("net_packet_debug", false, "Enable/Disable packet debugging", true);

	// Utils
	gEnv->pConsole->RegisterVariable("bUseStressMode", false, "Changes server settings to work with stress test", false, &UpdateStressMode);
	gEnv->pConsole->RegisterVariable("bUseGlobalChat", false, "Enable/Disable global chat", true);
	gEnv->pConsole->RegisterVariable("bUseTrustedServers", true, "Enable/Disable using trusted server", false);
	gEnv->pConsole->RegisterVariable("bUseScriptsSystem", false, "Enable/Disable using scripts system", false);
	gEnv->pConsole->RegisterVariable("bUseExperimentalFeatures", false, "Enable/Disable experimental features", true);
	gEnv->pConsole->RegisterVariable("bAutoSpawningGameSevers", false, "Enable/Disable automatic game server spawning when it crashed", true);
	gEnv->pConsole->RegisterVariable("bUseDedicatedArbitrators", true, "Enable/Disable using dedicated arbitrators", true);

	// Gloval vars (This variables not need read from server.cfg)
	gEnv->pConsole->RegisterVariable("bUseRedis", true, "Enable/Disable using Redis database", false);
	gEnv->pConsole->RegisterVariable("bUseMySQL", false, "Enable/Disable using MySql database", false);
	gEnv->pConsole->RegisterVariable("bUseHttpAuth", false, "Enable/Disable using HTTP authorization", false);
}

void CServerThread::ReadServerCFG(bool online)
{
	QSettings settings(QString(FIRENET_CONFIG), QSettings::IniFormat);

	QStringList serverCFG = settings.allKeys();

	// Read server.cfg and try set new variable values
	for (int i = 0; i < serverCFG.size(); ++i)
	{
		if (serverCFG[i].contains("#") || serverCFG[i].contains("//"))
			continue;

		QVariant::Type var_type = gEnv->pConsole->GetVariable(serverCFG[i]).type();

		switch (var_type)
		{
		case QVariant::Bool:
		{
			bool value = (settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toInt() == 1) ? true : false;

			QString strValue = settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toString();
			if (strValue == "true")
				value = true;
			else if (strValue == "false")
				value = false;
	

			if (gEnv->pConsole->GetVariable(serverCFG[i]).toBool() != value)
			{
				if (online)
					LogInfo("%s = %s", serverCFG[i].toStdString().c_str(), settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toString().toStdString().c_str());

				gEnv->pConsole->SetVariable(serverCFG[i], value);			
			}

			break;
		}
		case QVariant::Int:
		{
			int value = settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toInt();

			if (gEnv->pConsole->GetVariable(serverCFG[i]).toInt() != value)
			{
				if (online)
					LogInfo("%s = %s", serverCFG[i].toStdString().c_str(), settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toString().toStdString().c_str());

				gEnv->pConsole->SetVariable(serverCFG[i], value);
			}

			break;
		}
		case QVariant::Double:
		{
			double value = settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toDouble();

			if (gEnv->pConsole->GetVariable(serverCFG[i]).toDouble() != value)
			{
				if (online)
					LogInfo("%s = %s", serverCFG[i].toStdString().c_str(), settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toString().toStdString().c_str());

				gEnv->pConsole->SetVariable(serverCFG[i], value);			
			}

			break;
		}
		case QVariant::String:
		{
			QString value = settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toString();

			if (gEnv->pConsole->GetVariable(serverCFG[i]).toString() != value)
			{
				if (online)
					LogInfo("%s = %s", serverCFG[i].toStdString().c_str(), settings.value(serverCFG[i], gEnv->pConsole->GetVariable(serverCFG[i])).toString().toStdString().c_str());

				gEnv->pConsole->SetVariable(serverCFG[i], value);
			}	

			break;
		}
		default:
			break;
		}
	}
}

std::vector<std::string> CServerThread::DumpStatistic()
{
	std::vector<std::string> stats;

	CConsole* pConsole = gEnv->pConsole; // Can't be null

	stats.push_back("--------- Full server status ---------");

	// Main server status
	stats.push_back(_strFormat("Server version : %s", gEnv->m_serverFullName.toStdString().c_str()));
	stats.push_back(_strFormat("Main server address (%s:%d) - %s", pConsole->GetString("sv_ip").c_str(), pConsole->GetInt("sv_port"), gEnv->m_ServerStatus.m_MainServerStatus.toStdString().c_str()));
	stats.push_back(_strFormat("Clients count : %d/%d", gEnv->pServer->GetClientCount(), gEnv->pServer->GetMaxClientCount()));
	stats.push_back(_strFormat("Maximum active clients count : %d", gEnv->m_MaxClientCount));
	stats.push_back(_strFormat("Thread count : %d", gEnv->pConsole->GetInt("sv_thread_count")));
	stats.push_back(_strFormat("Server tickrate : %d per/sec.", gEnv->pConsole->GetInt("sv_tickrate")));

	// Remote server status
	stats.push_back(_strFormat("Remote server address (%s:%d) - %s", pConsole->GetString("sv_ip").c_str(), pConsole->GetInt("remote_server_port"), gEnv->m_ServerStatus.m_RemoteServerStatus.toStdString().c_str()));
	stats.push_back(_strFormat("Remote admin : %s", gEnv->pRemoteServer->IsHaveAdmin() ? "online" : "offline"));
	stats.push_back(_strFormat("Game servers count : %d/%d", gEnv->m_GameServersRegistered, pConsole->GetInt("sv_max_servers")));
	stats.push_back(_strFormat("Arbitrators count : %d/%d", gEnv->m_ArbitratorsCount, pConsole->GetInt("sv_max_arbitrators")));

	// Database info 
	stats.push_back(_strFormat("Database mode : %s", gEnv->m_ServerStatus.m_DBMode.toStdString().c_str()));
	stats.push_back(_strFormat("Database status : %s", gEnv->m_ServerStatus.m_DBStatus.toStdString().c_str()));

	// Packets info
	stats.push_back(_strFormat("Input packets count : %d", gEnv->m_InputPacketsCount));
	stats.push_back(_strFormat("Input packets current speed : %d packets/sec.", gEnv->m_InputSpeed));
	stats.push_back(_strFormat("Input packets max speed : %d packets/sec.", gEnv->m_InputMaxSpeed));

	stats.push_back(_strFormat("Output packets count : %d", gEnv->m_OutputPacketsCount));
	stats.push_back(_strFormat("Output packets current speed : %d packets/sec.", gEnv->m_OutputSpeed));
	stats.push_back(_strFormat("Output packets max speed : %d packets/sec.", gEnv->m_OutputMaxSpeed));

	// Debug messages
	stats.push_back(_strFormat("Debug messages : %d", gEnv->m_DebugsCount));
	stats.push_back(_strFormat("Warning messages : %d", gEnv->m_WarningsCount));
	stats.push_back(_strFormat("Error messages : %d", gEnv->m_ErrorsCount));
	
	return stats;
}

void CServerThread::EnableStressTest()
{
	gEnv->pConsole->SetVariable("sv_ui_log_level", 0);
	gEnv->pConsole->SetVariable("sv_file_log_level", 0);
	gEnv->pConsole->SetVariable("sv_max_players", 1000);
	gEnv->pConsole->SetVariable("net_max_bad_packets_count", 1000);
	gEnv->pConsole->SetVariable("net_max_packets_speed", 100);
	gEnv->pConsole->SetVariable("net_packet_debug", false);

	emit EnableStressMode();
}

void CServerThread::start()
{
	m_loop = new QEventLoop();

	// Init global environment
	gEnv->pServer = new TcpServer();
	gEnv->pRemoteServer = new RemoteServer();
	gEnv->pDBWorker = new DBWorker();
	gEnv->pTimer = new QTimer();
	gEnv->pConsole = new CConsole();
	gEnv->pScripts = new Scripts();

	// Connect pTimer with Update functions
	connect(gEnv->pTimer, &QTimer::timeout, gEnv->pServer, &TcpServer::Update);
	connect(gEnv->pTimer, &QTimer::timeout, gEnv->pRemoteServer, &RemoteServer::Update);

	if (Init())
	{
		StartLogging();

		RegisterVariables();
		ReadServerCFG();

		LogInfo(gEnv->m_serverFullName.toStdString().c_str());
		LogInfo("Created by Ilya Chernetsov");
		LogInfo("Copyright (c) 2014-2017. All rights reserved");

		if (gEnv->pConsole->GetBool("bUseScriptsSystem"))
		{
			// Load scripts 
			gEnv->pScripts->LoadShopScript();
			gEnv->pScripts->LoadTrustedServerList();
		}

		if (gEnv->pConsole->GetVariable("db_mode").toString() == "Redis")
		{
			gEnv->pConsole->SetVariable("bUseRedis", true);
			gEnv->pConsole->SetVariable("bUseMySQL", false);
		}
		else if (gEnv->pConsole->GetVariable("db_mode").toString() == "MySql")
		{
			gEnv->pConsole->SetVariable("bUseRedis", false);
			gEnv->pConsole->SetVariable("bUseMySQL", true);
		}
		else if (gEnv->pConsole->GetVariable("db_mode").toString() == "Redis+MySql")
		{
			gEnv->pConsole->SetVariable("bUseRedis", true);
			gEnv->pConsole->SetVariable("bUseMySQL", true);
		}
		else if (gEnv->pConsole->GetVariable("db_mode").toString() == "None")
		{
			gEnv->pConsole->SetVariable("bUseRedis", false);
			gEnv->pConsole->SetVariable("bUseMySQL", false);
		}

		gEnv->m_ServerStatus.m_DBMode = gEnv->pConsole->GetVariable("db_mode").toString();

		// Block online update for some variables
		gEnv->pConsole->BlockOnlineUpdate();

		LogInfo("Start server on %s", gEnv->pConsole->GetVariable("sv_ip").toString().toStdString().c_str());

		gEnv->pServer->SetMaxThreads(gEnv->pConsole->GetVariable("sv_thread_count").toInt());

		if (gEnv->pServer->Listen(QHostAddress(gEnv->pConsole->GetVariable("sv_ip").toString()), gEnv->pConsole->GetVariable("sv_port").toInt()))
		{
			LogInfo("Server started. Main thread <%d>", this->thread()->currentThreadId());

			// Start remote server
			gEnv->pRemoteServer->run();		

			// Set server tickrate
			LogInfo("Server tickrate set to %d per/sec.", gEnv->pConsole->GetVariable("sv_tickrate").toInt());

			int tick = 1000 / gEnv->pConsole->GetInt("sv_tickrate");
			gEnv->pTimer->start(tick);

			if (gEnv->pConsole->GetVariable("db_mode").toString() != "None")
			{
				// Init connection to databases
				gEnv->pDBWorker->Init();
			}
		}
		else
		{
			LogError("Server can't start. Reason = %s", gEnv->pServer->errorString().toStdString().c_str());
		}
	}

	m_loop->exec();
}

void CServerThread::stop() const
{
	if (gEnv->isQuiting)
		return;
	gEnv->isQuiting = true;

	LogInfo("FireNET closing...");

	if (gEnv->pTimer)
		gEnv->pTimer->stop();

	SAFE_CLEAR(gEnv->pServer);
	SAFE_CLEAR(gEnv->pRemoteServer);
	SAFE_CLEAR(gEnv->pConsole);
	SAFE_CLEAR(gEnv->pScripts);
	SAFE_CLEAR(gEnv->pDBWorker);

	while (gEnv->pServer && !gEnv->pServer->IsClosed())
	{
		QEventLoop loop;
		QTimer::singleShot(100, &loop, &QEventLoop::quit);
		loop.exec();
	}

	SAFE_RELEASE(gEnv->pTimer);
	SAFE_RELEASE(gEnv->pServer);
	SAFE_RELEASE(gEnv->pRemoteServer);
	SAFE_RELEASE(gEnv->pConsole);
	SAFE_RELEASE(gEnv->pScripts);
	SAFE_RELEASE(gEnv->pDBWorker);

	QThreadPool::globalInstance()->waitForDone(1000);

	if (m_loop)
		m_loop->exit();	

	gEnv->isReadyToClose = true;

	LogInfo("FireNET closed.");
}
