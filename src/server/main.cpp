// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QApplication>
#include <QThread>
#include <QSettings>
#include <QFile>
#include <QTimer>
#include <Logger.h>
#include <FileAppender.h>
#include <QMetaObject>

#include "global.h"

#include "Core/tcpserver.h"
#include "Core/remoteserver.h"

#include "Workers/Databases/dbworker.h"
#include "Workers/Databases/mysqlconnector.h"
#include "Workers/Databases/httpconnector.h"

#include "Tools/settings.h"
#include "Tools/scripts.h"
#include "Tools/UILogger.h"

#include "UI/mainwindow.h"

QApplication *pApp;
FileAppender *fileAppender;
UILogger    *uiAppender;

bool init()
{
	if (!QFile::exists("key.key") || !QFile::exists("key.pem"))
	{
		qCritical() << "Server can't start - SSL key files not found. First create SSL keys for starting server!";
		return false;
	}

	if (!QFile::exists("server.cfg"))
	{
		qWarning() << "Not found server.cfg! Using default settings...";
	}

	if (!QFile::exists("scripts/shop.xml"))
	{
		qWarning() << "Not found shop.xml! Shop system not be work!";
	}

	return true;
}

void start_logging(QString logName, int level)
{
	// Backup old log file
	QFile::remove(logName + ".bak");
	QFile::rename(logName, logName + ".bak");
	QFile::remove(logName);

	// Init logging tool
	fileAppender = new FileAppender(logName);
	uiAppender = new UILogger();

	Logger::LogLevel logLevel;

	switch (level)
	{
	case 0:
	{
		logLevel = Logger::Info;
		break;
	}
	case 1:
	{
		logLevel = Logger::Debug;
	}
	default:
	{
		logLevel = Logger::Debug;
		break;
	}
	}

	fileAppender->setDetailsLevel(logLevel);
#ifdef QT_NO_DEBUG
	fileAppender->setFormat(QLatin1String("%{time}{dd-MM-yyyyTHH:mm:ss.zzz} [%{type:-7}] %{message}\n"));
#else
	fileAppender->setFormat(QLatin1String("%{time}{dd-MM-yyyyTHH:mm:ss.zzz} [%{type:-7}] <%{function}> %{message}\n"));
#endif

	uiAppender->setDetailsLevel(logLevel);
#ifdef QT_NO_DEBUG
	uiAppender->setFormat(QLatin1String("[%{type:-7}] %{message}"));
#else
	uiAppender->setFormat(QLatin1String("[%{type:-7}] <%{function}> %{message}"));
#endif

	gEnv->m_LogLevel = level;

	logger->registerAppender((AbstractAppender*)fileAppender);
	logger->registerAppender((AbstractAppender*)uiAppender);
}

void UpdateLogLevel(int lvl)
{
	Logger::LogLevel logLevel;

	switch (lvl)
	{
	case 0:
	{
		logLevel = Logger::Info;
		break;
	}
	case 1:
	{
		logLevel = Logger::Debug;
	}
	default:
	{
		logLevel = Logger::Debug;
		break;
	}
	}

	gEnv->m_LogLevel = lvl;
	fileAppender->setDetailsLevel(logLevel);
	uiAppender->setDetailsLevel(logLevel);
}

void RegisterVariables()
{
	// Server vars
	gEnv->pSettings->RegisterVariable("sv_ip", "127.0.0.1");
	gEnv->pSettings->RegisterVariable("sv_port", 3322);
	gEnv->pSettings->RegisterVariable("sv_root_user", "administrator");
	gEnv->pSettings->RegisterVariable("sv_root_password", "qwerty");
#ifdef QT_NO_DEBUG
	gEnv->pSettings->RegisterVariable("sv_log_level", 0);
#else
	gEnv->pSettings->RegisterVariable("sv_log_level", 2);
#endif
	gEnv->pSettings->RegisterVariable("sv_thread_count", 1);
	gEnv->pSettings->RegisterVariable("sv_max_players", 1);
	gEnv->pSettings->RegisterVariable("sv_max_servers", 1);
	gEnv->pSettings->RegisterVariable("sv_tickrate", 30);
	// Remote server vars
	gEnv->pSettings->RegisterVariable("remote_server_port", 64000);
	// Database vars
	gEnv->pSettings->RegisterVariable("db_mode", "Redis");
	gEnv->pSettings->RegisterVariable("auth_mode", "Default");
	// Redis vars
	gEnv->pSettings->RegisterVariable("redis_ip", "127.0.0.1");
	gEnv->pSettings->RegisterVariable("redis_bg_saving", false);
	// MySQL vars
	gEnv->pSettings->RegisterVariable("mysql_host", "127.0.0.1");
	gEnv->pSettings->RegisterVariable("mysql_port", 3306);
	gEnv->pSettings->RegisterVariable("mysql_db_name", "FireNET");
	gEnv->pSettings->RegisterVariable("mysql_username", "admin");
	gEnv->pSettings->RegisterVariable("mysql_password", "password");
	// HTTP authorization vars
	gEnv->pSettings->RegisterVariable("http_login_page", "http://127.0.0.1/login.php");
	gEnv->pSettings->RegisterVariable("http_register_page", "http://127.0.0.1/reg.php");
	// Network vars
	gEnv->pSettings->RegisterVariable("bUseGlobalChat", false);
	gEnv->pSettings->RegisterVariable("net_encryption_timeout", 3);
	gEnv->pSettings->RegisterVariable("net_magic_key", 2016207);
	gEnv->pSettings->RegisterVariable("bUsePacketDebug", false);

	// Gloval vars (This variables not need read from server.cfg)
	gEnv->pSettings->RegisterVariable("bUseRedis", true);
	gEnv->pSettings->RegisterVariable("bUseMySQL", false);
	gEnv->pSettings->RegisterVariable("bUseHttpAuth", false);
}

