// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <queue>

#include "Network/UdpPacket.h"

class CReadQueue;

typedef boost::asio::io_service        BoostIO;
typedef boost::asio::ip::udp::socket   BoostUdpSocket;
typedef boost::asio::ip::udp::endpoint BoostUdpEndPoint;

class CUdpClient
{
public:
	CUdpClient(BoostIO& io_service, const char* ip, short port);
	~CUdpClient();
public:
	void                            Update();
	void                            SendNetMessage(CUdpPacket &packet);
public:
	void                            CloseConnection();
	bool                            IsConnected() { return bIsConnected; }
	int                             GetLastPacketNumber() { return m_LastOutPacketNumber; }
private:
	void                            Do_Connect();
	void                            Do_Read();
	void                            Do_Write();
public:
	void                            On_Connected(bool connected);
	void                            ResetTimeout() { m_ConnectionTimeout = 0.f; }
	void                            PrintClientStatistic();
private:
	void                            On_Disconnected();	
	void                            CalculateStatisctic();
public:
	enum EUdpClientStatus : int
	{
		NotConnected,
		Connecting,
		Connected,
		WaitStart,
		Playing,
	};

	void                            UpdateStatus(EUdpClientStatus newStatus);
private:
	std::queue<CUdpPacket>          m_Queue;
	EUdpClientStatus                m_Status;

	CReadQueue*                     pReadQueue;

	bool                            bIsConnected;
	bool                            bIsStopped;
private:
	BoostIO&                        m_IO_service;
	BoostUdpSocket                  m_UdpSocket;

	BoostUdpEndPoint                m_ServerEndPoint;
	BoostUdpEndPoint                m_UdpSenderEndPoint;

	char                            m_ReadBuffer[static_cast<int>(EFireNetUdpPackeMaxSize::SIZE)];
private:
	float                           m_ConnectionTimeout;  
	float                           m_LastSendedMessageTime;
	int                             m_LastOutPacketNumber;
private:
	float                           m_StatisticLastTime;
	int                             m_StatisticPacketsInputCount;
	int                             m_StatisticPacketsOutputCount;
};