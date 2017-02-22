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

enum class EUdpClientStatus : int
{
	NotConnected,
	Connecting,
	Connected,
};

class CReadQueue;

class CUdpClient
{
public:
	CUdpClient(boost::asio::io_service& io_service, const char* ip, short port);
	~CUdpClient();
public:
	void                            Update(float fDeltaTime);
	void                            SendNetMessage(CUdpPacket &packet);
public:
	void                            CloseConnection();
	bool                            IsConnected() { return bIsConnected; }
private:
	void                            Do_Connect(); // Virtual connecting (by Q/A)
	void                            Do_Read();
	void                            Do_Write();
public:
	void                            On_Connected(bool connected, EFireNetUdpServerError reason = EFireNetUdpServerError::NONE);
private: 
	void                            On_Disconnected();
private:
	std::queue<CUdpPacket>          m_Queue;
	EUdpClientStatus                m_Status;	

	CReadQueue*                     pReadQueue;

	bool                            bIsConnected;
private:
	boost::asio::io_service&        m_IO_service; 
	boost::asio::ip::udp::socket    m_UdpSocket;

	boost::asio::ip::udp::endpoint  m_ServerEndPoint;
	boost::asio::ip::udp::endpoint  m_UdpSenderEndPoint;

	char                            m_ReadBuffer[static_cast<int>(EFireNetUdpPackeMaxSize::SIZE)];
};