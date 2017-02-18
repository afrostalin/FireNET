// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet-Core>

class CTcpPacket;

class CReadQueue
{
public:
	CReadQueue() {}
	~CReadQueue() {}
public:
	void ReadPacket(CTcpPacket &packet);
private:
	void ReadResult(CTcpPacket &packet , EFireNetTcpPacketResult result);
	void ReadError(CTcpPacket &packet, EFireNetTcpPacketError error);
	void ReadServerMsg(CTcpPacket &packet, EFireNetTcpPacketSMessage serverMsg);
private:
	void LoadProfile(CTcpPacket &packet);
	void LoadShop(CTcpPacket &packet);
	void LoadGameServerInfo(CTcpPacket &packet);
};