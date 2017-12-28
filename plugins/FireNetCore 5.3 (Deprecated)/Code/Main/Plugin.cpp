// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "Plugin.h"

#include "Network/TcpClient.h"
#include "Network/NetworkThread.h"
#include "Network/TcpPacket.h"

#include <CryExtension/ICryPluginManager.h>
#include <CryCore/Platform/platform_impl.inl>
#include <CryThreading/IThreadConfigManager.h>

#include <FireNet.inl>

USE_CRYPLUGIN_FLOWNODES

IEntityRegistrator *IEntityRegistrator::g_pFirst = nullptr;
IEntityRegistrator *IEntityRegistrator::g_pLast = nullptr;

void CmdConnect(IConsoleCmdArgs* args)
{
	if (gFireNet)
	{
		gFireNet->pCore->ConnectToMasterServer();
	}
}

void CFireNetCorePlugin::RegisterCVars()
{
	//! CVars - Master server
	mEnv->net_master_ip = REGISTER_STRING("net_firenetMasterIP", "127.0.0.1", VF_NULL, "Sets the FireNet master server ip address");
	mEnv->net_version = REGISTER_STRING("net_firenetVersion", "1.0.0.0", gEnv->IsDedicated() ? VF_NULL : VF_CHEAT, "Sets the FireNet version for packet validation");

	if (!gEnv->IsDedicated())
	{
		REGISTER_CVAR2("net_firenetMasterPort", &mEnv->net_master_port, 3322, VF_NULL, "FireNet master server port");
	}
	else
	{
		REGISTER_CVAR2("net_firenetMasterPort", &mEnv->net_master_port, 64000, VF_NULL, "FireNet master server port");
	}
	
	REGISTER_CVAR2("net_firenetConnectionTimeout", &mEnv->net_master_timeout, 10, VF_CHEAT, "FireNet connection timeout");
	REGISTER_CVAR2("net_firenetAnswerTimeout", &mEnv->net_answer_timeout, 5, VF_CHEAT, "FireNet answer timeout");
	REGISTER_CVAR2("net_firenetAutoConnect", &mEnv->net_auto_connect, 0, VF_NULL, "Auto connect to FireNet after client init");
	//! CVars - Other
#ifndef  NDEBUG
	REGISTER_CVAR2("net_firenetPacketDebug", &mEnv->net_debug, 0, VF_CHEAT, "FireNet packet debugging");
	REGISTER_COMMAND("net_firenetConnect", CmdConnect, VF_CHEAT, "Connect to FireNet master server");
#endif
	REGISTER_CVAR2("net_firenetLogLevel", &mEnv->net_LogLevel, 0, VF_NULL, "FireNet log level (0 - 2)");
}

CFireNetCorePlugin::~CFireNetCorePlugin()
{
	//! Unregister entities
	IEntityRegistrator* pTemp = IEntityRegistrator::g_pFirst;
	while (pTemp != nullptr)
	{
		pTemp->Unregister();
		pTemp = pTemp->m_pNext;
	}

	//! Unregister CVars
	IConsole* pConsole = gEnv->pConsole;
	if (pConsole)
	{
		pConsole->UnregisterVariable("net_firenetMasterIP", true);
		pConsole->UnregisterVariable("net_firenetVersion", true);
		pConsole->UnregisterVariable("net_firenetMasterPort", true);
		pConsole->UnregisterVariable("net_firenetConnectionTimeout", true);
		pConsole->UnregisterVariable("net_firenetAnswerTimeout", true);
		pConsole->UnregisterVariable("net_firenetAutoConnect", true);
#ifndef NDEBUG
		pConsole->UnregisterVariable("net_firenetPacketDebug", true);
		pConsole->RemoveCommand("net_firenetConnect");
#endif
		pConsole->UnregisterVariable("net_firenetLogLevel", true);
	}

	//! Close network thread
	if (mEnv->pNetworkThread && mEnv->pTcpClient)
	{
		if (!gEnv->IsDedicated())
			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Network thread not deleted! Use Quit function to normal shutdown plugin!");

		mEnv->pNetworkThread->SignalStopWork();

		if (gEnv->pThreadManager)
			gEnv->pThreadManager->JoinThread(mEnv->pNetworkThread, eJM_Join);
	}

	//! Unregister listeners
	if (gEnv->pSystem)
		gEnv->pSystem->GetISystemEventDispatcher()->RemoveListener(this);

	SAFE_DELETE(mEnv->pNetworkThread);
	SAFE_DELETE(gFireNet);

	FireNetLog(TITLE "Unloaded.");
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
	gFireNet->pCore = static_cast<IFireNetCore*>(this);

	// Load FireNet thread config
	gEnv->pThreadManager->GetThreadConfigManager()->LoadConfig("config/firenet.thread_config");

	ICryPlugin::SetUpdateFlags(EUpdateType_Update);

	return true;
}

