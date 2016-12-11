// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include "Global.h"

void CNetWorker::ReadPacket(const char* data)
{
	CryLog(TITLE  "Parsing packet data...");

	NetPacket m_packet(data);

	if (m_packet.getType() == net_Result)
	{
		CryLog(TITLE "Packet type = net_Result");

		switch (m_packet.ReadInt())
		{
#ifndef DEDICATED_SERVER
		case net_result_auth_complete:
		{
			CryLog(TITLE "Authorization complete. Profile not found");
			gModuleEnv->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnLoginComplete);
			break;
		}
		case net_result_auth_complete_with_profile:
		{
			CryLog(TITLE "Authorization complete. Profile found");
			gModuleEnv->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnLoginComplete);

			// Send get profile query
			NetPacket m_paket(net_Query);
			m_paket.WriteInt(net_query_get_profile);
			gModuleEnv->pNetwork->SendQuery(m_paket);
			break;
		}
		case net_result_register_complete:
		{
			CryLog(TITLE "Registration complete");
			gModuleEnv->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnRegComplete);
			break;
		}
		case net_result_profile_creation_complete:
		{
			CryLog(TITLE "Creating profile complete");
			onProfileDataRecived(m_packet);
			break;
		}
#endif
		case net_result_get_profile_complete:
		{
			CryLog(TITLE "Get profile complete");
			onProfileDataRecived(m_packet);
			break;
		}

#ifndef DEDICATED_SERVER
		case net_result_get_shop_complete:
		{
			CryLog(TITLE "Get shop complete");
			onShopItemsRecived(m_packet);
			break;
		}
		case net_result_buy_item_complete:
		{
			CryLog(TITLE "Buy item complete");
			break;
		}
		case net_result_remove_item_complete:
		{
			CryLog(TITLE "Remove item complete");
			break;
		}
		case net_result_send_invite_complete:
		{
			CryLog(TITLE "Send invite complete");
			break;
		}
		case net_result_decline_invite_complete:
		{
			CryLog(TITLE "Decline invite complete");
			break;
		}
		case net_result_accept_invite_complete:
		{
			CryLog(TITLE "Accept invite complete");
			break;
		}
		case net_result_add_friend_complete:
		{
			CryLog(TITLE "Add friend complete");
			break;
		}
		case net_result_remove_friend_complete:
		{
			CryLog(TITLE "Remove friend complete");
			break;
		}
		case net_result_send_chat_msg_complete:
		{
			CryLog(TITLE "Send chat message complete");
			break;
		}
		case net_result_get_server_complete:
		{
			CryLog(TITLE "Get game server complete");
			onGameServerDataRecived(m_packet);
			break;
		}
#else
		case net_result_remote_get_profile_complete:
		{
			CryLog(TITLE "Get profile complete");
			onProfileDataRecived(m_packet);
			break;
		}
		case net_result_remote_register_server_complete:
		{
			CryLog(TITLE "Register game server complete");
			break;
		}
		case net_result_remote_update_profile_complete:
		{
			CryLog(TITLE "Update profile complete");
			break;
		}
		case net_result_remote_update_server_complete:
		{
			CryLog(TITLE "Update game server info complete");
			break;
		}
#endif
		default:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't get result type");
			break;
		}
		}
	}
	else if (m_packet.getType() == net_Server)
	{
		CryLog(TITLE "Packet type = net_Server");

		switch (m_packet.ReadInt())
		{
		case net_server_command:
		{
			string m_value = m_packet.ReadString();
			CryLog(TITLE "Recived server command %s", m_value);
			gEnv->pConsole->ExecuteString(m_value);

			break;
		}
#ifndef DEDICATED_SERVER
		case net_server_global_chat_msg:
		{
			string from = m_packet.ReadString();
			QString message = m_packet.ReadString();
			QString clean = message.replace("#comma#", ",");

			SUIArguments args;
			args.AddArgument(clean.toStdString().c_str());
			args.AddArgument(from);
			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnGlobalChatMessageRecived, args);

			CryLog(TITLE "Recived global chat message from %s", from);
			break;
		}
		case net_server_message:
		{
			string message = m_packet.ReadString();
			CryLog(TITLE "Recived server message %s", message);

			SUIArguments args;
			args.AddArgument(message);
			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerMessageRecive, args);

			break;
		}
		case net_server_private_chat_msg:
		{
			string from = m_packet.ReadString();
			QString message = m_packet.ReadString();
			QString clean = message.replace("#comma#", ",");

			SUIArguments args;
			args.AddArgument(clean.toStdString().c_str());
			args.AddArgument(from);
			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnPrivateChatMessageRecived, args);

			CryLog(TITLE "Recived private chat message from %s", from);
			break;
		}
