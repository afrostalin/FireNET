// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"

#include "Network/UdpServer.h"
#include "Network/UdpPacket.h"

#include <CryEntitySystem/IEntitySystem.h>
#include <CryGame/IGameFramework.h>

void CReadQueue::ReadPacket(CUdpPacket & packet)
{
	m_LastPacketTime = gEnv->pTimer->GetAsyncCurTime();

	//! Check packet number
	if (packet.getPacketNumber() < m_LastInputPacketNumber)
	{
		CryLog(TITLE "Packet from client can't be readed, because packet too old. Packet number : %d, last number : %d", packet.getPacketNumber(), m_LastInputPacketNumber);
		return;
	}
	else if (packet.getPacketNumber() >= m_LastInputPacketNumber)
	{
		m_LastInputPacketNumber = packet.getPacketNumber();
	}

	//! Server can't send to client empty packet, it's wrong, but you can see that if it happened
	switch (packet.getType())
	{
	case EFireNetUdpPacketType::Empty :
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Packet type = EFireNetUdpPacketType::Empty");
		break;
	}
	case EFireNetUdpPacketType::Ping :
	{
		ReadPing();
		break;
	}
	case EFireNetUdpPacketType::Ask :
	{
		ReadAsk(packet, packet.ReadAsk());
		break;
	}
	case EFireNetUdpPacketType::Request :
	{
		ReadRequest(packet, packet.ReadRequest());
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadAsk(CUdpPacket & packet, EFireNetUdpAsk ask)
{
	switch (ask)
	{
	case EFireNetUdpAsk::ConnectToServer:
	{
		break;
	}
	case EFireNetUdpAsk::ChangeTeam:
	{
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadPing()
{
	CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Ping);
	SendPacket(packet);
}

void CReadQueue::ReadRequest(CUdpPacket & packet, EFireNetUdpRequest request)
{
	switch (request)
	{
	case EFireNetUdpRequest::Spawn:
	{
		CryLog(TITLE "Client %d request spawn", m_ClientID);

		//! Spawn packet
		CUdpPacket packet(m_LastOutputPacketNumber, EFireNetUdpPacketType::Request);
		packet.WriteRequest(EFireNetUdpRequest::Spawn);
		packet.WriteInt(1000001);           //! FireNet uid
		packet.WriteInt(2);                 //! Chanel id (always > 1)
		packet.WriteFloat(130.562943f);           //! Spawn position x
		packet.WriteFloat(143.508270f);           //! Spawn position y
		packet.WriteFloat(32.171688f);           //! Spawn position x
		packet.WriteFloat(1);           //! Spawn rotatin w
		packet.WriteFloat(0);         //! Spawn rotatin vector x
		packet.WriteFloat(0);         //! Spawn rotatin vector y
		packet.WriteFloat(0);         //! Spawn rotatin vector z
		packet.WriteString("player_model"); //! Player model
		packet.WriteString("nickname");     //! Player nickname

		SendPacket(packet);

		break;
	}
	case EFireNetUdpRequest::Movement:
	{
		break;
	}
	case EFireNetUdpRequest::Action:
	{
		break;
	}
	default:
		break;
	}
}

void CReadQueue::SendPacket(CUdpPacket & packet)
{
	mEnv->pUdpServer->SendToClient(packet, m_ClientID);
	m_LastOutputPacketNumber++;
}
