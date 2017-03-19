// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#ifdef _WINSOCKAPI_
#undef _WINSOCKAPI_
#endif
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <queue>
#include <map>

#include <FireNet>

#include "Network/UdpPacket.h"
#include "ReadQueue.h"

typedef boost::asio::io_service        BoostIO;
typedef boost::asio::ip::udp::socket   BoostUdpSocket;
typedef boost::asio::ip::udp::endpoint BoostUdpEndPoint;

// TODO - Create client list + player list
struct SFireNetUdpServerClient
{
	uint32                          m_ID;
	BoostUdpEndPoint                m_EndPoint;

	SFireNetProfile*                pFireNetProfile;
	CReadQueue*                     pReader;

	bool                            bConnected;
	bool                            bInGame;
	bool                            bNeedToRemove;
};

struct SFireNetUdpMessage
{
	uint32           m_ClientID;
	CUdpPacket       m_Packet;
	BoostUdpEndPoint m_EndPoint;
};

typedef std::map<uint32, SFireNetUdpServerClient> UdpClientList;
typedef UdpClientList::value_type UdpClient;

class CUdpServer
{
public:
	CUdpServer(BoostIO& io_service, const char* ip, short port);
	~CUdpServer();
public:
	void                                 Update();
	void                                 Close();
public:
	void                                 SetServerStatus(EFireNetUdpServerStatus status) 
	{
		CryLog(TITLE "Update server status from %d to %d", static_cast<int>(m_Status), static_cast<int>(status));
		m_Status = status;
	}
public:
	short                                GetClientCount() { return m_Clients.size(); }
	EFireNetUdpServerStatus              GetServerStatus() { return m_Status; }
public:
	void                                 SendToClient(CUdpPacket &packet, uint32 clientID);
	void                                 SendToAll(CUdpPacket &packet);
	void                                 SendToAllExcept(uint32 id, CUdpPacket &packet);
private:	
	uint32                               GetOrCreateClientID(BoostUdpEndPoint endpoint);
	SFireNetUdpServerClient*             GetClient(uint32 id);
	void                                 RemoveClient(uint32 id);
private:
	void                                 Do_Receive();
	void                                 Do_Send();
private:
	void                                 On_RemoteError(const boost::system::error_code error_code, const BoostUdpEndPoint endPoint);
	void                                 On_ClientDisconnect(uint32 id);
private:
	void                                 MessageProcess(const char* data, uint32 id);
private:
	std::queue<SFireNetUdpMessage>       m_PacketQueue;
private:
	BoostIO&                             m_IO_service;
	BoostUdpSocket                       m_UdpSocket;
	EFireNetUdpServerStatus              m_Status;

	BoostUdpEndPoint                     m_RemoteEndPoint;
	uint32                               m_NextClientID;

	char                                 m_ReadBuffer[static_cast<int>(EFireNetUdpPackeMaxSize::SIZE)];
private:
	UdpClientList                        m_Clients;
	float                                m_LastSendedMessageTime;
};