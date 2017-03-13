// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "Plugin.h"

#include "Network/TcpClient.h"
#include "Network/NetworkThread.h"
#include "Network/TcpPacket.h"

#include <CryExtension/ICryPluginManager.h>
#include <CryCore/Platform/platform_impl.inl>
#include <CryExtension/ICryPluginManager.h>
#include <CryThreading/IThreadConfigManager.h>

#include <FireNet.inl>

USE_CRYPLUGIN_FLOWNODES

IEntityRegistrator *IEntityRegistrator::g_pFirst = nullptr;
IEntityRegistrator *IEntityRegistrator::g_pLast = nullptr;

void CmdConnect(IConsoleCmdArgs* args)
{
	if (gFireNet)
		gFireNet->pCore->ConnectToMasterServer();
}

CFireNetCorePlugin::~CFireNetCorePlugin()
{
	// Unregister entities
	IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
	while (pTemp != nullptr)
	{
		pTemp->Unregister();
		pTemp = pTemp->m_pNext;
	}

	// Unregister CVars
	IConsole* pConsole = gEnv->pConsole;
	if (pConsole)
	{
		pConsole->UnregisterVariable("firenet_master_ip");
		pConsole->UnregisterVariable("firenet_master_port");
		pConsole->UnregisterVariable("firenet_master_remote_port");
		pConsole->UnregisterVariable("firenet_master_timeout");

		pConsole->UnregisterVariable("firenet_game_server_ip");
		pConsole->UnregisterVariable("firenet_game_server_port");
		pConsole->UnregisterVariable("firenet_game_server_timeout");
		pConsole->UnregisterVariable("firenet_game_server_map");
		pConsole->UnregisterVariable("firenet_game_server_gamerules");
		pConsole->UnregisterVariable("firenet_game_server_max_players");

#ifndef NDEBUG
		pConsole->UnregisterVariable("firenet_packet_debug");
#endif
	}

	// Close network thread
	if (mEnv->pNetworkThread && mEnv->pTcpClient)
	{
		if (!gEnv->IsDedicated())
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Network thread not deleted! Use Quit function to normal shutdown plugin!");

		mEnv->pNetworkThread->SignalStopWork();

		if (gEnv->pThreadManager)
			gEnv->pThreadManager->JoinThread(mEnv->pNetworkThread, eJM_Join);
	}

	// Unregister listeners
	if (gEnv->pSystem)
		gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	SAFE_DELETE(mEnv->pNetworkThread);
	SAFE_DELETE(gFireNet);

	CryLogAlways(TITLE "Unloaded.");
}

bool CFireNetCorePlugin::Initialize(SSystemGlobalEnvironment& env, const SSystemInitParams& initParams)
{
	if (initParams.bEditor && !gEnv->IsEditor())
		gEnv->SetIsEditor(true);

	if (initParams.bDedicatedServer && !gEnv->IsDedicated())
		gEnv->SetIsDedicated(true);

	gEnv->pSystem->GetISystemEventDispatcher()->RegisterListener(this);

	// Init FireNet core pointer
	gFireNet = new IFireNetEnv();
	gFireNet->pCore = dynamic_cast<IFireNetCore*>(this);

	// Load FireNet thread config
	gEnv->pThreadManager->GetThreadConfigManager()->LoadConfig("config/firenet.thread_config");

	ICryPlugin::SetUpdateFlags(EUpdateType_Update);
	
/*	if (!gEnv->IsDedicated())
		gEnv->pSystem->GetIPluginManager()->LoadPluginFromDisk(ICryPluginManager::EPluginType::EPluginType_CPP, "FireNet-Client", "FireNetClient_Plugin");
	else if(gEnv->IsDedicated())
		gEnv->pSystem->GetIPluginManager()->LoadPluginFromDisk(ICryPluginManager::EPluginType::EPluginType_CPP, "FireNet-Server", "FireNetServer_Plugin");*/

	return true;
}

void CFireNetCorePlugin::OnPluginUpdate(EPluginUpdateType updateType)
{
	switch (updateType)
	{
	case IPluginUpdateListener::EUpdateType_Update:
	{
		//! Automatic deleting network thread if it's ready to close
		if (mEnv->pNetworkThread && mEnv->pNetworkThread->IsReadyToClose())
		{
			SAFE_DELETE(mEnv->pNetworkThread);
		}
		break;
	}
	default:
		break;
	}
}

