// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QCoreApplication>
#include <QThread>
#include <QFile>
#include <QSettings>

#include "global.h"
#include "client.h"
#include "inputlistener.h"

#define SETTINGS_CFG "RemoteAdmin.cfg"

void ReadSettings()
{ 
	LogInfo("Reading %s ...", SETTINGS_CFG);

	if (!QFile::exists(SETTINGS_CFG))
	{
		LogWarning("%s not found! Using default settings...", SETTINGS_CFG);
	}

	QSettings settings(QString(SETTINGS_CFG), QSettings::IniFormat);
	QStringList settingsCFG = settings.allKeys();

	for (int i = 0; i < settingsCFG.size(); i++)
	{
		if (settingsCFG[i] == "sv_ip")
		{
			gEnv->m_ServerIP = settings.value(settingsCFG[i], "127.0.0.1").toString();
			LogInfo("Server ip <%s>", gEnv->m_ServerIP.toStdString().c_str());
		}
		else if (settingsCFG[i] == "sv_port")
		{
			gEnv->m_ServerPort = settings.value(settingsCFG[i], 64000).toInt();
			LogInfo("Server port <%d>", gEnv->m_ServerPort);
		}
		else if (settingsCFG[i] == "net_version")
		{
			gEnv->net_version = settings.value(settingsCFG[i], "1.0.0.0").toString();
			LogInfo("Server net version <%s>", gEnv->net_version.toStdString().c_str());
		}
		else if (settingsCFG[i] == "net_packet_debug")
		{
			gEnv->bUsePacketDebug = settings.value(settingsCFG[i], false).toBool();
			LogInfo("Use network packet debug <%s>", gEnv->bUsePacketDebug ? "true" : "false");
		}
	}
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	qDebug() << "***************************************************";
    qDebug() << "***   Remote administration panel for FireNET   ***";
    qDebug() << "*** Use 'list' command to get full command list ***";
	qDebug() << "***************************************************";

	ReadSettings();

    QThread* pListenerThread = new QThread();
    gEnv->pListener = new InputListener();
    gEnv->pListener->moveToThread(pListenerThread);
    QObject::connect(pListenerThread, &QThread::started, gEnv->pListener, &InputListener::Run);
    QObject::connect(pListenerThread, &QThread::finished, gEnv->pListener, &InputListener::deleteLater);
    pListenerThread->start();

    return a.exec();
}
