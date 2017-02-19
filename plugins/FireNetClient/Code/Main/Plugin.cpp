// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "Plugin.h"

#include "Network/UdpClient.h"
#include "Network/NetworkThread.h"
#include "Network/SyncGameState.h"

#include <CryCore/Platform/platform_impl.inl>

IEntityRegistrator *IEntityRegistrator::g_pFirst = nullptr;
IEntityRegistrator *IEntityRegistrator::g_pLast = nullptr;

void TestConnection(IConsoleCmdArgs* argc)
{
	if (mEnv->pNetworkThread)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "FireNet client thread alredy spawned!");
		return;
	}

	if (!gEnv->IsDedicated() && !gEnv->IsEditor())
	{
		mEnv->pNetworkThread = new CNetworkThread();

		if (!gEnv->pThreadManager->SpawnThread(mEnv->pNetworkThread, "FireNetClient_Thread"))
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't spawn FireNet client thread!");

			SAFE_DELETE(mEnv->pNetworkThread);
		}
		else
			CryLog(TITLE "FireNet client thread spawned");
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't spawn FireNet client thread : It's not dedicated server or it's editor");
	}
}

CFireNetClientPlugin::~CFireNetClientPlugin()
{
	if (gEnv->pSystem)
		gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	if (gEnv->pGameFramework)
		gEnv->pGameFramework->UnregisterListener(this);

	IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
	while (pTemp != nullptr)
	{
		pTemp->Unregister();
		pTemp = pTemp->m_pNext;
	}

	// Stop and delete network thread
	if (mEnv->pNetworkThread)
		mEnv->pNetworkThread->SignalStopWork();

	SAFE_DELETE(mEnv->pNetworkThread);

	// Clear FireNet client pointer
	gEnv->pFireNetClient = nullptr;
}

bool CFireNetClientPlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	// Init FireNet client pointer
	gEnv->pFireNetClient = this;

	if (initParams.bEditor)
		gEnv->SetIsEditor(true);

	if (initParams.bDedicatedServer)
		gEnv->SetIsDedicated(true);

	return true;
}

void CFireNetClientPlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_GAME_POST_INIT:
	{
		// Register entities
		IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
		while (pTemp != nullptr)
		{
			pTemp->Register();
			pTemp = pTemp->m_pNext;
		}

		// Register CVars
		mEnv->net_ip = REGISTER_STRING("firenet_game_server_ip", "127.0.0.1", VF_NULL, "Sets the FireNet game server ip address");
		REGISTER_CVAR2("firenet_game_server_port", &mEnv->net_port, 64000, VF_CHEAT, "FireNet game server port");
		REGISTER_CVAR2("firenet_game_server_timeout", &mEnv->net_timeout, 10.0f, VF_NULL, "FireNet game server timeout");

		// Test functionality
		REGISTER_COMMAND("net_connect", TestConnection, VF_NULL, "");

		break;
	}
	case ESYSTEM_EVENT_EDITOR_GAME_MODE_CHANGED:
	{
		if (mEnv->pGameSync && wparam == 0)
			mEnv->pGameSync->Reset();
		break;
	}
	case ESYSTEM_EVENT_GAME_FRAMEWORK_INIT_DONE:
	{
		if (gEnv->pGameFramework)
		{
			gEnv->pGameFramework->RegisterListener(this, "FireNetClient_GameFrameworkListener", FRAMEWORKLISTENERPRIORITY_DEFAULT);
			CryLog(TITLE "Framework listener registered");
		}
		else
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't register framework listener!");

		break;
	}
	break;
	}
}

void CFireNetClientPlugin::OnPostUpdate(float fDeltaTime)
{
	if (mEnv->pNetworkThread && mEnv->pUdpClient)
	{
		mEnv->pUdpClient->Update(fDeltaTime);
	}
}

void CFireNetClientPlugin::ConnectToServer(const char * ip, int port)
{
	if (mEnv->pNetworkThread)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "FireNet client thread alredy spawned!");
		return;
	}

	if (!gEnv->IsDedicated() && !gEnv->IsEditor())
	{
		mEnv->pNetworkThread = new CNetworkThread();

		if (!gEnv->pThreadManager->SpawnThread(mEnv->pNetworkThread, "FireNetClient_Thread"))
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't spawn FireNet client thread!");

			SAFE_DELETE(mEnv->pNetworkThread);
		}
		else
			CryLog(TITLE "FireNet client thread spawned");
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't spawn FireNet client thread : It's not dedicated server or it's editor");
	}
}

void CFireNetClientPlugin::DisconnectFromServer()
{
	if (mEnv->pNetworkThread && mEnv->pUdpClient)
	{
		mEnv->pNetworkThread->SignalStopWork();
	}
}

void CFireNetClientPlugin::SendMovementRequest(EFireNetClientActions action, float value)
{
	if (mEnv->pUdpClient && mEnv->pUdpClient->IsConnected())
	{
		CUdpPacket packet(mEnv->m_LastOutPacketNumber, EFireNetUdpPacketType::Request);
		packet.WriteRequest(EFireNetUdpRequest::Action);
		packet.WriteInt(action);
		packet.WriteFloat(value);

		mEnv->pUdpClient->SendNetMessage(packet);
	}
}

CRYREGISTER_SINGLETON_CLASS(CFireNetClientPlugin)