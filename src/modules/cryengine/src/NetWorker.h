// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _Xml_Worker_H_
#define _Xml_Worker_H_

#include "../includes/FireNET_Base.h"
#include "NetPacket.h"

class CNetWorker
{
public:
	CNetWorker() {}
	~CNetWorker() {}

public:
	void ReadPacket(const char* data);
private:
	void onProfileDataRecived(NetPacket &packet);

#ifndef DEDICATED_SERVER
	void onShopItemsRecived(NetPacket &packet);
	void onGameServerDataRecived(NetPacket &packet);
#endif
};

#endif // _Xml_Worker_H_