#endif
		default:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't get server packet type");
			break;
		}
		}
	}
	else if (m_packet.getType() == net_Error)
	{
		CryLog(TITLE "Packet type = net_Error");

		switch (m_packet.ReadInt())
		{
#ifndef DEDICATED_SERVER
		case net_result_auth_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Authorization failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@loginNotFound");
			else if (errorType == 1)
				args.AddArgument("@accountBlocked");
			else if (errorType == 2)
				args.AddArgument("@incorrectPassword");
			else if (errorType == 3)
				args.AddArgument("@doubleAuthorization");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_register_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Registration failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@loginAlredyRegistered");
			else if (errorType == 1)
				args.AddArgument("@serverError");
			else if (errorType == 2)
				args.AddArgument("@doubleRegistration");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_profile_creation_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Profile creation failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@clientAlredyHaveProfile");
			else if (errorType == 1)
				args.AddArgument("@nicknameAlredyRegister");
			else if (errorType == 2)
				args.AddArgument("@serverError");
			else if (errorType == 3)
				args.AddArgument("@doubleProfileCreation");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_get_profile_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Get profile failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@profileNotFound");
			else 
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);

			break;
		}
		case net_result_get_shop_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Get shop failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@serverError");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);

			break;
		}
		case net_result_buy_item_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Buy item failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@itemAlredyPurchased");
			else if (errorType == 1)
				args.AddArgument("@lowProfileLevel");
			else if (errorType == 2)
				args.AddArgument("@insufficientMoney");
			else if (errorType == 3)
				args.AddArgument("@itemNotFound");
			else if (errorType == 4)
				args.AddArgument("@profileNotFound");
			else if (errorType == 5)
				args.AddArgument("@profileNotUpdated");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_remove_item_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Remove item failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@itemNotFound");
			else if (errorType == 1)
				args.AddArgument("@itemNotFoundInProfile");
			else if (errorType == 2)
				args.AddArgument("@profileNotFound");
			else if (errorType == 3)
				args.AddArgument("@profileNotUpdated");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_send_invite_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Send invite failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@userNotFound");
			else if (errorType == 1)
				args.AddArgument("@userNotOnline");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_decline_invite_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Decline invite failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@userNotFound");
			else if (errorType == 1)
				args.AddArgument("@userNotOnline");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_accept_invite_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Accept invite failed! Error type = %d", errorType);
			break;
		}
		case net_result_add_friend_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Add friend failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@friendAlredyAdded");
			else if (errorType == 1)
				args.AddArgument("@cantAddYourself");
			else if (errorType == 2)
				args.AddArgument("@friendNotFound");
			else if (errorType == 3)
				args.AddArgument("@profileNotFound");
			else if (errorType == 4)
				args.AddArgument("@profileNotUpdated");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_remove_friend_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Remove friend failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@friendNotFound");
			else if (errorType == 1)
				args.AddArgument("@profileNotFound");
			else if (errorType == 2)
				args.AddArgument("@profileNotUpdated");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_send_chat_msg_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Send chat message failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@cantSendMessageYourself");
			else if (errorType == 1)
				args.AddArgument("@reciveNotOnline");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
		case net_result_get_server_fail:
		{
			int errorType = m_packet.ReadInt();
			CryLog(TITLE "Get game server failed! Error type = %d", errorType);

			SUIArguments args;

			if (errorType == 0)
				args.AddArgument("@notAnyOnlineServer");
			else if (errorType == 1)
				args.AddArgument("@serverNotFound");
			else
				args.AddArgument("@unknownError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			break;
		}
#else
        case net_result_remote_get_profile_fail:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Get profile failed!");
			break;
		}
		case net_result_remote_register_server_fail:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't register game server in FireNET!");
			break;
		}
		case net_result_remote_update_profile_fail:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't update profile!");
			break;
		}
		case net_result_remote_update_server_fail:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't update game server info in FireNET");
			break;
		}
#endif

		default:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't get error type");
			break;
		}
		}
	}
	else if (m_packet.getType() == net_Query)
	{
		CryLog(TITLE "Packet type = net_Query");

#ifndef DEDICATED_SERVER
		switch (m_packet.ReadInt())
		{
		case net_query_send_invite:
		{
			int inviteType = m_packet.ReadInt();
			string from = m_packet.ReadString();

			if (inviteType == 0) // Friend invite
			{
				SUIArguments args;
				args.AddArgument(from);
				gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnFriendInviteRecived, args);

				CryLog(TITLE  "Recived friend invite from %s", from.c_str());
			}
			
			break;
		}

		default:
		{
			CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't get query type");
			break;
		}

		}
#endif
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't get packet type");
	}
}

