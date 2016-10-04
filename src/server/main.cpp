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

int logLevel;

int main(int argc, char *argv[])
{
	QCoreApplication *a = new QCoreApplication(argc, argv);

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

	// Backup old log file
	QFile::remove("FireNET.log.bak");
	QFile::rename("FireNET.log", "FireNET.log.bak");
	QFile::remove("FireNET.log");

	// Init logging tool
	FileAppender *fileAppender = new FileAppender("FireNET.log");
	ConsoleAppender *consoleAppender = new ConsoleAppender();

	Logger::LogLevel level;
	switch (logLevel)
	{
	case 1:
	{
		level = Logger::Debug;
		break;
	}
	case 2:
	{
		level = Logger::Info;
	}
	default:
		break;
	}

	consoleAppender->setDetailsLevel(level);
	fileAppender->setDetailsLevel(level);
	logger->registerAppender((AbstractAppender*)fileAppender);
	logger->registerAppender((AbstractAppender*)consoleAppender);

	// Build version and number
	QString buildVersion = "2.0.1";
	int buildNumber = 48;

	QCoreApplication::setApplicationName("FireNET");
	QCoreApplication::setApplicationVersion(buildVersion);

	qInfo() << "FireNET" << buildVersion << " Build" << buildNumber;
	qInfo() << "Created by Ilya Chernetsov";
	qInfo() << "Copyright (c) All rights reserved";
	qInfo() << "Start server on" << serverIP;

	pServer = new TcpServer;
	pServer->setMaxThreads(maxThreads);

	if (pServer->listen(QHostAddress(serverIP), serverPort))
	{
		qInfo() << "Server started!";
		qInfo() << "Start redis...";

		QThread* redisThread = new QThread;
		pRedis = new RedisConnector;
		pRedis->moveToThread(redisThread);
		QObject::connect(redisThread, &QThread::started, pRedis, &RedisConnector::run);
		QObject::connect(redisThread, &QThread::finished, pRedis, &RedisConnector::deleteLater);
		redisThread->start();
	}
	else
	{
		qCritical() << "Server can't start. Reason = " << pServer->errorString();
	}

	return a->exec();
}


































