// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"
#include "TcpPacket.h"

void CReadQueue::ReadPacket(CTcpPacket & packet)
{
	switch (packet.getType())
	{
	case EFireNetTcpPacketType::Empty:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Packet type = EFireNetTcpPacketType::Empty");
		break;
	}
	case EFireNetTcpPacketType::Query:
	{
		FireNetLogDebug(TITLE "Packet type = EFireNetTcpPacketType::Query");
		ReadQuery(packet, packet.ReadQuery());
		break;
	}
	case EFireNetTcpPacketType::Result:
	{
		FireNetLogDebug(TITLE "Packet type = EFireNetTcpPacketType::Result");
		ReadResult(packet, packet.ReadResult());
		break;
	}
	case EFireNetTcpPacketType::Error:
	{
		FireNetLogDebug(TITLE "Packet type = EFireNetTcpPacketType::Error");
		ReadError(packet, packet.ReadError());
		break;
	}
	case EFireNetTcpPacketType::ServerMessage:
	{
		FireNetLogDebug(TITLE "Packet type = EFireNetTcpPacketType::ServerMessage");
		ReadServerMsg(packet, packet.ReadSMessage());
		break;
	}
	default:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Unknown packet type!");
		break;
	}
	}
}

void CReadQueue::ReadQuery(CTcpPacket & packet, EFireNetTcpQuery querry)
{
	switch (querry)
	{
	case EFireNetTcpQuery::StartServerPrepare:
	{
		if (gEnv->IsDedicated())
		{
			int m_PvPSession = packet.ReadInt();

			FireNetLogAlways(TITLE "Start prepare server for playing ... PvP session = %d", m_PvPSession);

			for (const auto &it : mEnv->m_Listeners)
			{
				it->OnGameServerStartPrepare(m_PvPSession);
			}

			break;
		}
	}
	case EFireNetTcpQuery::RequestServerReload:
	{
		if (gEnv->IsDedicated())
		{
			FireNetLogAlways(TITLE "Master server request reload after game finished...");
			FireNet::SendEmptyEvent(FIRENET_EVENT_SERVER_NEED_RELOAD);
		}

		break;
	}
	case EFireNetTcpQuery::PingPong:
	{
		// Send ping-pong packets
		if (gEnv->IsDedicated())
		{
			CTcpPacket packet(EFireNetTcpPacketType::Query);
			packet.WriteQuery(EFireNetTcpQuery::PingPong);

			FireNet::SendPacket(packet);
		}
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadResult(CTcpPacket & packet, EFireNetTcpResult result)
{
	switch (result)
	{	
	case EFireNetTcpResult::LoginComplete :
	{
		FireNetLog(TITLE "Authorization complete. Profile not found");
		FireNet::SendEmptyEvent(FIRENET_EVENT_AUTHORIZATION_COMPLETE);

		break;
	}
	case EFireNetTcpResult::LoginCompleteWithProfile :
	{
		FireNetLog(TITLE "Authorization complete. Profile loading...");
		FireNet::SendEmptyEvent(FIRENET_EVENT_AUTHORIZATION_COMPLETE_WITH_PROFILE);

		if (gFireNet && gFireNet->pCore)
		{
			gFireNet->pCore->GetProfile();
		}

		break;
	}	
	case EFireNetTcpResult::RegisterComplete :
	{
		FireNetLog(TITLE "Registration complete");
		FireNet::SendEmptyEvent(FIRENET_EVENT_REGISTRATION_COMPLETE);

		break;
	}	
	case EFireNetTcpResult::ProfileCreationComplete :
	{
		FireNetLog(TITLE "Creating profile complete. Loading...");
		FireNet::SendEmptyEvent(FIRENET_EVENT_CREATE_PROFILE_COMPLETE);
		LoadProfile(packet);

		break;
	}	
	case EFireNetTcpResult::GetProfileComplete :
	{
		FireNetLog(TITLE "Get profile complete. Loading...");
		FireNet::SendEmptyEvent(FIRENET_EVENT_GET_PROFILE_COMPLETE);
		LoadProfile(packet);

		break;
	}	
	case EFireNetTcpResult::GetShopComplete :
	{
		FireNetLog(TITLE "Get shop complete. Loading...");
		LoadShop(packet);

		break;
	}	
	case EFireNetTcpResult::BuyItemComplete :
	{
		FireNetLog(TITLE "Buy item complete. Updating profile...");
		FireNet::SendEmptyEvent(FIRENET_EVENT_BUY_ITEM_COMPLETE);
		LoadProfile(packet);

		break;
	}
	case EFireNetTcpResult::RemoveItemComplete :
	{
		FireNetLog(TITLE "Remove item complete. Updating profile...");
		FireNet::SendEmptyEvent(FIRENET_EVENT_REMOVE_ITEM_COMPLETE);
		LoadProfile(packet);

		break;
	}	
	case EFireNetTcpResult::SendInviteComplete :
	{
		FireNetLog(TITLE "Send invite complete");
		FireNet::SendEmptyEvent(FIRENET_EVENT_SEND_INVITE_COMPLETE);
		break;
	}	
	case EFireNetTcpResult::DeclineInviteComplete :
	{
		FireNetLog(TITLE "Decline invite complete");
		FireNet::SendEmptyEvent(FIRENET_EVENT_DECLINE_INVITE_COMPLETE);
		break;
	}	
	case EFireNetTcpResult::AcceptInviteComplete :
	{
		FireNetLog(TITLE "Accept invite complete");
		FireNet::SendEmptyEvent(FIRENET_EVENT_ACCEPT_INVITE_COMPLETE);
		break;
	}
	case EFireNetTcpResult::RemoveFriendComplete :
	{
		FireNetLog(TITLE "Remove friend complete. Updating profile...");
		FireNet::SendEmptyEvent(FIRENET_EVENT_REMOVE_FRIEND_COMPLETE);
		LoadProfile(packet);
		break;
	}
	case EFireNetTcpResult::SendChatMsgComplete :
	{
		FireNetLog(TITLE "Send chat message complete");
		FireNet::SendEmptyEvent(FIRENET_EVENT_SEND_CHAT_MSG_COMPLETE);
		break;
	}	
	case EFireNetTcpResult::GetServerComplete :
	{
		FireNetLog(TITLE "Get game server complete. Connecting...");
		LoadGameServerInfo(packet);
		break;
	}
	case EFireNetTcpResult::RegisterServerComplete :
	{
		FireNetLog(TITLE "Register game server complete");
		FireNet::SendEmptyEvent(FIRENET_EVENT_SERVER_REGISTERED);
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadError(CTcpPacket & packet, EFireNetTcpError error)
{
	EFireNetTcpErrorCode errorCode = packet.ReadErrorCode();
	const char* m_Error = ErrorCodeToString(errorCode);
	int reason = static_cast<int>(errorCode);

	switch (error)
	{
	case EFireNetTcpError::LoginFail :
	{	
		FireNetLog(TITLE "Authorization failed. Reason = %s", m_Error);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnAuthorizationFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::RegisterFail :
	{
		FireNetLog(TITLE "Registration failed. Reason = %s", m_Error);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnRegistrationFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::ProfileCreationFail :
	{
		FireNetLog(TITLE "Creating profile failed. Reason = %s", m_Error);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnPlayerProfileCreationFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::GetProfileFail :
	{
		FireNetLog(TITLE "Get profile failed. Reason = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnGetPlayerProfileFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::GetShopFail :
	{
		FireNetLog(TITLE "Get shop failed. Reason = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnGetShopItemsFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::BuyItemFail :
	{
		FireNetLog(TITLE "Buy item failed. Reason = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnBuyItemFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::RemoveItemFail :
	{
		FireNetLog(TITLE "Remove item failed. Reasong = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnRemoveItemFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::SendInviteFail :
	{
		FireNetLog(TITLE "Send invite failed. Reason = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnSendInviteFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::DeclineInviteFail : // TODO
	{
		FireNetLog(TITLE "Decline invite failed. Reason = %d", reason);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnDeclineInviteFailed(reason);
		}

		break;
	}
	case EFireNetTcpError::AcceptInviteFail : // TODO
	{
		FireNetLog(TITLE "Accept invite failed. Reason = %d", reason);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnAcceptInviteFailed(reason);
		}

		break;
	}
	case EFireNetTcpError::RemoveFriendFail :
	{
		FireNetLog(TITLE "Remove friend failed. Reason = %s", m_Error);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnRemoveFriendFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::SendChatMsgFail :
	{
		FireNetLog(TITLE "Send chat message failed. Reason = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnSendChatMsgFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::GetServerFail :
	{
		FireNetLog(TITLE "Get game server failed. Reason = %s", m_Error);
		
		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnGetGameServerFailed(reason, m_Error);
		}

		break;
	}
	case EFireNetTcpError::RegisterServerFail :
	{
		FireNetLog(TITLE "Register game server failed. Reason = %s", m_Error);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnGameServerRegisterFailed(reason, m_Error);
		}

		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadServerMsg(CTcpPacket & packet, EFireNetTcpSMessage serverMsg)
{
	switch (serverMsg)
	{
	case EFireNetTcpSMessage::GlobalChatMsg :
	{
		FireNetLog(TITLE "Received global chat message");

		string from = packet.ReadString();
		string msg = packet.ReadString();

		FireNetLog(TITLE "[GlobalChat] %s : %s", from, msg);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnGlobalChatMsgReceived(from.c_str(), msg.c_str());
		}

		break;
	}
	case EFireNetTcpSMessage::PrivateChatMsg :
	{
		FireNetLog(TITLE "Received private chat message");

		string from = packet.ReadString();
		string msg = packet.ReadString();

		FireNetLog(TITLE "[PrivateChat] %s : %s", from, msg);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnPrivateChatMsgReceived(from.c_str(), msg.c_str());
		}

		break;
	}
	case EFireNetTcpSMessage::ClanChatMsg :
	{
		FireNetLog(TITLE "Received clan chat message");

		string from = packet.ReadString();
		string msg = packet.ReadString();

		FireNetLog(TITLE "[ClanChat] %s : %s", from, msg);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnClanChatMsgReceived(from.c_str(), msg.c_str());
		}

		break;
	}
	case EFireNetTcpSMessage::ServerMessage :
	{
		FireNetLog(TITLE "Received server message");

		string msg = packet.ReadString();

		FireNetLogAlways(TITLE "[ServerMessage] %s ", msg);

		for (const auto &it : mEnv->m_Listeners)
		{
			it->OnServerMsgReceived(msg.c_str());
		}

		break;
	}
	case EFireNetTcpSMessage::ServerCommand :
	{
		FireNetLog(TITLE "Received server command");

		const char* rawCmd = packet.ReadString();
		
		if (rawCmd)
		{
			FireNetLog("[Command] %s", rawCmd);

			for (const auto &it : mEnv->m_Listeners)
			{
				it->OnConsoleCommandReceived(rawCmd);
			}
		}

		break;
	}
	default:
		break;
	}
}

void CReadQueue::LoadProfile(CTcpPacket & packet)
{
	int uid = packet.ReadInt();
	string nickname = packet.ReadString();
	string fileModel = packet.ReadString();
	int lvl = packet.ReadInt();
	int xp = packet.ReadInt();
	int money = packet.ReadInt();
	string items = packet.ReadString();
#ifndef STEAM_SDK_ENABLED
	string friends = packet.ReadString();
#endif

	FireNetLogDebug(TITLE "Profile : uid = %d", uid);
	FireNetLogDebug(TITLE "Profile : nickname = %s", nickname);
	FireNetLogDebug(TITLE "Profile : fileModel = %s", fileModel);
	FireNetLogDebug(TITLE "Profile : lvl = %d", lvl);
	FireNetLogDebug(TITLE "Profile : xp = %d", xp);
	FireNetLogDebug(TITLE "Profile : money = %d", money);
	FireNetLogDebug(TITLE "Profile : items = %s", items);
#ifndef STEAM_SDK_ENABLED
	FireNetLogDebug(TITLE "Profile : friends = %s", friends);
#endif

	// Send event with profile data
	SFireNetProfile profile;
	profile.uid = uid;
	profile.nickname = nickname;
	profile.fileModel = fileModel;
	profile.lvl = lvl;
	profile.xp = xp;
	profile.money = money;
	ParseProfileItems(items.c_str(), profile.items);
#ifndef STEAM_SDK_ENABLED
	ParseProfileFriends(friends.c_str(), profile.friends);
#endif

	for (const auto &it : mEnv->m_Listeners)
	{
		it->OnPlayerProfileUpdated(profile);
	}
}

void CReadQueue::LoadShop(CTcpPacket & packet)
{
	string rawShop = packet.ReadString();

	// TODO

	FireNet::SendEmptyEvent(FIRENET_EVENT_GET_SHOP_COMPLETE);
}

void CReadQueue::LoadGameServerInfo(CTcpPacket & packet)
{
	// TODO
}

void CReadQueue::ParseProfileItems(const char * items, std::vector<SFireNetItem>& to)
{
	// TODO
}

#ifndef STEAM_SDK_ENABLED
void CReadQueue::ParseProfileFriends(const char * friends, std::vector<SFireNetFriend>& to)
{
}
#endif