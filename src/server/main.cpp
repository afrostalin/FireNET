#include "tcpserver.h"
#include "global.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>


int main(int argc, char *argv[])
{
	QCoreApplication *a = new QCoreApplication(argc, argv);

    QCoreApplication::setApplicationName("FireNET");
    QCoreApplication::setApplicationVersion("2.0");

    qDebug() << "*******************************************************************************";
    qDebug() << "*                                FireNET 2.0                                  *";
    qDebug() << "*                           Created by AfroStalin                             *";
    qDebug() << "*                      Copyright (c) All rights reserved                      *";
    qDebug() << "*******************************************************************************";

	qDebug() << "[Main] Start TcpServer....";

	pServer = new TcpServer;
	pServer->setMaxThreads(4);

	if (pServer->listen(QHostAddress::Any, 3322))
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

