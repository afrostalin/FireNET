// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QFile>
// Logging tool
#include <Logger.h>
#include <FileAppender.h>
#include <ConsoleAppender.h>
//
#include "global.h"
#include "tcpserver.h"
#include "dbworker.h"
#include "mysqlconnector.h"

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

	if (!QFile::exists("shop.xml"))
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
	gEnv->Init();

	// Build version and number
	QString buildVersion = "2.0.1";
	int buildNumber = 178;

	QCoreApplication::setApplicationName("FireNET");
	QCoreApplication::setApplicationVersion(buildVersion);

	if (init())
	{
		// Reading server config
		QSettings settings(QString("server.cfg"), QSettings::IniFormat);

		gEnv->serverIP = settings.value("sv_ip", "127.0.0.1").toString();
		gEnv->serverPort = settings.value("sv_port", "3322").toInt();
		gEnv->serverRootUser = settings.value("sv_admin", "admin").toString();
		gEnv->serverRootPassword = settings.value("sv_adminPassword", "qwerty").toString();
		gEnv->logLevel = settings.value("sv_loglevel", "2").toInt();
		gEnv->maxPlayers = settings.value("sv_maxplayers", "1000").toInt();
		gEnv->maxServers = settings.value("sv_maxservers", "100").toInt();
		gEnv->maxThreads = settings.value("sv_maxthreads", "4").toInt();

		// Redis settings
		gEnv->redisHost = settings.value("db_redis_host", "127.0.0.1").toString();
		gEnv->bRedisBackgroundSave = settings.value("db_redis_background_saving", "0").toBool();

		// MySql settings
		gEnv->bUseMySql = settings.value("db_use_mysql", "0").toBool();
		gEnv->mySqlHost = settings.value("db_mysql_host", "127.0.0.1").toString();
		gEnv->mySqlPort = settings.value("db_mysql_port", "3306").toInt();
		gEnv->mySqlDbName = settings.value("db_mysql_database", "FireNET").toString();
		gEnv->mySqlUsername = settings.value("db_mysql_username", "admin").toString();
		gEnv->mySqlPassword = settings.value("db_mysql_password", "qwerty").toString();

		start_logging("FireNET.log", gEnv->logLevel);

		qInfo() << "FireNET" << buildVersion << " Build" << buildNumber;
		qInfo() << "Created by Ilya Chernetsov";
		qInfo() << "Copyright (c) All rights reserved";
		qInfo() << "Start server on" << gEnv->serverIP;

		gEnv->pServer = new TcpServer;
		gEnv->pServer->setMaxThreads(gEnv->maxThreads);

		if (gEnv->pServer->listen(QHostAddress(gEnv->serverIP), gEnv->serverPort))
		{
			// Create database worker
			gEnv->pDataBase = new DBWorker;

			qInfo() << "Server started!";
			qInfo() << "Start Redis...";

			QThread* redisThread = new QThread;
			gEnv->pRedis = new RedisConnector;
			gEnv->pRedis->moveToThread(redisThread);
			QObject::connect(redisThread, &QThread::started, gEnv->pRedis, &RedisConnector::run);
			QObject::connect(redisThread, &QThread::finished, gEnv->pRedis, &RedisConnector::deleteLater);
			redisThread->start();

			if (gEnv->bUseMySql)
			{
				qInfo() << "Start MySql...";

				// If you use MySql you need save all data in Redis first.
				gEnv->bRedisBackgroundSave = true;

				QThread* mysqlThread = new QThread;
				gEnv->pMySql = new MySqlConnector;
				gEnv->pMySql->moveToThread(mysqlThread);
				QObject::connect(mysqlThread, &QThread::started, gEnv->pMySql, &MySqlConnector::run);
				QObject::connect(mysqlThread, &QThread::finished, gEnv->pMySql, &MySqlConnector::deleteLater);
				mysqlThread->start();
			}
		}
		else
		{
			qCritical() << "Server can't start. Reason = " << gEnv->pServer->errorString();
		}
	}

	return a->exec();
}
