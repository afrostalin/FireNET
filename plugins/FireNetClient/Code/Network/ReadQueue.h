// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

class CUdpPacket;

class CReadQueue
{
public:
	CReadQueue() 
	{
		m_LastInputPacketNumber = 0;
		m_LocalPlayerChannelID = 0;
	}
	~CReadQueue() {}
public:
	void ReadPacket(CUdpPacket &packet);
private:
	void ReadAsk(CUdpPacket &packet, EFireNetUdpAsk ask);
	void ReadRequest(CUdpPacket &packet, EFireNetUdpRequest request);
	void ReadResult(CUdpPacket &packet, EFireNetUdpResult result);
	void ReadError(CUdpPacket &packet, EFireNetUdpError error);
private:
	int  m_LastInputPacketNumber;
	int  m_LocalPlayerChannelID;
};