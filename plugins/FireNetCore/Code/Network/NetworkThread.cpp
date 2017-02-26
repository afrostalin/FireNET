// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "NetworkThread.h"
#include "TcpClient.h"

void CNetworkThread::ThreadEntry()
{
	try
	{
		CryLog(TITLE "Init TCP client...");

		BoostIO io_service;

		// Load SSL certificate
		boost::asio::const_buffer buffer(SSL_CERTIFICATE, strlen(SSL_CERTIFICATE));
		BoostSslContex ctx(BoostSslContex::sslv23);
		ctx.add_certificate_authority(buffer);

		// Init client
		mEnv->pTcpClient = new CTcpClient(io_service, ctx);

		io_service.run();

		CryLog(TITLE "TCP client closed.");

		SAFE_DELETE(mEnv->pTcpClient);
	}
	catch (const std::exception& e)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't init TCP client : %s", e.what());
	}
}

void CNetworkThread::SignalStopWork()
{
	if (mEnv->pTcpClient)
		mEnv->pTcpClient->CloseConnection();
}
