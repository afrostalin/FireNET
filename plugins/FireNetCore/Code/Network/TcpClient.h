// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>
#include <queue>

#include "TcpPacket.h"

class CReadQueue;

enum class ETcpPacketMaxSize : int { SIZE = 512};

enum class ETcpClientStatus : int
{
	NotConnected,
	Connecting,
	Connected,
};

enum class ETcpMessageStatus : int
{
	None,
	Sending,
	Sended,
	Wait,
	Recieved,
	Error,	
};

class CTcpClient
{
public:
	CTcpClient(boost::asio::io_service& io_service, boost::asio::ssl::context& context);
	~CTcpClient();
public:
	void                    CloseConnection();
public:
	void                    Update(float fDeltaTime);	
	bool                    IsConnected() { return bIsConnected; };
public:
	void                    SendQuery(CTcpPacket &packet);
	void                    SendSyncQuery(CTcpPacket &packet, float timeout);
private:
	bool                    Do_VerifyCertificate(bool preverified, boost::asio::ssl::verify_context& ctx);
	void                    Do_Connect();
	void                    Do_Handshake();
	void                    Do_Read();
	void                    Do_Write();
private:
	void                    On_Disconnected();
private:
	void                    AddToSendQueue(CTcpPacket &packet);
private:
	ETcpClientStatus        m_Status;
	ETcpMessageStatus       m_MessageStatus;
	std::queue <CTcpPacket> m_Queue;

	CReadQueue*             pReadQueue;

	float                   fConTimeout;
	float                   fMsgTimeout;
	float                   fMsgEndTime;

	bool                    bIsConnected;

	char                    m_ReadBuffer[static_cast<int>(ETcpPacketMaxSize::SIZE)];
private:
	boost::asio::io_service&                                m_IO_service;
	boost::asio::ssl::stream<boost::asio::ip::tcp::socket>  m_SslSocket;
};