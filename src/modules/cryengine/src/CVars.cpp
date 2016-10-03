// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "StdAfx.h"
#include "CVars.h"
#include "Global.h"
#include <thread>

void _NetworkThread()
{
	gCryModule->pNetwork = new CNetwork;

	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->Init();
	else
		return;
}

void ConnectToFireNET(IConsoleCmdArgs* pArgs)
{
	if (gCryModule->pNetwork != nullptr)
	{
		if (gCryModule->pNetwork->isInit())
			gCryModule->pNetwork->ConnectToServer();
		else
			gEnv->pLog->LogError(TITLE "Can't connect to server, because network thread not init!");
	}
	else
	{
		// First init network thread
		std::thread networkThread(_NetworkThread);
		networkThread.detach();
	}
}

void DisconnectFromFireNET(IConsoleCmdArgs* pArgs)
{
	if (gCryModule->pNetwork != nullptr && gCryModule->bConnected)
	{
		gCryModule->pNetwork->CloseConnection();
	}
	else
		gEnv->pLog->LogError(TITLE "Can't diconnect to server, because you not connected!");
}

void RegisterGameServer(IConsoleCmdArgs* pArgs)
{
	gEnv->pLog->LogWarning(TITLE "Register game server in FireNET...");

	// Get server ip adress 
	QString ip = gEnv->pConsole->GetCVar("sv_bind")->GetString();
	// Get server port
	int port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
	// Get server name 
	QString serverName = gEnv->pConsole->GetCVar("sv_servername")->GetString();
	// Get current players count 
	int online = 0;
	// Get max players
	int maxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers")->GetIVal();
	// Get map name 
	QString mapName = gEnv->pConsole->GetCVar("sv_map")->GetString();
	// Get gamerules
	QString gamerules = (char*)gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

	QString query = "<query type='register_game_server'><data name = '" + serverName +
		"' ip = '" + ip +
		"' port = '" + QString::number(port) +
		"' map = '" + mapName +
		"' gamerules = '" + gamerules +
		"' online = '" + QString::number(online) +
		"' maxPlayers = '" + QString::number(maxPlayers) + "'/></query>";

	if (gCryModule->pNetwork != nullptr)
		gCryModule->pNetwork->SendQuery(query.toStdString().c_str());
	else
		gEnv->pLog->LogError(TITLE "Can't register game server because you not connected to FireNET");
}


bool CModuleCVars::RegisterCVars()
{
	gEnv->pLog->Log(TITLE "Register CVars");

	if(gEnv->pConsole)
	{
		gEnv->pConsole->RegisterString("firenet_ip", gCryModule->online_ms_address, VF_NULL, "FireNET ip address");
		gEnv->pConsole->RegisterInt("firenet_port", gCryModule->online_ms_port, VF_NULL, "FireNET port");
		return true;
	}

	gEnv->pLog->LogError(TITLE "Register CVars failed!");
	return false;
}


bool CModuleCVars::RegisterCComands()
{
	gEnv->pLog->Log(TITLE "Register console commands");

	if(gEnv->pConsole)
	{
		gEnv->pConsole->AddCommand("firenet_connect", ConnectToFireNET, VF_NULL, "Connect to FireNET");
		gEnv->pConsole->AddCommand("firenet_disconnect",DisconnectFromFireNET, VF_NULL, "Disconnect from FireNET");
		gEnv->pConsole->AddCommand("firenet_reg_gs", RegisterGameServer, VF_NULL, "Register game server in FireNET");
		return true;
	}

	gEnv->pLog->LogError(TITLE "Register console commands failed!");
	return false;
}