void CFireNetCorePlugin::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
	switch (event)
	{
	case ESYSTEM_EVENT_GAME_POST_INIT:
	{
		//! Register entities
		IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
		while (pTemp != nullptr)
		{
			pTemp->Register();
			pTemp = pTemp->m_pNext;
		}

		//! CVars - Master server
		mEnv->net_master_ip = REGISTER_STRING("firenet_master_ip", "127.0.0.1", VF_NULL, "Sets the FireNet master server ip address");
		REGISTER_CVAR2("firenet_master_port", &mEnv->net_master_port, 3322, VF_CHEAT, "FireNet master server port");
		REGISTER_CVAR2("firenet_master_timeout", &mEnv->net_master_timeout, 10, VF_NULL, "FireNet master server timeout");
		REGISTER_CVAR2("firenet_master_remote_port", &mEnv->net_master_remote_port, 5200, VF_CHEAT, "FireNet master server port for game server");

		//! CVars - Game server
		mEnv->net_game_server_ip = REGISTER_STRING("firenet_game_server_ip", "127.0.0.1", VF_NULL, "Sets the FireNet game server ip address");
		mEnv->net_game_server_map = REGISTER_STRING("firenet_game_server_map", "", VF_NULL, "Map name for loading and register in master server");
		mEnv->net_game_server_gamerules = REGISTER_STRING("firenet_game_server_gamerules", "TDM", VF_NULL, "Gamerules name for loading and register in master server");
		REGISTER_CVAR2("firenet_game_server_port", &mEnv->net_game_server_port, 64000, VF_CHEAT, "FireNet game server port");
		REGISTER_CVAR2("firenet_game_server_timeout", &mEnv->net_game_server_timeout, 10, VF_NULL, "FireNet game server timeout");
		REGISTER_CVAR2("firenet_game_server_max_players", &mEnv->net_game_server_max_players, 64, VF_NULL, "FireNet game server max players count");

		//! CVars - Other
#ifndef  NDEBUG
		REGISTER_CVAR2("firenet_packet_debug", &mEnv->net_debug, 0, VF_NULL, "FireNet packet debugging");
		REGISTER_COMMAND("firenet_master_connect", CmdConnect, VF_NULL, "Connect to FireNet master server");
#endif
		break;
	}
	case ESYSTEM_EVENT_GAME_FRAMEWORK_INIT_DONE:
	{
		//! Load FireNet settings
		gEnv->pConsole->ExecuteString("exec firenet.cfg");

		//! Automatic connect to master server
		if (gFireNet && gFireNet->pCore && !gEnv->IsEditor())
			gFireNet->pCore->ConnectToMasterServer();

		break;
	}
	case ESYSTEM_EVENT_FULL_SHUTDOWN:
	{
		Quit();
		break;
	}
	case ESYSTEM_EVENT_FAST_SHUTDOWN:
	{
		Quit();
		break;
	}
	default:
		break;
	}
}

void CFireNetCorePlugin::ConnectToMasterServer()
{
	if (mEnv->pNetworkThread)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "FireNet core thread alredy spawned. Removing...");
		
		mEnv->pNetworkThread->SignalStopWork();
		gEnv->pThreadManager->JoinThread(mEnv->pNetworkThread, eJM_Join);

		SAFE_DELETE(mEnv->pNetworkThread);
	}

	FireNet::SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_START_CONNECTION);

	mEnv->pNetworkThread = new CNetworkThread();
	if (!gEnv->pThreadManager->SpawnThread(mEnv->pNetworkThread, "FireNetCore_Thread"))
	{
		SFireNetEventArgs args;
		args.AddInt(0);
		args.AddString("cant_spawn_network_thread");

		FireNet::SendFireNetEvent(FIRENET_EVENT_MASTER_SERVER_CONNECTION_ERROR, args);

		SAFE_DELETE(mEnv->pNetworkThread);
	
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't spawn FireNet core thread!");
	}
	else
		CryLog(TITLE "FireNet core thread spawned");
}

void CFireNetCorePlugin::Authorization(const std::string & login, const std::string & password)
{
	CryLog(TITLE "Try authorizate by login and password");

	if (!login.empty() && !password.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::Login);
		packet.WriteString(login.c_str());
		packet.WriteString(password.c_str());

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't authorizate. Login or password empty");
	}	
}

void CFireNetCorePlugin::Registration(const std::string & login, const std::string & password)
{
	CryLog(TITLE "Try register new account by login and password");

	if (!login.empty() && !password.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::Register);
		packet.WriteString(login.c_str());
		packet.WriteString(password.c_str());

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't register new account. Login or password empty");
	}
}

