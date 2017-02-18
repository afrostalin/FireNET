// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "NetworkThread.h"
#include "Network/UdpClient.h"

using namespace boost::system;
using namespace boost::asio;

void CNetworkThread::ThreadEntry()
{
	try
	{
		CryLog(TITLE "Init UDP client...");

		io_service io_service;

		mEnv->pUdpClient = new CUdpClient(io_service);

		io_service.run();

		CryLog(TITLE "UDP client closed.");
	}
	catch (const std::exception& e)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't init UDP client : %s", e.what());
	}	
}

void CNetworkThread::SignalStopWork()
{
	if (mEnv->pUdpClient)
		mEnv->pUdpClient->CloseConnection();

	SAFE_DELETE(mEnv->pUdpClient);
}