void CNetWorker::onProfileDataRecived(NetPacket &packet)
{
#ifndef DEDICATED_SERVER
	if (!gModuleEnv->m_profile)
		gModuleEnv->m_profile = new FireNET::SProfile;

	FireNET::SProfile* m_profile = gModuleEnv->m_profile;
	m_profile->uid = packet.ReadInt();
	m_profile->nickname = packet.ReadString();
	m_profile->fileModel = packet.ReadString();
	m_profile->lvl = packet.ReadInt();
	m_profile->xp = packet.ReadInt();
	m_profile->money = packet.ReadInt();
	m_profile->items = packet.ReadString();
	m_profile->friends = packet.ReadString();
	

	if (m_profile->uid > 0)
	{
		SUIArguments args;
		args.AddArgument(m_profile->nickname);
		args.AddArgument(m_profile->fileModel);
		args.AddArgument(m_profile->lvl);
		args.AddArgument(m_profile->xp);
		args.AddArgument(m_profile->money);
		gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnProfileDataRecived, args);

		CryLog(TITLE "Recived profile data. Uid = %d, Nickname = %s, FileModel = %s, Lvl = %d, XP = %d, Money = %d", m_profile->uid, m_profile->nickname, m_profile->fileModel, m_profile->lvl, m_profile->xp, m_profile->money);
	}
#else
	FireNET::SProfile m_profile;
	m_profile.uid = packet.ReadInt();
	m_profile.nickname = packet.ReadString();
	m_profile.fileModel = packet.ReadString();
	m_profile.lvl = packet.ReadInt();
	m_profile.xp = packet.ReadInt();
	m_profile.money = packet.ReadInt();
	m_profile.items = packet.ReadString();
	m_profile.friends = packet.ReadString();

	if (m_profile.uid > 0)
	{
		CryLog(TITLE "Try add new profile (%s)", m_profile.nickname);

		for (int i = 0; i < gModuleEnv->m_Profiles.size(); ++i)
		{
			if (gModuleEnv->m_Profiles[i].uid == m_profile.uid)
			{
				gModuleEnv->m_Profiles[i] = m_profile;
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "Profile (%s) alredy added. Updating...", m_profile.nickname);
			}
		}

		gModuleEnv->m_Profiles.push_back(m_profile);
		CryLog(TITLE "Profile (%s) added", m_profile.nickname);
	}
#endif
}

#ifndef DEDICATED_SERVER

void CNetWorker::onShopItemsRecived(NetPacket &packet)
{
/*	SUIArguments args;
	args.AddArgument(name);
	args.AddArgument(icon);
	args.AddArgument(description);
	args.AddArgument(cost);
	args.AddArgument(minLvl);

	gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnShopItemRecived, args);

	CryLog(TITLE "Recived shop item. Name = %s, Icon = %s, Description = %s, Cost = %d, MinLvl = %d", name, icon, description, cost, minLvl);*/
}

void CNetWorker::onGameServerDataRecived(NetPacket &packet)
{
	QString serverName = packet.ReadString();
	QString serverIp = packet.ReadString();
	int serverPort = packet.ReadInt();
	QString mapName = packet.ReadString();
	QString gamerules = packet.ReadString();
	int online = packet.ReadInt();
	int maxPlayers = packet.ReadInt();

	if (serverName.isEmpty() || serverIp.isEmpty() ||
		serverPort == 0 || mapName.isEmpty() || gamerules.isEmpty())
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE  "Game server data wrong or empty");
		return;
	}

	CryLog(TITLE  "Recived game server data");
	CryLog(TITLE  "Server name = %s, ip = %s, port = %d, map = %s, gamerules = %s, online = %d, maxPlayers = %d", serverName.toStdString().c_str(), serverIp.toStdString().c_str(),
		serverPort, mapName.toStdString().c_str(), gamerules.toStdString().c_str(), online, maxPlayers);

	ICVar* pClServerIp = gEnv->pConsole->GetCVar("cl_serveraddr");
	ICVar* pClServerPort = gEnv->pConsole->GetCVar("cl_serverport");
	ICVar* pClNickname = gEnv->pConsole->GetCVar("cl_nickname");

	if (pClServerIp && pClServerPort)
	{
		CryLog(TITLE "Set client parametres");

		pClServerIp->Set(serverIp.toStdString().c_str());
		pClServerPort->Set(serverPort);
		pClNickname->Set(gModuleEnv->m_profile->nickname);

		SUIArguments args;
		args.AddArgument(serverName.toStdString().c_str());
		args.AddArgument(serverIp.toStdString().c_str());
		args.AddArgument(serverPort);
		gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnMatchmakingSuccess, args);

		CryLog(TITLE  "New client parametres : address = %s , port = %d, nickname = %s", pClServerIp->GetString(), pClServerPort->GetIVal(), pClNickname->GetString());
	}

	return;
}

#endif