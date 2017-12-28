// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QCoreApplication>
#include <QSettings>
#include <QTimer>
#include <QFile>

#include "global.h"
#include "tcpclient.h"

void ReadSettings()
{
	qDebug() << "Reading AutoTest.cfg ...";

	if (!QFile::exists("AutoTest.cfg"))
	{
		qDebug() << "AutoTest.cfg not found! Using default settings...";
	}

	QSettings settings(QString("AutoTest.cfg"), QSettings::IniFormat);
	QStringList AutoTestCFG = settings.allKeys();

	for (int i = 0; i < AutoTestCFG.size(); i++)
	{
		if (AutoTestCFG[i] == "sv_ip")
		{
			gEnv->m_ServerIP = settings.value(AutoTestCFG[i], "127.0.0.1").toString();
			qDebug() << "Server ip =" << gEnv->m_ServerIP;
		}
		else if (AutoTestCFG[i] == "sv_port")
		{
			gEnv->m_ServerPort = settings.value(AutoTestCFG[i], 3322).toInt();
			qDebug() << "Server port =" << gEnv->m_ServerPort;
		}
		else if (AutoTestCFG[i] == "cl_tickrate")
		{
			gEnv->m_ClientTickrate = settings.value(AutoTestCFG[i], 30).toInt();
			qDebug() << "Server tickrate =" << gEnv->m_ClientTickrate;
		}
		else if (AutoTestCFG[i] == "net_version")
		{
			gEnv->net_version = settings.value(AutoTestCFG[i], "1.0.0.0").toString();
			qDebug() << "Server net version =" << gEnv->net_version.toStdString().c_str();
		}
	}
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTimer* pTimer = new QTimer();

    qDebug() << "Starting auto test...";

	ReadSettings();

    QVector<tcpclient*> m_clients;

    for(int i = 0; i < 10; i++)
    {
        tcpclient* pClient = new tcpclient(gEnv->m_ServerIP, gEnv->m_ServerPort);
        QObject::connect(pTimer, &QTimer::timeout, pClient, &tcpclient::update);

        pClient->CreateClient();
        pClient->CreateTestList();

        m_clients.push_back(pClient);
    }

	int tickrate = 1000 / gEnv->m_ClientTickrate;
    pTimer->start(tickrate);

    return a.exec();
}
