// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _CFireNET_H_
#define _CFireNET_H_

#include "../includes/FireNET_Base.h"

namespace FireNET
{
	class CFireNETBase : public IFireNETBase, public IGameFrameworkListener, public ISystemEventListener
	{
	public:
		CFireNETBase() {}
		~CFireNETBase() {}
	public:
		void Init();
		IFireNETBase* GetBase() const;
	public:
		// IFireNETBase
		virtual void RegisterFlowGraphNodes();
		virtual int GetUID();
		virtual SProfile* GetProfile(int uid);
#if defined(DEDICATED_SERVER)
		virtual bool UpdateProfile(SProfile* profile);
#endif

		// IGameFrameworkListener
		void OnPostUpdate(float fDeltaTime);
		void OnSaveGame(ISaveGame* pSaveGame) {};
		void OnLoadGame(ILoadGame* pLoadGame) {};
		void OnLevelEnd(const char* nextLevel) {};
		void OnActionEvent(const SActionEvent& event) {};

		// ISystemEventListener
		void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);
	};

	// Global environment for plugin
	extern CFireNETBase* gFireNET;
}

#endif