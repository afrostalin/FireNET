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
	args.AddInt(reason);

	switch (error)
	{
	case EFireNetTcpError::LoginFail :
	{		
		CryLog(TITLE "Authorization failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_AUTHORIZATION_FAILED, args);
		break;
	}
	case EFireNetTcpError::RegisterFail :
	{
		CryLog(TITLE "Registration failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_REGISTRATION_FAILED, args);
		break;
	}
	case EFireNetTcpError::ProfileCreationFail :
	{
		CryLog(TITLE "Creating profile failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_CREATE_PROFILE_FAILED, args);
		break;
	}
	case EFireNetTcpError::GetProfileFail :
	{
		CryLog(TITLE "Get profile failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_GET_PROFILE_FAILED, args);
		break;
	}
	case EFireNetTcpError::GetShopFail :
	{
		CryLog(TITLE "Get shop failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_GET_SHOP_FAILED, args);
		break;
	}
	case EFireNetTcpError::BuyItemFail :
	{
		CryLog(TITLE "Buy item failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_BUY_ITEM_FAILED, args);
		break;
	}
	case EFireNetTcpError::RemoveItemFail :
	{
		CryLog(TITLE "Remove item failed. Reasong = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_REMOVE_ITEM_FAILED, args);
		break;
	}
	case EFireNetTcpError::SendInviteFail :
	{
		CryLog(TITLE "Send invite failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_SEND_INVITE_FAILED, args);
		break;
	}
	case EFireNetTcpError::DeclineInviteFail :
	{
		CryLog(TITLE "Decline invite failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_DECLINE_INVITE_FAILED, args);
		break;
	}
	case EFireNetTcpError::AcceptInviteFail :
	{
		CryLog(TITLE "Accept invite failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_ACCEPT_INVITE_FAILED, args);
		break;
	}
	case EFireNetTcpError::RemoveFriendFail :
	{
		CryLog(TITLE "Remove friend failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_REMOVE_FRIEND_FAILED, args);
		break;
	}
	case EFireNetTcpError::SendChatMsgFail :
	{
		CryLog(TITLE "Send chat message failed. Reason = %d", reason);
		FireNet::SendFireNetEvent(FIRENET_EVENT_SEND_CHAT_MSG_FAILED, args);
		break;
	}
	case EFireNetTcpError::GetServerFail :
	{
		CryLog(TITLE "Get game server failed. Reason = %d", reason);
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

		CryLog(TITLE "[ServerMessafe] %s ", msg);

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

	// Send event with shop data
	SFireNetEventArgs shop;
	shop.AddString(rawShop);
	FireNet::SendFireNetEvent(FIRENET_EVENT_GET_SHOP_COMPLETE, shop);
}

void CReadQueue::LoadGameServerInfo(CTcpPacket & packet)
{
	// Send event with game server info
	SFireNetEventArgs gameServerInfo;
	FireNet::SendFireNetEvent(FIRENET_EVENT_GET_GAME_SERVER_COMPLETE, gameServerInfo);
}