void CFireNetCorePlugin::CreateProfile(const std::string & nickname, const std::string & character)
{
	CryLog(TITLE "Try create profile by nickname and character model");

	if (!nickname.empty() && !character.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::CreateProfile);
		packet.WriteString(nickname.c_str());
		packet.WriteString(character.c_str());

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't create new profile. Nickname or character model empty");
	}
}

void CFireNetCorePlugin::GetProfile(int uid)
{
	CryLog(TITLE "Try get profile");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::GetProfile);

	FireNet::SendPacket(packet);
}

SFireNetProfile * CFireNetCorePlugin::GetLocalProfile()
{
	// TODO
	return nullptr;
}

void CFireNetCorePlugin::GetShop()
{
	CryLog(TITLE "Try get shop");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::GetShop);

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::BuyItem(const std::string & item)
{
	CryLog(TITLE "Try buy item by name");

	if (!item.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::BuyItem);
		packet.WriteString(item.c_str());

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't buy item. Item name empty");
	}
}

void CFireNetCorePlugin::RemoveItem(const std::string & item)
{
	CryLog(TITLE "Try remove item by name");

	if (!item.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::RemoveItem);
		packet.WriteString(item.c_str());

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't remove item. Item name empty");
	}
}

void CFireNetCorePlugin::SendInvite(EFireNetInviteType type, int uid)
{
	CryLog(TITLE "Try send invite");

	if (uid > 0)
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::SendInvite);
		packet.WriteInt(type);
		packet.WriteInt(uid);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't send invite. Wrong uid");
	}
}

void CFireNetCorePlugin::DeclineInvite()
{
	CryLog(TITLE "Try decline invite");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::DeclineInvite);

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::AcceptInvite()
{
	CryLog(TITLE "Try accept invite");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::AcceptInvite);

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::RemoveFriend(int uid)
{
	CryLog(TITLE "Try remove friend");

	if (uid > 0)
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::RemoveFriend);
		packet.WriteInt(uid);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't remove friend. Wrong uid");
	}
}

void CFireNetCorePlugin::SendChatMessage(EFireNetChatMsgType type, int uid)
{
	CryLog(TITLE "Try send chat message");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::SendChatMsg);
	packet.WriteInt(type);

	if (type == EMsg_PrivateChat)
	{
		packet.WriteInt(uid);
	}

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::GetGameServer(const std::string & map, const std::string & gamerules)
{
	CryLog(TITLE "Try get game server");

	if (!map.empty() && !gamerules.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::GetServer);
		packet.WriteString(map.c_str());
		packet.WriteString(gamerules.c_str());

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't get game server. Map or gamerules empty");
	}
}

void CFireNetCorePlugin::SendRawRequestToMasterServer(CTcpPacket &packet)
{
	FireNet::SendPacket(packet);
}

bool CFireNetCorePlugin::IsConnected()
{
	return mEnv->pTcpClient ? mEnv->pTcpClient->IsConnected() : false;
}

void CFireNetCorePlugin::RegisterFireNetListener(IFireNetListener * listener)
{
	if (listener == nullptr)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Failed register FireNetEventListener. Null pointer");
		return;
	}

	for (const auto &it : mEnv->m_Listeners)
	{
		if (it == listener)
		{
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Failed register FireNetEventListener. Listener alredy registered");
			return;
		}
	}

	CryLog(TITLE "FireNet event listener registered (%p)", listener);

	mEnv->m_Listeners.push_back(listener);
}

void CFireNetCorePlugin::SendFireNetEvent(EFireNetEvents event, SFireNetEventArgs& args)
{
	FireNet::SendFireNetEvent(event, args);
}

bool CFireNetCorePlugin::Quit()
{
	CryLogAlways(TITLE "Closing plugin...");

	// Close FireNet-Core thread
	if (mEnv->pNetworkThread)
	{
		mEnv->pNetworkThread->SignalStopWork();
		gEnv->pThreadManager->JoinThread(mEnv->pNetworkThread, eJM_Join);
	}

	SAFE_DELETE(mEnv->pNetworkThread);

	if (!mEnv->pNetworkThread)
	{
		CryLogAlways(TITLE "Plugin ready to unload");
		return true;
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't normaly close plugin - network thread not deleted!");

	return false;
}

CRYREGISTER_SINGLETON_CLASS(CFireNetCorePlugin)