// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "StdAfx.h"
#include "ReadQueue.h"
#include "TcpPacket.h"

void CReadQueue::ReadPacket(CTcpPacket & packet)
{
	// Server can't send to client empty or query packet, it's wrong, but you can see that if it happened
	switch (packet.getType())
	{
	case EFireNetTcpPacketType::Empty:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Packet type = EFireNetTcpPacketType::Empty");
		break;
	}
	case EFireNetTcpPacketType::Query:
	{
		CryWarning(VALIDATOR_MODULE_NETWORK, VALIDATOR_ERROR, TITLE "Packet type = EFireNetTcpPacketType::Query");
		break;
	}
	case EFireNetTcpPacketType::Result:
	{
		CryLog(TITLE "Packet type = EFireNetTcpPacketType::Result");
		ReadResult(packet, packet.ReadResult());
		break;
	}
	case EFireNetTcpPacketType::Error:
	{
		CryLog(TITLE "Packet type = EFireNetTcpPacketType::Error");
		ReadError(packet, packet.ReadError());
		break;
	}
	case EFireNetTcpPacketType::ServerMessage:
	{
		CryLog(TITLE "Packet type = EFireNetTcpPacketType::ServerMessage");
		ReadServerMsg(packet, packet.ReadSMessage());
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
		CryLog(TITLE "Authorization complete. Profile not found");	
		FireNet::SendFireNetEvent(FIRENET_EVENT_AUTHORIZATION_COMPLETE);

		break;
	}
	case EFireNetTcpResult::LoginCompleteWithProfile :
	{
		CryLog(TITLE "Authorization complete. Profile loading...");
		FireNet::SendFireNetEvent(FIRENET_EVENT_AUTHORIZATION_COMPLETE_WITH_PROFILE);

		if (gFireNet && gFireNet->pCore)
		{
			gFireNet->pCore->GetProfile();
		}

		break;
	}	
	case EFireNetTcpResult::RegisterComplete :
	{
		CryLog(TITLE "Registration complete");
		FireNet::SendFireNetEvent(FIRENET_EVENT_REGISTRATION_COMPLETE);

		break;
	}	
	case EFireNetTcpResult::ProfileCreationComplete :
	{
		CryLog(TITLE "Creating profile complete. Loading...");
		FireNet::SendFireNetEvent(FIRENET_EVENT_CREATE_PROFILE_COMPLETE);
		LoadProfile(packet);

		break;
	}	
	case EFireNetTcpResult::GetProfileComplete :
	{
		CryLog(TITLE "Get profile complete. Loading...");
		FireNet::SendFireNetEvent(FIRENET_EVENT_GET_PROFILE_COMPLETE);
		LoadProfile(packet);

		break;
	}	
	case EFireNetTcpResult::GetShopComplete :
	{
		CryLog(TITLE "Get shop complete. Loading...");
		LoadShop(packet);

		break;
	}	
	case EFireNetTcpResult::BuyItemComplete :
	{
		CryLog(TITLE "Buy item complete. Updating profile...");
		FireNet::SendFireNetEvent(FIRENET_EVENT_BUY_ITEM_COMPLETE);
		LoadProfile(packet);

		break;
	}
	case EFireNetTcpResult::RemoveItemComplete :
	{
		CryLog(TITLE "Remove item complete. Updating profile...");
		FireNet::SendFireNetEvent(FIRENET_EVENT_REMOVE_ITEM_COMPLETE);
		LoadProfile(packet);

		break;
	}	
	case EFireNetTcpResult::SendInviteComplete :
	{
		CryLog(TITLE "Send invite complete");
		FireNet::SendFireNetEvent(FIRENET_EVENT_SEND_INVITE_COMPLETE);
		break;
	}	
	case EFireNetTcpResult::DeclineInviteComplete :
	{
		CryLog(TITLE "Decline invite complete");
		FireNet::SendFireNetEvent(FIRENET_EVENT_DECLINE_INVITE_COMPLETE);
		break;
	}	
	case EFireNetTcpResult::AcceptInviteComplete :
	{
		CryLog(TITLE "Accept invite complete");
		FireNet::SendFireNetEvent(FIRENET_EVENT_ACCEPT_INVITE_COMPLETE);
		break;
	}
	case EFireNetTcpResult::RemoveFriendComplete :
	{
		CryLog(TITLE "Remove friend complete. Updating profile...");
		FireNet::SendFireNetEvent(FIRENET_EVENT_REMOVE_FRIEND_COMPLETE);
		LoadProfile(packet);
		break;
	}
	case EFireNetTcpResult::SendChatMsgComplete :
	{
		CryLog(TITLE "Send chat message complete");
		FireNet::SendFireNetEvent(FIRENET_EVENT_SEND_CHAT_MSG_COMPLETE);
		break;
	}	
	case EFireNetTcpResult::GetServerComplete :
	{
		CryLog(TITLE "Get game server complete. Connecting...");
		LoadGameServerInfo(packet);
		break;
	}
	default:
		break;
	}
}

