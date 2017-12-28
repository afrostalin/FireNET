// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/ssl.hpp>
#include <queue>

#include "TcpPacket.h"

#define MAX_TCP_PACKET_SIZE 512

class CReadQueue;

typedef boost::asio::io_service        BoostIO;
typedef boost::asio::ip::tcp::socket   BoostTcpSocket;
typedef boost::asio::ip::tcp::endpoint BoostTcpEndPoint;
typedef boost::asio::ip::tcp::resolver BoostTcpResolver;
typedef boost::asio::ssl::context      BoostSslContex;
typedef boost::asio::deadline_timer    BoostTimer;

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
	CTcpClient(BoostIO& io_service, BoostSslContex& context);
	~CTcpClient();
public:
	void                    CloseConnection();
public:
	bool                    IsConnected() { return bIsConnected; };
public:
	void                    SendQuery(CTcpPacket &packet);
private:
	bool                    Do_VerifyCertificate(bool preverified, boost::asio::ssl::verify_context& ctx);
	void                    Do_Connect();
	void                    Do_Handshake();
	void                    Do_Read();
	void                    Do_Write();
private:
	void                    On_Disconnected();             
private:
	void                    TimeOutCheck();
	void                    AnswerTimeout();
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
	bool                    bIsClosing;

	char                    m_ReadBuffer[MAX_TCP_PACKET_SIZE];
private:
	BoostIO&                                  m_IO_service;
	boost::asio::ssl::stream<BoostTcpSocket>  m_SslSocket;
	BoostTimer                                m_Timer;
	BoostTimer                                m_AnswerTimer;
};