void ReadServerCFG()
{
	QSettings settings(QString("server.cfg"), QSettings::IniFormat);

	QStringList serverCFG = settings.allKeys();

	for (int i = 0; i < serverCFG.size(); ++i)
	{
		// Read server.cfg and try set new variable values
		gEnv->pSettings->SetVariable(serverCFG[i], settings.value(serverCFG[i], gEnv->pSettings->GetVariable(serverCFG[i])));
	}
}

int main(int argc, char *argv[])
{
    pApp = new QApplication(argc, argv);
	
	// UI
	gEnv->pUI = new MainWindow();
	gEnv->pUI->show();

	// Init global environment
	gEnv->pServer = new TcpServer;
	gEnv->pRemoteServer = new RemoteServer;
	gEnv->pDBWorker = new DBWorker;
	gEnv->pTimer = new QTimer;	
	gEnv->pSettings = new SettingsManager;
	gEnv->pScripts = new Scripts;

	// Connect pTimer with Update functions
	QObject::connect(gEnv->pTimer, &QTimer::timeout, gEnv->pServer, &TcpServer::Update);
	QObject::connect(gEnv->pTimer, &QTimer::timeout, gEnv->pRemoteServer, &RemoteServer::Update);

	// Connect quit with clean up function
	QObject::connect(pApp, &QApplication::aboutToQuit, gEnv->pUI, &MainWindow::CleanUp);
	
	// Build version and number
	QString buildVersion = "v.2.1.0";
	int buildNumber = 123;
	QString appVersion = buildVersion + "." + QString::number(buildNumber);
	
	pApp->addLibraryPath("plugins");
	pApp->setApplicationName("FireNET");
	pApp->setApplicationVersion(appVersion);
	
	if (init())
	{
#ifdef QT_NO_DEBUG
		start_logging("FireNET.log", 0);
		QString buildType = ". Release profile";
#else
		start_logging("FireNET.log", 2);
		QString buildType = ". Debug profile";
#endif 
		QString fullName = "FireNET " + buildVersion  + ". Build " + QString::number(buildNumber) + buildType;

		qInfo() << fullName.toStdString().c_str();
		qInfo() << "Created by Ilya Chernetsov";
		qInfo() << "Copyright (c) 2014-2017. All rights reserved";

		RegisterVariables();
		ReadServerCFG();

		UpdateLogLevel(gEnv->pSettings->GetVariable("sv_log_level").toInt());

		// Load scripts 
		gEnv->pScripts->LoadShopScript();
		gEnv->pScripts->LoadTrustedServerList();

		if (gEnv->pSettings->GetVariable("db_mode").toString() == "Redis")
		{
			gEnv->pSettings->SetVariable("bUseRedis", true);
			gEnv->pSettings->SetVariable("bUseMySQL", false);
		}
		else if (gEnv->pSettings->GetVariable("db_mode").toString() == "MySql")
		{
			gEnv->pSettings->SetVariable("bUseRedis", false);
			gEnv->pSettings->SetVariable("bUseMySQL", true);
		}
		else if (gEnv->pSettings->GetVariable("db_mode").toString() == "Redis+MySql")
		{
			gEnv->pSettings->SetVariable("bUseRedis", true);
			gEnv->pSettings->SetVariable("bUseMySQL", true);
		}
		else if (gEnv->pSettings->GetVariable("auth_mode").toString() == "HTTP")
			gEnv->pSettings->SetVariable("bUseHttpAuth", true);

		qInfo() << "Start server on" << gEnv->pSettings->GetVariable("sv_ip").toString();

		gEnv->pServer->SetMaxThreads(gEnv->pSettings->GetVariable("sv_thread_count").toInt());	
		gEnv->pServer->SetMaxConnections(gEnv->pSettings->GetVariable("sv_max_players").toInt());

		if (gEnv->pServer->Listen(QHostAddress(gEnv->pSettings->GetVariable("sv_ip").toString()), gEnv->pSettings->GetVariable("sv_port").toInt()))
		{
			qInfo() << "Server started. Main thread " << QThread::currentThread();			

     		// Start remote server
			gEnv->pRemoteServer->run();

			// Calculate and set server tick rate
			int tick = 1000 / gEnv->pSettings->GetVariable("sv_tickrate").toInt();
			qInfo() << "Server tickrate set to" << gEnv->pSettings->GetVariable("sv_tickrate").toInt() << "per/sec.";
			gEnv->pTimer->start(tick);

			// Init connection to databases
			gEnv->pDBWorker->Init();
		}
		else
		{
			qCritical() << "Server can't start. Reason = " << gEnv->pServer->errorString();
		}
	}

	return pApp->exec();
}