void CFireNetCorePlugin::OnPluginUpdate(EPluginUpdateType updateType)
{
	switch (updateType)
	{
	case IPluginUpdateListener::EUpdateType_Update:
	{
		//! Automatic deleting network thread if it's ready to close
		if (mEnv->pNetworkThread && mEnv->pNetworkThread->IsReadyToClose() && !gEnv->pSystem->IsQuitting())
		{
			FireNetLog(TITLE "Network thread stopped - delete it");
			SAFE_DELETE(mEnv->pNetworkThread);

			for (const auto &it : mEnv->m_Listeners)
			{
				it->OnReadyToReconnect();
			}
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

		//! Register console vars and commands
		RegisterCVars();
		break;
	}
	case ESYSTEM_EVENT_GAME_FRAMEWORK_INIT_DONE:
	{
		//! Read firenet.cfg
		if (gEnv->IsDedicated())
		{
			gEnv->pConsole->ExecuteString("exec firenet.cfg");
		}

		//! Automatic connect to master server
		if (!gEnv->IsEditor() && mEnv->net_auto_connect > 0)
			ConnectToMasterServer();

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

	if (mEnv->pNetworkThread == nullptr)
	{
		FireNet::SendEmptyEvent(FIRENET_EVENT_MASTER_SERVER_START_CONNECTION);

		mEnv->pNetworkThread = new CNetworkThread();

		if (!gEnv->pThreadManager->SpawnThread(mEnv->pNetworkThread, "FireNetCore_Thread"))
		{
			for (const auto &it : mEnv->m_Listeners)
			{
				it->OnConnectionError(EFireNetCoreErrorCodes::CantSpawnThread);
			}

			SAFE_DELETE(mEnv->pNetworkThread);

			CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't spawn FireNet core thread!");
		}
		else
			FireNetLog(TITLE "FireNet core thread spawned");
	}
	else
	{
		FireNetLog(TITLE "Network thread not deleted, try later");

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnConnectionError(EFireNetCoreErrorCodes::CantRemoveThread);
		}
	}
}

void CFireNetCorePlugin::Authorization(const char* login, const char* password)
{
	FireNetLog(TITLE "Try authorizate by login and password");

	std::string m_login(login);
	std::string m_password(password);

	if (!m_login.empty() && !m_password.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::Login);
		packet.WriteString(login);
		packet.WriteString(password);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't authorizate. Login or password empty");
	}	
}

void CFireNetCorePlugin::Registration(const char* login, const char* password)
{
	FireNetLog(TITLE "Try register new account by login and password");

	std::string m_login(login);
	std::string m_password(password);

	if (!m_login.empty() && !m_password.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::Register);
		packet.WriteString(login);
		packet.WriteString(password);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't register new account. Login or password empty");
	}
}

void CFireNetCorePlugin::CreateProfile(const char* nickname, const char* character)
{
	FireNetLog(TITLE "Try create profile by nickname and character model");

	std::string m_nickname(nickname);
	std::string m_character(character);

	if (!m_nickname.empty() && !m_character.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::CreateProfile);
		packet.WriteString(nickname);
		packet.WriteString(character);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't create new profile. Nickname or character model empty");
	}
}

void CFireNetCorePlugin::GetProfile(int uid)
{
	FireNetLog(TITLE "Try get profile");

	CTcpPacket packet(EFireNetTcpPacketType::Query, true);
	packet.WriteQuery(EFireNetTcpQuery::GetProfile);

	FireNet::SendPacket(packet);
}

SFireNetProfile * CFireNetCorePlugin::GetLocalProfile()
{
	FireNetLog(TITLE "%s - TODO", __FUNCTION__);
	return nullptr;
}

void CFireNetCorePlugin::GetShop()
{
	FireNetLog(TITLE "Try get shop");

	CTcpPacket packet(EFireNetTcpPacketType::Query, true);
	packet.WriteQuery(EFireNetTcpQuery::GetShop);

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::BuyItem(const char* item)
{
	FireNetLog(TITLE "Try buy item by name");

	std::string m_item(item);

	if (!m_item.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::BuyItem);
		packet.WriteString(item);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't buy item. Item name empty");
	}
}

void CFireNetCorePlugin::RemoveItem(const char* item)
{
	FireNetLog(TITLE "Try remove item by name");

	std::string m_item(item);
	if (!m_item.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::RemoveItem);
		packet.WriteString(item);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't remove item. Item name empty");
	}
}

