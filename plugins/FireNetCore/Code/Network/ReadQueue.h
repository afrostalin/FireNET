// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

class CTcpPacket;

class CReadQueue
{
public:
	CReadQueue() {}
	~CReadQueue() {}
public:
	void ReadPacket(CTcpPacket &packet);
private:
	void ReadResult(CTcpPacket &packet , EFireNetTcpResult result);
	void ReadError(CTcpPacket &packet, EFireNetTcpError error);
	void ReadServerMsg(CTcpPacket &packet, EFireNetTcpSMessage serverMsg);
private:
	void LoadProfile(CTcpPacket &packet);
	void LoadShop(CTcpPacket &packet);
	void LoadGameServerInfo(CTcpPacket &packet);
};