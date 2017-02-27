// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "NetworkThread.h"
#include "Network/UdpServer.h"

void CNetworkThread::ThreadEntry()
{
	try
	{
		CryLog(TITLE "Init UDP server...");

		ICVar* ip = gEnv->pConsole->GetCVar("firenet_game_server_ip");
		ICVar* port = gEnv->pConsole->GetCVar("firenet_game_server_port");

		BoostIO io_service;

		if (ip && port)
		{
			mEnv->pUdpServer = new CUdpServer(io_service, ip->GetString(), port->GetIVal());

			io_service.run();
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't init UDP server - Can't ger address to bind");
		}

		CryLog(TITLE "UDP server closed.");

		SAFE_DELETE(mEnv->pUdpServer);

		bIsReadyToClose = true;
	}
	catch (const std::exception& e)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't init UDP server : %s", e.what());
	}
}

void CNetworkThread::SignalStopWork()
{
	if (mEnv->pUdpServer)
		mEnv->pUdpServer->Close();
}