void CReadQueue::ReadError(CTcpPacket & packet, EFireNetTcpError error)
{
	int reason = packet.ReadInt();
	SFireNetEventArgs args;

	switch (error)
	{
	case EFireNetTcpError::LoginFail :
	{	
		const char* m_Error;

		if (reason == 0)
			m_Error = "login_not_found";
		else if (reason == 1)
			m_Error = "account_blocked";
		else if (reason == 2)
			m_Error = "incorrect_password";
		else if (reason == 3)
			m_Error = "double_authorization";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Authorization failed. Reason = %s", m_Error);

		FireNet::SendFireNetEvent(FIRENET_EVENT_AUTHORIZATION_FAILED, args);
		break;
	}
	case EFireNetTcpError::RegisterFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "login_alredy_register";
		else if (reason == 1)
			m_Error = "cant_create_account";
		else if (reason == 2)
			m_Error = "double_registration";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Registration failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_REGISTRATION_FAILED, args);
		break;
	}
	case EFireNetTcpError::ProfileCreationFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "client_alredy_have_profile";
		else if (reason == 1)
			m_Error = "nickname_alredy_registered";
		else if (reason == 2)
			m_Error = "server_error";
		else if (reason == 3)
			m_Error = "double_profile_creation";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Creating profile failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_CREATE_PROFILE_FAILED, args);
		break;
	}
	case EFireNetTcpError::GetProfileFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "profile_not_found";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Get profile failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_GET_PROFILE_FAILED, args);
		break;
	}
	case EFireNetTcpError::GetShopFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "server_error";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Get shop failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_GET_SHOP_FAILED, args);
		break;
	}
	case EFireNetTcpError::BuyItemFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "item_alredy_purchased";
		else if (reason == 1)
			m_Error = "player_lvl_block";
		else if (reason == 2)
			m_Error = "insufficient_money";
		else if (reason == 3)
			m_Error = "item_not_found";
		else if (reason == 4)
			m_Error = "server_error";
		else if (reason == 5)
			m_Error = "server_error";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Buy item failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_BUY_ITEM_FAILED, args);
		break;
	}
	case EFireNetTcpError::RemoveItemFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "item_not_found_in_shop";
		else if (reason == 1)
			m_Error = "item_not_found_in_profile";
		else if (reason == 2)
			m_Error = "server_error";
		else if (reason == 3)
			m_Error = "server_error";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Remove item failed. Reasong = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_REMOVE_ITEM_FAILED, args);
		break;
	}
	case EFireNetTcpError::SendInviteFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "user_not_found";
		else if (reason == 1)
			m_Error = "user_not_online";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Send invite failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_SEND_INVITE_FAILED, args);
		break;
	}
	case EFireNetTcpError::DeclineInviteFail : // TODO
	{
		CryLog(TITLE "Decline invite failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_DECLINE_INVITE_FAILED, args);
		break;
	}
	case EFireNetTcpError::AcceptInviteFail : // TODO
	{
		CryLog(TITLE "Accept invite failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_ACCEPT_INVITE_FAILED, args);
		break;
	}
	case EFireNetTcpError::RemoveFriendFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "friend_not_found";
		else if (reason == 1)
			m_Error = "server_error";
		else if (reason == 2)
			m_Error = "server_error";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Remove friend failed. Reason = %s", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_REMOVE_FRIEND_FAILED, args);
		break;
	}
	case EFireNetTcpError::SendChatMsgFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "cant_send_msg_yourself";
		else if (reason == 1)
			m_Error = "reciver_not_online";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Send chat message failed. Reason = %d", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_SEND_CHAT_MSG_FAILED, args);
		break;
	}
	case EFireNetTcpError::GetServerFail :
	{
		const char* m_Error;

		if (reason == 0)
			m_Error = "not_any_online_servers";
		else if (reason == 1)
			m_Error = "server_not_found";
		else
			m_Error = "unknown_error";

		args.AddInt(reason);
		args.AddString(m_Error);

		CryLog(TITLE "Get game server failed. Reason = %d", m_Error);
		FireNet::SendFireNetEvent(FIRENET_EVENT_GET_GAME_SERVER_FAILED, args);
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
		CryLog(TITLE "Received global chat message");

		string from = packet.ReadString();
		string msg = packet.ReadString();

		CryLog(TITLE "[GlobalChat] %s : %s", from, msg);

		SFireNetEventArgs chat;
		chat.AddString(from);
		chat.AddString(msg);

		FireNet::SendFireNetEvent(FIRENET_EVENT_GLOBAL_CHAT_MSG_RECEIVED, chat);

		break;
	}
	case EFireNetTcpSMessage::PrivateChatMsg :
	{
		CryLog(TITLE "Received private chat message");

		string from = packet.ReadString();
		string msg = packet.ReadString();

		CryLog(TITLE "[PrivateChat] %s : %s", from, msg);

		SFireNetEventArgs chat;
		chat.AddString(from);
		chat.AddString(msg);

		FireNet::SendFireNetEvent(FIRENET_EVENT_PRIVATE_CHAT_MSG_RECEIVED, chat);

		break;
	}
	case EFireNetTcpSMessage::ClanChatMsg :
	{
		CryLog(TITLE "Received clan chat message");

		string from = packet.ReadString();
		string msg = packet.ReadString();

		CryLog(TITLE "[ClanChat] %s : %s", from, msg);

		SFireNetEventArgs chat;
		chat.AddString(from);
		chat.AddString(msg);

		FireNet::SendFireNetEvent(FIRENET_EVENT_CLAN_CHAT_MSG_RECEIVED, chat);

		break;
	}
	case EFireNetTcpSMessage::ServerMessage :
	{
		CryLog(TITLE "Received server message");

		string msg = packet.ReadString();

		CryLog(TITLE "[ServerMessage] %s ", msg);

		SFireNetEventArgs chat;
		chat.AddString(msg);

		FireNet::SendFireNetEvent(FIRENET_EVENT_SERVER_MESSAGE_RECEIVED, chat);

		break;
	}
	case EFireNetTcpSMessage::ServerCommand :
	{
		CryLog(TITLE "Received server command");

		string rawCmd = packet.ReadString();

		SFireNetEventArgs command;
		command.AddString(rawCmd);

		FireNet::SendFireNetEvent(FIRENET_EVENT_CONSOLE_COMMAND_RECEIVED, command);

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
	string friends = packet.ReadString();

	CryLog(TITLE "Profile : uid = %d", uid);
	CryLog(TITLE "Profile : nickname = %s", nickname);
	CryLog(TITLE "Profile : fileModel = %s", fileModel);
	CryLog(TITLE "Profile : lvl = %d", lvl);
	CryLog(TITLE "Profile : xp = %d", xp);
	CryLog(TITLE "Profile : money = %d", money);
	CryLog(TITLE "Profile : items = %s", items);
	CryLog(TITLE "Profile : friends = %s", friends);

	// Send event with profile data
	SFireNetEventArgs profile;
	profile.AddInt(uid);
	profile.AddString(nickname);
	profile.AddString(fileModel);
	profile.AddInt(lvl);
	profile.AddInt(xp);
	profile.AddInt(money);
	profile.AddString(items);
	profile.AddString(friends);

	FireNet::SendFireNetEvent(FIRENET_EVENT_UPDATE_PROFILE, profile);
}

void CReadQueue::LoadShop(CTcpPacket & packet)
{
	string rawShop = packet.ReadString();

	// TODO

	FireNet::SendFireNetEvent(FIRENET_EVENT_GET_SHOP_COMPLETE);
}

void CReadQueue::LoadGameServerInfo(CTcpPacket & packet)
{
	FireNet::SendFireNetEvent(FIRENET_EVENT_GET_GAME_SERVER_COMPLETE);
}
