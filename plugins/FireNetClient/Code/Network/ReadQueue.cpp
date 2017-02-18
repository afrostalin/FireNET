// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"
#include "UdpPacket.h"

void CReadQueue::ReadPacket(CUdpPacket & packet)
{
	// Server can't send to client empty packet, it's wrong, but you can see that if it happened
	switch (packet.getType())
	{
	case EFireNetUdpPacketType::Empty :
	{
		CryLog(TITLE "Packet type = EFireNetUdpPacketType::Empty");
		break;
	}
	case EFireNetUdpPacketType::Ask :
	{
		CryLog(TITLE "Packet type = EFireNetUdpPacketType::Ask");
		ReadAsk(packet, packet.ReadAsk());
		break;
	}
	case EFireNetUdpPacketType::Request :
	{
		CryLog(TITLE "Packet type = EFireNetUdpPacketType::Request");
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

void CReadQueue::ReadRequest(CUdpPacket & packet, EFireNetUdpRequest request)
{
	switch (request)
	{
	case EFireNetUdpRequest::Spawn:
	{
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
