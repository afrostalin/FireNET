// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet>

class CUdpPacket;

class CReadQueue
{
public:
	CReadQueue(uint32 id) : m_ClientID(id)
	{
		m_LastInputPacketNumber = 0;
		m_LastOutputPacketNumber = 0;
		m_LastPacketTime = 0.f;
	}
	~CReadQueue() {}
public:
	void   ReadPacket(CUdpPacket &packet);
	float  GetLastTime() { return m_LastPacketTime; }
private:
	void   ReadAsk(CUdpPacket &packet, EFireNetUdpAsk ask);
	void   ReadPing();
	void   ReadRequest(CUdpPacket &packet, EFireNetUdpRequest request);
private:
	void   SendPacket(CUdpPacket &packet);
private:
	uint32 m_ClientID;

	int    m_LastInputPacketNumber;
	int    m_LastOutputPacketNumber;

	float  m_LastPacketTime;
};