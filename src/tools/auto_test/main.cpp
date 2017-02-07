// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QCoreApplication>
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include <QFile>

#include "tcpclient.h"

QString m_ServerIP;
int     m_ServerPort;
int     m_ClientTickrate;

void ReadSettings()
{
	qDebug() << "Reading AutoTest.cfg ...";

	if (!QFile::exists("AutoTest.cfg"))
	{
		qDebug() << "AutoTest.cfg not found! Using default settings...";
		m_ServerIP = "127.0.0.1";
		m_ServerPort = 3322;
		m_ClientTickrate = 30;
	}

	QSettings settings(QString("AutoTest.cfg"), QSettings::IniFormat);
	QStringList AutoTestCFG = settings.allKeys();

	for (int i = 0; i < AutoTestCFG.size(); i++)
	{
		if (AutoTestCFG[i] == "sv_ip")
		{
			m_ServerIP = settings.value(AutoTestCFG[i], "127.0.0.1").toString();
			qDebug() << "Server ip =" << m_ServerIP;
		}
		else if (AutoTestCFG[i] == "sv_port")
		{
			m_ServerPort = settings.value(AutoTestCFG[i], 3322).toInt();
			qDebug() << "Server port =" << m_ServerPort;
		}
		else if (AutoTestCFG[i] == "cl_tickrate")
		{
			m_ClientTickrate = settings.value(AutoTestCFG[i], 30).toInt();
			qDebug() << "Server tickrate =" << m_ClientTickrate;
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
        tcpclient* pClient = new tcpclient(m_ServerIP, m_ServerPort);
        QObject::connect(pTimer, &QTimer::timeout, pClient, &tcpclient::update);

        pClient->CreateClient();
        pClient->CreateTestList();

        m_clients.push_back(pClient);
    }

	int tickrate = 1000 / m_ClientTickrate;
    pTimer->start(tickrate);

    return a.exec();
}
