// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#pragma once

namespace FireNET
{
	struct SProfile
	{
		int uid;
		string nickname;
		string fileModel;
		int lvl;
		int xp;
		int money;
		string items;
		string friends;
	};

	struct IFireNETBase
	{
		// Register flow graph nodes from plugin
		virtual void RegisterFlowGraphNodes() = 0;

		// Get FireNET uid (Only client)
		virtual int GetUID() = 0;

		// Get FireNET profile by uid (Server and Client)
		virtual SProfile* GetProfile(int uid) = 0;
#if defined(DEDICATED_SERVER)
		// Update FireNET profile (Only server)
		virtual bool UpdateProfile(SProfile* profile) = 0;
#endif
	};
};

// Global FireNET environment for GameDll only
#if !defined(FIRENET_MODULE)
extern FireNET::IFireNETBase* gFireNET;
#endif