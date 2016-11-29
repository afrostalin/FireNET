// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QFile>
#include <QTimer>
// Logging tool
#include <Logger.h>
#include <FileAppender.h>
#include <ConsoleAppender.h>
//
#include "global.h"
#include "tcpserver.h"
#include "dbworker.h"
#include "mysqlconnector.h"
#include "httpconnector.h"
#include "remoteserver.h"
#include "settings.h"

FileAppender *fileAppender;
ConsoleAppender *consoleAppender;

#ifdef _WIN64
#include <signal.h>
static void ClearManager(int sig)
{
	if (sig == SIGBREAK || sig == SIGINT || sig == SIGTERM)
	{
		qInfo() << "FireNET quiting...";

		gEnv->pServer->Clear();
		gEnv->pRemoteServer->Clear();
		gEnv->pDBWorker->Clear();
		gEnv->pSettings->Clear();

		gEnv->pServer->deleteLater();
		gEnv->pRemoteServer->deleteLater();
		gEnv->pDBWorker->deleteLater();
		gEnv->pSettings->deleteLater();

		qApp->quit();
	}
}
#endif

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
	consoleAppender = new ConsoleAppender();

	Logger::LogLevel logLevel;

	switch (level)
	{
	case 1:
	{
		logLevel = Logger::Info;
		break;
	}
	case 2:
	{
		logLevel = Logger::Debug;
	}
	default:
	{
		logLevel = Logger::Debug;
		break;
	}
	}

	consoleAppender->setDetailsLevel(logLevel);
	fileAppender->setDetailsLevel(logLevel);

	logger->registerAppender((AbstractAppender*)fileAppender);
	logger->registerAppender((AbstractAppender*)consoleAppender);
}

void UpdateLogLevel(int lvl)
{
	Logger::LogLevel logLevel;

	switch (lvl)
	{
	case 1:
	{
		logLevel = Logger::Info;
		break;
	}
	case 2:
	{
		logLevel = Logger::Debug;
	}
	default:
	{
		logLevel = Logger::Debug;
		break;
	}
	}

	consoleAppender->setDetailsLevel(logLevel);
	fileAppender->setDetailsLevel(logLevel);
}

void RegisterVariables()
{
	// Server vars
	gEnv->pSettings->RegisterVariable("sv_ip", "127.0.0.1");
	gEnv->pSettings->RegisterVariable("sv_port", 3322);
	gEnv->pSettings->RegisterVariable("sv_root_user", "administrator");
	gEnv->pSettings->RegisterVariable("sv_root_password", "qwerty");
	gEnv->pSettings->RegisterVariable("sv_log_level", 2);
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
	gEnv->pSettings->RegisterVariable("net_magic_key", 2016206);

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
    QCoreApplication *a = new QCoreApplication(argc, argv);

	
	// Init global environment
	gEnv->pServer = new TcpServer;
	gEnv->pRemoteServer = new RemoteServer;
	gEnv->pDBWorker = new DBWorker;
	gEnv->pTimer = new QTimer;	
	gEnv->pSettings = new SettingsManager;

	// Connect pTimer with Update functions
	QObject::connect(gEnv->pTimer, &QTimer::timeout, gEnv->pServer, &TcpServer::Update);
	QObject::connect(gEnv->pTimer, &QTimer::timeout, gEnv->pRemoteServer, &RemoteServer::Update);
	QObject::connect(gEnv->pTimer, &QTimer::timeout, gEnv->pDBWorker, &DBWorker::Update);
	
	// Build version and number
	QString buildVersion = "2.0.6";
	int buildNumber = 101;
	QString appVersion = buildVersion + "." + QString::number(buildNumber);

    a->addLibraryPath("plugins");
    a->setApplicationName("FireNET");
    a->setApplicationVersion(appVersion);
	
	if (init())
	{
		start_logging("FireNET.log", 2);

		qInfo() << "FireNET" << buildVersion << " Build" << buildNumber;
		qInfo() << "Created by Ilya Chernetsov";
		qInfo() << "Copyright (c) All rights reserved";

		RegisterVariables();
		ReadServerCFG();

		UpdateLogLevel(gEnv->pSettings->GetVariable("sv_log_level").toInt());

		if (gEnv->pSettings->GetVariable("db_mode").toString() == "Redis")
		{
			gEnv->pSettings->SetVariable("bUseRedis", true);
			gEnv->pSettings->SetVariable("bUseMySQL", false);
		}
		if (gEnv->pSettings->GetVariable("db_mode").toString() == "MySql")
		{
			gEnv->pSettings->SetVariable("bUseRedis", false);
			gEnv->pSettings->SetVariable("bUseMySQL", true);
		}
		if (gEnv->pSettings->GetVariable("db_mode").toString() == "Redis+MySql")
		{
			gEnv->pSettings->SetVariable("bUseRedis", true);
			gEnv->pSettings->SetVariable("bUseMySQL", true);
		}

		if (gEnv->pSettings->GetVariable("auth_mode").toString() == "HTTP")
			gEnv->pSettings->SetVariable("bUseHttpAuth", true);

		qInfo() << "Start server on" << gEnv->pSettings->GetVariable("sv_ip").toString();

		gEnv->pServer->setMaxThreads(gEnv->pSettings->GetVariable("sv_thread_count").toInt());

		if (gEnv->pServer->listen(QHostAddress(gEnv->pSettings->GetVariable("sv_ip").toString()), gEnv->pSettings->GetVariable("sv_port").toInt()))
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

#ifdef _WIN64
	signal(SIGBREAK, ClearManager);
#endif
	return a->exec();
}
