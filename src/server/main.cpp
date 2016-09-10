// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "tcpserver.h"
#include "global.h"
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QScopedPointer>
#include <QTextStream>
#include <QDateTime>
#include <QLoggingCategory>

QScopedPointer<QFile> m_logFile;
int logLevel;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

int main(int argc, char *argv[])
{
	QCoreApplication *a = new QCoreApplication(argc, argv);

	QCoreApplication::setApplicationName("FireNET");
	QCoreApplication::setApplicationVersion("2.0");

	qInfo() << "*******************************************************************************";
	qInfo() << "*                                FireNET 2.0                                  *";
	qInfo() << "*                         Created by Ilya Chernetsov                          *";
	qInfo() << "*                      Copyright (c) All rights reserved                      *";
	qInfo() << "*******************************************************************************";


	// Init logging
	QString logFileName = "FireNET.log";
	QFile::remove(logFileName);
	m_logFile.reset(new QFile(logFileName));
	m_logFile.data()->open(QFile::Append | QFile::Text);
	qInstallMessageHandler(messageHandler);

	// Reading server config
	qInfo() << "[Main] Reading server configuration...";
	QSettings settings(QString("server.cfg"), QSettings::IniFormat);

	QString serverIP = settings.value("sv_ip", "127.0.0.1").toString();
	int serverPort = settings.value("sv_port", "3322").toInt();
	QString serverAdmin = settings.value("sv_admin", "admin").toString();
	QString serverAdminPassword = settings.value("sv_adminPassword", "qwerty").toString();
	logLevel = settings.value("sv_loglevel", "0").toInt();
	int maxPlayers = settings.value("sv_maxplayers", "1000").toInt();
	int maxServers = settings.value("sv_maxservers", "100").toInt();
	int maxThreads = settings.value("sv_maxthreads", "4").toInt();

	qWarning() << "Test warning";
	qDebug() << "Test debug";

	qInfo() << "[Main] Start server on" << serverIP;

	pServer = new TcpServer;
	pServer->setMaxThreads(maxThreads);

	if (pServer->listen(QHostAddress(serverIP), serverPort))
	{
		qInfo() << "[Main] Server started!";

		qInfo() << "[Main] Start redis....";
		QThread* redisThread = new QThread;
		pRedis = new RedisConnector;
		pRedis->moveToThread(redisThread);
		QObject::connect(redisThread, &QThread::started, pRedis, &RedisConnector::run);
		QObject::connect(redisThread, &QThread::finished, pRedis, &RedisConnector::deleteLater);
		redisThread->start();
	}
	else
	{
		qCritical() << "[Main] Server can't start. Reason = " << pServer->errorString();
	}

	return a->exec();
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QTextStream out(m_logFile.data());
	QTextStream consoleOut(stdout);

	switch (type)
	{
	case QtInfoMsg:
	{
		out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
		out << "[INFO] ";
		out << ": " << msg << endl;
		consoleOut << "[INFO] " << msg << endl;
		break;
	}
	case QtDebugMsg:
	{
		if (logLevel == 2)
		{
			out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
			out << "[DEBAG] ";
			out << ": " << msg << endl;
			consoleOut << "[DEBAG] " << msg << endl;
		}

		break;
	}
	case QtWarningMsg:
	{
		if (logLevel > 0)
		{
			out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
			out << "[WARNING] ";
			out << ": " << msg << endl;
			consoleOut << "[WARNING] " << msg << endl;
		}
		break;
	}
	case QtCriticalMsg:
	{
		out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
		out << "[CRITICAL] ";
		out << ": " << msg << endl;
		consoleOut << "[CRITICAL] " << msg << endl;
		break;
	}
	case QtFatalMsg:
	{
		out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz ");
		out << "[FATAL] ";
		out << ": " << msg << endl;
		consoleOut << "[FATAL] " << msg << endl;
		break;
	}

	out.flush();
	consoleOut.flush();
	}
}
