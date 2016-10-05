// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "tcpserver.h"
#include "global.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QFile>
// Logging tool
#include <Logger.h>
#include <FileAppender.h>
#include <ConsoleAppender.h>

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
		logLevel = Logger::Debug;
		break;
	}
	case 2:
	{
		logLevel = Logger::Info;
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

	// Build version and number
	QString buildVersion = "2.0.1";
	int buildNumber = 62;

	QCoreApplication::setApplicationName("FireNET");
	QCoreApplication::setApplicationVersion(buildVersion);

	if (init())
	{
		// Reading server config
		QSettings settings(QString("server.cfg"), QSettings::IniFormat);
		QString serverIP = settings.value("sv_ip", "127.0.0.1").toString();
		int serverPort = settings.value("sv_port", "3322").toInt();
		QString serverAdmin = settings.value("sv_admin", "admin").toString();
		QString serverAdminPassword = settings.value("sv_adminPassword", "qwerty").toString();
		int logLevel = settings.value("sv_loglevel", "1").toInt();
		int maxPlayers = settings.value("sv_maxplayers", "1000").toInt();
		int maxServers = settings.value("sv_maxservers", "100").toInt();
		int maxThreads = settings.value("sv_maxthreads", "4").toInt();
		bool useMySql = settings.value("db_use_mysql", "0").toBool();

		start_logging("FireNET.log", logLevel);

		qInfo() << "FireNET" << buildVersion << " Build" << buildNumber;
		qInfo() << "Created by Ilya Chernetsov";
		qInfo() << "Copyright (c) All rights reserved";
		qInfo() << "Start server on" << serverIP;

		pServer = new TcpServer;
		pServer->setMaxThreads(maxThreads);

		if (pServer->listen(QHostAddress(serverIP), serverPort))
		{
			qInfo() << "Server started!";
			qInfo() << "Start Redis...";

			QThread* redisThread = new QThread;
			pRedis = new RedisConnector;
			pRedis->moveToThread(redisThread);
			QObject::connect(redisThread, &QThread::started, pRedis, &RedisConnector::run);
			QObject::connect(redisThread, &QThread::finished, pRedis, &RedisConnector::deleteLater);
			redisThread->start();

			if (useMySql)
			{
				qInfo() << "Start MySql...";
				// TO DO
			}
		}
		else
		{
			qCritical() << "Server can't start. Reason = " << pServer->errorString();
		}
	}

	return a->exec();
}