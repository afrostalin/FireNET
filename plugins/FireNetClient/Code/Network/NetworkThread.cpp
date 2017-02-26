// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "NetworkThread.h"
#include "Network/UdpClient.h"

void CNetworkThread::ThreadEntry()
{
	try
	{
		CryLog(TITLE "Init UDP client...");

		BoostIO io_service;

		ICVar* ip = gEnv->pConsole->GetCVar("firenet_game_server_ip");
		ICVar* port = gEnv->pConsole->GetCVar("firenet_game_server_port");

		if (ip && port)
		{
			mEnv->pUdpClient = new CUdpClient(io_service, ip->GetString(), port->GetIVal());

			io_service.run();
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't init UDP client - Can't ger game server ip/port");
		}

		CryLog(TITLE "UDP client closed.");

		SAFE_DELETE(mEnv->pUdpClient);
	}
	catch (const std::exception& e)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't init UDP client : %s", e.what());
	}	
}

void CNetworkThread::SignalStopWork()
{
	if(mEnv->pUdpClient)
		mEnv->pUdpClient->CloseConnection();
}