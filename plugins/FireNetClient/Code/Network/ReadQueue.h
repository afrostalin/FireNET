// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <FireNet-Core>

class CUdpPacket;

class CReadQueue
{
public:
	CReadQueue() {}
	~CReadQueue() {}
public:
	void ReadPacket(CUdpPacket &packet);
private:
	void ReadAsk(CUdpPacket &packet, EFireNetUdpAsk ask);
	void ReadRequest(CUdpPacket &packet, EFireNetUdpRequest request);
};