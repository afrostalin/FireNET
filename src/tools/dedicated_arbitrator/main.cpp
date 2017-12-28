// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QCoreApplication>
#include <QThread>
#include <QFile>
#include <QSettings>

#include "global.h"
#include "client.h"

#define SETTINGS_CFG "DedicatedArbitrator.cfg"

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
		if (settingsCFG[i] == "sv_name")
		{
			gEnv->m_Name = settings.value(settingsCFG[i], "DedicatedArbitrator").toString();
			LogInfo("Arbitrator name <%s>", gEnv->m_Name.toStdString().c_str());
		}
		else if (settingsCFG[i] == "sv_ip")
		{
			gEnv->m_ServerIP = settings.value(settingsCFG[i], "127.0.0.1").toString();
			LogInfo("FireNet ip <%s>", gEnv->m_ServerIP.toStdString().c_str());
		}
		else if (settingsCFG[i] == "sv_port")
		{
			gEnv->m_ServerPort = settings.value(settingsCFG[i], 64000).toInt();
			LogInfo("FireNet port <%d>", gEnv->m_ServerPort);
		}
		else if (settingsCFG[i] == "sv_pathToExe")
		{
			gEnv->m_PathToDedicatedGameServer = settings.value(settingsCFG[i], "").toString();
			LogInfo("Arbitrator path to game server <%s>", gEnv->m_PathToDedicatedGameServer.toStdString().c_str());
		}
		else if (settingsCFG[i] == "sv_maxGameServersCount")
		{
			gEnv->m_GameServersMaxCount = settings.value(settingsCFG[i], 10).toInt();
			LogInfo("Max game servers count in arbitrator pool <%d>", gEnv->m_GameServersMaxCount);
		}
		else if (settingsCFG[i] == "net_version")
		{
			gEnv->net_version = settings.value(settingsCFG[i], "1.0.0.0").toString();
			LogInfo("Packet version <%s>", gEnv->net_version.toStdString().c_str());
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

	ReadSettings();

	gEnv->pClient = new RemoteClient();
	gEnv->pClient->ConnectToServer(gEnv->m_ServerIP, gEnv->m_ServerPort);

    return a.exec();
}
