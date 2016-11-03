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

#include <signal.h>

static void ClearManager(int sig)
{
	gEnv->pServer->close();
	qApp->quit();
}

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
	FileAppender *fileAppender = new FileAppender(logName);
	ConsoleAppender *consoleAppender = new ConsoleAppender();

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

int main(int argc, char *argv[])
{
    QCoreApplication *a = new QCoreApplication(argc, argv);

	// Init global environment
	gEnv->pServer = new TcpServer;
	gEnv->pRemoteServer = new RemoteServer;
	gEnv->pDataBases = new DBWorker;
	gEnv->pTimer = new QTimer;	

	// Build version and number
	QString buildVersion = "2.0.4";
	int buildNumber = 42;

    a->addLibraryPath("plugins");
    a->setApplicationName("FireNET");
    a->setApplicationVersion(buildVersion);

	if (init())
	{
		// Reading server config
		QSettings settings(QString("server.cfg"), QSettings::IniFormat);

		gEnv->pServer->serverIP = settings.value("sv_ip", "127.0.0.1").toString();
		gEnv->pServer->serverPort = settings.value("sv_port", "3322").toInt();
		gEnv->pServer->serverRootUser = settings.value("sv_admin", "admin").toString();
		gEnv->pServer->serverRootPassword = settings.value("sv_adminPassword", "qwerty").toString();
		gEnv->pServer->logLevel = settings.value("sv_loglevel", "2").toInt();
		gEnv->pServer->maxPlayers = settings.value("sv_maxplayers", "1").toInt();
		gEnv->pServer->maxServers = settings.value("sv_maxservers", "1").toInt();
		gEnv->pServer->maxThreads = settings.value("sv_maxthreads", "1").toInt();
		gEnv->pServer->tickRate = settings.value("sv_tickrate", "30").toInt();

		// Database mode (Redis, MySql, Redis+MySql)
		QString db_mode = settings.value("db_mode", "Redis").toString();

		if (db_mode == "Redis")
		{
			gEnv->pDataBases->bUseRedis = true;
			gEnv->pDataBases->bUseMySql = false;
		}
		if (db_mode == "MySql")
		{
			gEnv->pDataBases->bUseMySql = true;
			gEnv->pDataBases->bUseRedis = false;
		}
		if (db_mode == "Redis+MySql")
		{
			gEnv->pDataBases->bUseMySql = true;
			gEnv->pDataBases->bUseRedis = true;
		}

		// Authorization mode (Default, HTTP)
		QString auth_mode = settings.value("auth_mode", "Default").toString();

		if (auth_mode == "HTTP")
			gEnv->pDataBases->bUseAuthByHTTP = true;
		else
			gEnv->pDataBases->bUseAuthByHTTP = false;
		

		// Redis settings
		gEnv->pDataBases->redisHost = settings.value("db_redis_host", "127.0.0.1").toString();
		gEnv->pDataBases->bRedisBackgroundSave = settings.value("db_redis_background_saving", "0").toBool();

		// MySql settings
		gEnv->pDataBases->mySqlHost = settings.value("db_mysql_host", "127.0.0.1").toString();
		gEnv->pDataBases->mySqlPort = settings.value("db_mysql_port", "3306").toInt();
		gEnv->pDataBases->mySqlDbName = settings.value("db_mysql_database", "FireNET").toString();
		gEnv->pDataBases->mySqlUsername = settings.value("db_mysql_username", "admin").toString();
		gEnv->pDataBases->mySqlPassword = settings.value("db_mysql_password", "qwerty").toString();

		gEnv->pDataBases->mySqlUsersTableName = settings.value("db_mysql_auth_table_name", "users").toString();
		gEnv->pDataBases->mySqlUsersUidName = settings.value("db_mysql_auth_uid_element_name", "uid").toString();
		gEnv->pDataBases->mySqlUsersLoginName = settings.value("db_mysql_auth_login_element_name", "login").toString();
		gEnv->pDataBases->mySqlUsersPasswordName = settings.value("db_mysql_auth_password_element_name", "password").toString();
		gEnv->pDataBases->mySqlUsersBanName = settings.value("db_mysql_auth_ban_status_element_name", "ban").toString();

		// HTTP settings
		gEnv->pDataBases->http_authPage = settings.value("http_auth_page", "http://127.0.0.1/auth.php").toString();
		gEnv->pDataBases->http_regPage = settings.value("http_reg_page", "http://127.0.0.1/reg.php").toString();

		// Network settings
		gEnv->pServer->bGlobalChatEnable = settings.value("net_global_chat_enable", "0").toBool();

		// Remote server settings
		gEnv->pRemoteServer->serverIp = settings.value("rs_ip", "127.0.0.1").toString();
		gEnv->pRemoteServer->serverPort = settings.value("rs_port", "64000").toInt();

		start_logging("FireNET.log", gEnv->pServer->logLevel);

		qInfo() << "FireNET" << buildVersion << " Build" << buildNumber;
		qInfo() << "Created by Ilya Chernetsov";
		qInfo() << "Copyright (c) All rights reserved";
		qInfo() << "Start server on" << gEnv->pServer->serverIP;

		qDebug() << "Main thread " << QThread::currentThread();

		gEnv->pServer->setMaxThreads(gEnv->pServer->maxThreads);

		if (gEnv->pServer->listen(QHostAddress(gEnv->pServer->serverIP), gEnv->pServer->serverPort))
		{
			qInfo() << "Server started!";

			// Create thread with Redis connection
			if (gEnv->pDataBases->bUseRedis)
			{
				qInfo() << "Start Redis service...";
				QThread* redisThread = new QThread;
				gEnv->pDataBases->pRedis = new RedisConnector;
				gEnv->pDataBases->pRedis->moveToThread(redisThread);
				QObject::connect(redisThread, &QThread::started, gEnv->pDataBases->pRedis, &RedisConnector::run);
				QObject::connect(redisThread, &QThread::finished, gEnv->pDataBases->pRedis, &RedisConnector::deleteLater);
				redisThread->start();
			}

			// Create thread with MySQL connection
			if (gEnv->pDataBases->bUseMySql)
			{
				qInfo() << "Start MySql service...";
				gEnv->pDataBases->bRedisBackgroundSave = true;
				QThread* mysqlThread = new QThread;
				gEnv->pDataBases->pMySql = new MySqlConnector;
				gEnv->pDataBases->pMySql->moveToThread(mysqlThread);
				QObject::connect(mysqlThread, &QThread::started, gEnv->pDataBases->pMySql, &MySqlConnector::run);
				QObject::connect(mysqlThread, &QThread::finished, gEnv->pDataBases->pMySql, &MySqlConnector::deleteLater);
				mysqlThread->start();
			}

			// Create HTTP connector
			if (gEnv->pDataBases->bUseAuthByHTTP)
			{
				qWarning() << "Authorization mode set to HTTP, this can slows server";
				gEnv->pDataBases->pHTTP = new HttpConnector;
			}
		}
		else
		{
			qCritical() << "Server can't start. Reason = " << gEnv->pServer->errorString();
		}
	}

	// Start remote server for remote administration and game server connection
	qInfo() << "Start remote server...";
	QThread* remoteThread = new QThread;
	gEnv->pRemoteServer->moveToThread(remoteThread);
	QObject::connect(remoteThread, &QThread::started, gEnv->pRemoteServer, &RemoteServer::run);
	QObject::connect(remoteThread, &QThread::finished, gEnv->pRemoteServer, &RemoteServer::deleteLater);
	remoteThread->start();

	// Calculate and set server tick rate
	int tick = 1000 / gEnv->pServer->tickRate;
	gEnv->pTimer->start(tick);

	signal(SIGBREAK, ClearManager);
	return a->exec();
}
