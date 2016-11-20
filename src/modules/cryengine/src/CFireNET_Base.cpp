// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include <CryFlowGraph/IFlowBaseNode.h>

#include "Global.h"
#include "CFireNET_Base.h"
#include <thread>

#ifndef SAFESTR
#define SAFESTR(x) (((const char*)x)?((const char*)x):"")
#endif

void NetworkThread()
{
	gModuleEnv->pNetwork = new CNetwork;

	if (gModuleEnv->pNetwork)
		gModuleEnv->pNetwork->Init();
	else
		return;
}

namespace FireNET
{
	void CFireNETBase::Init()
	{
		if (gEnv && gEnv->pSystem && !gEnv->pSystem->IsQuitting())
		{
			if (gEnv && gEnv->pSystem && gEnv->pSystem->GetISystemEventDispatcher())
			{
				gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error register in ISystemEvent");
			}
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error register listener");
		}
	}

	IFireNETBase *CFireNETBase::GetBase() const
	{
		return static_cast<IFireNETBase*>(const_cast<CFireNETBase*>(this));
	}

	void CFireNETBase::RegisterFlowGraphNodes()
	{
#ifndef DEDICATED_SERVER
		CryLog(TITLE "Register flow nodes...");

		if (IFlowSystem* pFlow = gEnv->pGame->GetIGameFramework()->GetIFlowSystem())
		{
			for (CAutoRegFlowNodeBase* pFactory = CAutoRegFlowNodeBase::m_pFirst; pFactory; pFactory = pFactory->m_pNext)
			{
				TFlowNodeTypeId nTypeId = pFlow->RegisterType(pFactory->m_sClassName, pFactory);

				if (nTypeId != InvalidFlowNodeTypeId)
				{
					CryLog(TITLE "Flow node class(%s) registered!", SAFESTR(pFactory->m_sClassName));
				}

				else
				{
					CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Flow node class(%s) couldn't register!", SAFESTR(pFactory->m_sClassName));
				}
			}

			return;
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error register flow nodes!");
		}
#endif
	}

	// IGameFrameworkListener 
	void CFireNETBase::OnPostUpdate(float fDeltaTime)
	{

	}

	// ISystemEventListener
	void CFireNETBase::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
	{
		switch (event)
		{

		case ESYSTEM_EVENT_GAME_POST_INIT_DONE:
		{
			// Register in IGameFramework
			if (gEnv && gEnv->pGame && gEnv->pGame->GetIGameFramework())
			{
				gEnv->pGame->GetIGameFramework()->RegisterListener(this, "FireNET", FRAMEWORKLISTENERPRIORITY_DEFAULT);
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Error register IGameFramework");
			}

			// Automatic start connecting to FireNET
			if (!gEnv->IsEditor())
			{
				std::thread networkThread(NetworkThread);
				networkThread.detach();
			}

			break;
		}

		default:
			break;
		}
	}

	// Get FireNET uid on client
	int CFireNETBase::GetUID()
	{
#ifndef DEDICATED_SERVER
		CryLog(TITLE "GetUID() executed on client");

		if (gModuleEnv->m_profile)
		{
			return gModuleEnv->m_profile->uid;
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "Failed get local profile");
			return 0;
		}
#else
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "GetUID() function can't call on dedicated server");
		return 0;
#endif
	}

	// Get pointer to profile by FireNET uid
	SProfile* CFireNETBase::GetProfile(int uid)
	{
#ifndef DEDICATED_SERVER
		CryLog(TITLE "GetProfile() executed on client");

		if (gModuleEnv->m_profile)
			return gModuleEnv->m_profile;
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "Failed get local profile");
			return nullptr;
		}
#else
		CryLog(TITLE "GetProfile() executed on server");

		if (gModuleEnv->pNetwork && gModuleEnv->bConnected)
		{
			for (int i = 0; i < gModuleEnv->m_Profiles.size(); ++i)
			{
				if (gModuleEnv->m_Profiles[i].uid == uid)
				{
					CryLog(TITLE "Profile %d found", uid);
					return &(gModuleEnv->m_Profiles[i]);
				}
			}

			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "Profile %d not found", uid);

			QString query = "<query type='get_online_profile'><data uid ='" + QString::number(uid) + "'/></query>";
			gModuleEnv->pNetwork->SendQuery(query.toStdString().c_str());

			return nullptr;
		}
		else
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't get profile because you not connected to FireNET");
			return nullptr;
		}
#endif
	}

#if defined(DEDICATED_SERVER)
	// Update profile
	bool CFireNETBase::UpdateProfile(SProfile * profile)
	{
		CryLog(TITLE "UpdateProfile()");

		QString query = "<query type='update_online_profile'>"
			"<data uid ='" + QString::number(profile->uid) +
			"' nickname ='" + profile->nickname +
			"' fileModel ='" + profile->fileModel +
			"' lvl ='" + QString::number(profile->lvl) +
			"' xp ='" + QString::number(profile->xp) +
			"' money ='" + QString::number(profile->money) +
			"' items ='" + profile->items +
			"' friends ='" + profile->friends + "'/>"
			"</query>";

		if (gModuleEnv->pNetwork)
		{
			// TODO : Use sync query
			return gModuleEnv->pNetwork->SendQuery(query.toStdString().c_str());
		}

		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't update profile because you not connected to FireNET");
		return false;
	}
#endif

}