void CFireNetCorePlugin::SendInvite(EFireNetInviteType type, int uid)
{
	FireNetLog(TITLE "Try send invite");

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
	FireNetLog(TITLE "Try decline invite");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::DeclineInvite);

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::AcceptInvite()
{
	FireNetLog(TITLE "Try accept invite");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::AcceptInvite);

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::RemoveFriend(int uid)
{
	FireNetLog(TITLE "Try remove friend");

	if (uid > 0)
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
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
	FireNetLog(TITLE "Try send chat message");

	CTcpPacket packet(EFireNetTcpPacketType::Query);
	packet.WriteQuery(EFireNetTcpQuery::SendChatMsg);
	packet.WriteInt(type);

	if (type == EMsg_PrivateChat)
	{
		packet.WriteInt(uid);
	}

	FireNet::SendPacket(packet);
}

void CFireNetCorePlugin::RegisterGameServer(SFireNetGameServer serverInfo)
{
	if (serverInfo.IsValid() && gEnv->IsDedicated())
	{
		FireNetLog(TITLE "Try register game server");

		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::RegisterServer);
		packet.WriteString(serverInfo.name);
		packet.WriteString(serverInfo.ip);
		packet.WriteInt(serverInfo.port);
		packet.WriteString(serverInfo.map);
		packet.WriteString(serverInfo.gamerules);
		packet.WriteInt(serverInfo.online);
		packet.WriteInt(serverInfo.maxPlayers);
		packet.WriteInt(static_cast<int>(serverInfo.status));
		packet.WriteInt(serverInfo.currentPID);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't register game server. Server info not valid!");
	}
}

void CFireNetCorePlugin::UpdateGameServer(SFireNetGameServer serverInfo)
{
	if (serverInfo.IsValid() && gEnv->IsDedicated())
	{
		FireNetLog(TITLE "Try update game server info ");

		CTcpPacket packet(EFireNetTcpPacketType::Query);
		packet.WriteQuery(EFireNetTcpQuery::UpdateServer);
		packet.WriteString(serverInfo.name);
		packet.WriteString(serverInfo.ip);
		packet.WriteInt(serverInfo.port);
		packet.WriteString(serverInfo.map);
		packet.WriteString(serverInfo.gamerules);
		packet.WriteInt(serverInfo.online);
		packet.WriteInt(serverInfo.maxPlayers);
		packet.WriteInt(static_cast<int>(serverInfo.status));
		packet.WriteInt(serverInfo.currentPID);

		FireNet::SendPacket(packet);
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't update game server info. Server info not valid!");
	}
}

void CFireNetCorePlugin::GetGameServer(const char* map, const char* gamerules)
{
	FireNetLog(TITLE "Try get game server");

	std::string m_map(map);
	std::string m_gamerules(gamerules);

	if (!m_map.empty() && !m_gamerules.empty())
	{
		CTcpPacket packet(EFireNetTcpPacketType::Query, true);
		packet.WriteQuery(EFireNetTcpQuery::GetServer);
		packet.WriteString(map);
		packet.WriteString(gamerules);

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

void CFireNetCorePlugin::GetFireNetEventListeners(std::vector<IFireNetListener*>& vector)
{
	vector = mEnv->m_Listeners;
}

void CFireNetCorePlugin::GetHWID(string &hwid)
{
	HW_PROFILE_INFO hwProfileInfo;
	if (GetCurrentHwProfile(&hwProfileInfo))
	{
		std::string str_hwid  = hwProfileInfo.szHwProfileGuid;
		hwid = str_hwid.c_str();
	}
}

void CFireNetCorePlugin::ExecuteEvent(EFireNetEvents eventName)
{
	FireNet::SendEmptyEvent(eventName);
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

	FireNetLog(TITLE "FireNet event listener registered (%p)", listener);

	mEnv->m_Listeners.push_back(listener);
}

void CFireNetCorePlugin::UnregisterFireNetListener(IFireNetListener * listener)
{
	if (!listener)
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Failed unregister FireNetEventListener. Null pointer");
		return;
	}

	for (auto it = mEnv->m_Listeners.begin(); it != mEnv->m_Listeners.end(); ++it)
	{
		if (*it == listener)
		{
			mEnv->m_Listeners.erase(it);
			return;
		}
	}
}

bool CFireNetCorePlugin::Quit()
{
	FireNetLog(TITLE "Closing plugin...");

	if (mEnv->pNetworkThread)
	{
		mEnv->pNetworkThread->SignalStopWork();
		gEnv->pThreadManager->JoinThread(mEnv->pNetworkThread, eJM_Join);
	}

	SAFE_DELETE(mEnv->pNetworkThread);

	if (mEnv->pNetworkThread == nullptr)
	{
		FireNetLog(TITLE "Plugin ready to unload");
		return true;
	}
	else
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE  "Can't normaly close plugin - network thread not deleted!");

	return false;
}

CRYREGISTER_SINGLETON_CLASS(CFireNetCorePlugin)