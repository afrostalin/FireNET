// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <CryThreading/IThreadManager.h>

class CNetworkThread : public IThread
{
public:
	CNetworkThread(){}
	~CNetworkThread() {}
public:
	virtual void ThreadEntry();
	void         SignalStopWork();
};