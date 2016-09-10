// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "tcpserver.h"
#include "global.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QSettings>


int main(int argc, char *argv[])
{
	QCoreApplication *a = new QCoreApplication(argc, argv);

    QCoreApplication::setApplicationName("FireNET");
    QCoreApplication::setApplicationVersion("2.0");

    qDebug() << "*******************************************************************************";
    qDebug() << "*                                FireNET 2.0                                  *";
    qDebug() << "*                         Created by Ilya Chernetsov                          *";
    qDebug() << "*                      Copyright (c) All rights reserved                      *";
    qDebug() << "*******************************************************************************";

	qDebug() << "Reading server configuration...";

	// Reading server config
	QSettings settings(QString("server.cfg"), QSettings::IniFormat);

	QString serverIP = settings.value("sv_ip", "127.0.0.1").toString();
	int serverPort = settings.value("sv_port", "3322").toInt();
	QString serverAdmin = settings.value("sv_admin", "admin").toString();
	QString serverAdminPassword = settings.value("sv_adminPassword", "qwerty").toString();
	int debugLevel = settings.value("sv_debuglevel", "0").toInt();
	int logLevel = settings.value("sv_loglevel", "0").toInt();
	int maxPlayers = settings.value("sv_maxplayers", "1000").toInt();
	int maxServers = settings.value("sv_maxservers", "100").toInt();
	int maxThreads = settings.value("sv_maxthreads", "4").toInt();

	qDebug() << "[Main] Start server on" << serverIP;

	pServer = new TcpServer;
	pServer->setMaxThreads(maxThreads);

	if (pServer->listen(QHostAddress(serverIP), serverPort))
	{
		qDebug() << "[Main] Server started!";

		qDebug() << "[Main] Start redis....";
		QThread* redisThread = new QThread;
		pRedis = new RedisConnector;
		pRedis->moveToThread(redisThread);
		QObject::connect(redisThread, &QThread::started, pRedis, &RedisConnector::run);
		QObject::connect(redisThread, &QThread::finished, pRedis, &RedisConnector::deleteLater);
		redisThread->start();
	}
	else
	{
		qDebug() << "[Main] Server can't start. Reason = " << pServer->errorString();
	}

    return a->exec();
}

