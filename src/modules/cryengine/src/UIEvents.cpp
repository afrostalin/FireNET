// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "StdAfx.h"
#include "Global.h"


bool CModuleUIEvents::RegisterUIEvents()
{
	gEnv->pLog->Log(TITLE "Register UI events");

	if (gEnv->pFlashUI)
	{		
		m_pGameEvents = gEnv->pFlashUI->CreateEventSystem("FireNET", IUIEventSystem::eEST_SYSTEM_TO_UI);

		// On error
		SUIEventDesc OnError("System:OnError", "System:OnError", "Event when client give some error from online server");
		OnError.AddParam<SUIParameterDesc::eUIPT_String>("Error", "Error string");
		m_EventMap[eUIGE_OnError] = m_pGameEvents->RegisterEvent(OnError);

		// On login complete
		SUIEventDesc OnLoginComplete("AuthSystem:OnLoginComplete", "AuthSystem:OnLoginComplete", "Event when client login in online server");
		m_EventMap[eUIGE_OnLoginComplete] = m_pGameEvents->RegisterEvent(OnLoginComplete);

		// On register complete
		SUIEventDesc OnRegComplete("AuthSystem:OnRegisterComplete", "AuthSystem:OnRegisterComplete", "Event when client register in online server");
		m_EventMap[eUIGE_OnRegComplete] = m_pGameEvents->RegisterEvent(OnRegComplete);

		// On character data recived
		SUIEventDesc OnProfileDataRecived("Profile:OnProfileDataRecived", "Profile:OnProfileDataRecived", "Event when client recived profile data from online server");
		OnProfileDataRecived.AddParam<SUIParameterDesc::eUIPT_String>("Nickname", "Nickname");
		OnProfileDataRecived.AddParam<SUIParameterDesc::eUIPT_String>("Model", "Character model name");
		OnProfileDataRecived.AddParam<SUIParameterDesc::eUIPT_Int>("Level", "Character level");
		OnProfileDataRecived.AddParam<SUIParameterDesc::eUIPT_Int>("XP", "Character xp");
		OnProfileDataRecived.AddParam<SUIParameterDesc::eUIPT_Int>("Money", "Character money");
		m_EventMap[eUIGE_OnProfileDataRecived] = m_pGameEvents->RegisterEvent(OnProfileDataRecived);

		// On shop item recived
		SUIEventDesc OnShopItemRecived("Shop:OnShopItemRecived", "Shop:OnShopItemRecived", "Event when client recived shop item from online server");
		OnShopItemRecived.AddParam<SUIParameterDesc::eUIPT_String>("Name", "Item name");
		OnShopItemRecived.AddParam<SUIParameterDesc::eUIPT_String>("Icon", "Item icon");
		OnShopItemRecived.AddParam<SUIParameterDesc::eUIPT_String>("Description", "Item description");
		OnShopItemRecived.AddParam<SUIParameterDesc::eUIPT_Int>("Cost", "Item cost");
		OnShopItemRecived.AddParam<SUIParameterDesc::eUIPT_Int>("MinLevel", "Minimum character level to buy this item");
		m_EventMap[eUIGE_OnShopItemRecived] = m_pGameEvents->RegisterEvent(OnShopItemRecived);

		// On inventory item recived
		SUIEventDesc OnInventoryItemRecived("Inventory:OnInventoryItemRecived", "Inventory:OnInventoryItemRecived", "Event when client recived inventory item from online server");
		OnInventoryItemRecived.AddParam<SUIParameterDesc::eUIPT_String>("Name", "Item name");
		OnInventoryItemRecived.AddParam<SUIParameterDesc::eUIPT_String>("Icon", "Item icon");
		OnInventoryItemRecived.AddParam<SUIParameterDesc::eUIPT_String>("Description", "Item description");
		m_EventMap[eUIGE_OnInventoryItemRecived] = m_pGameEvents->RegisterEvent(OnInventoryItemRecived);

		// On friend recived
		SUIEventDesc OnFriendRecived("Friends:OnFriendRecived", "Friends:OnFriendRecived", "Event when client recived friend from online server");
		OnFriendRecived.AddParam<SUIParameterDesc::eUIPT_String>("Name", "Friend name");
		OnFriendRecived.AddParam<SUIParameterDesc::eUIPT_Int>("Uid", "Friend uid");
		OnFriendRecived.AddParam<SUIParameterDesc::eUIPT_String>("Status", "Friend status (offline, online, ingame, afk)");
		m_EventMap[eUIGE_OnFriendRecived] = m_pGameEvents->RegisterEvent(OnFriendRecived);

		// On stats recived
		SUIEventDesc OnStatsRecived("Statistic:OnStatsRecived", "Statistic:StatsRecived", "Event when client recived stats from online server");
		OnStatsRecived.AddParam<SUIParameterDesc::eUIPT_Int>("Kills", "Kills amount");
		OnStatsRecived.AddParam<SUIParameterDesc::eUIPT_Int>("Deaths", "Deaths amount");
		OnStatsRecived.AddParam<SUIParameterDesc::eUIPT_String>("KD", "Kill/death amount");
		m_EventMap[eUIGE_OnStatsRecived] = m_pGameEvents->RegisterEvent(OnStatsRecived);

		// On connection with server establishment
		SUIEventDesc OnConnectionEstablishment("System:OnConnectionEstablishment", "System:OnConnectionEstablishment", "Event when client establishment connection with online server");
		m_EventMap[eUIGE_OnConnectionEstablishment] = m_pGameEvents->RegisterEvent(OnConnectionEstablishment);

		// On global chat message recived
		SUIEventDesc OnGlobalChatMessageRecived("Chat:OnGlobalChatMessageRecived", "Chat:OnGlobalChatMessageRecived", "Event when client recived global chat message from online server");
		OnGlobalChatMessageRecived.AddParam<SUIParameterDesc::eUIPT_String>("Message", "Chat message");
		OnGlobalChatMessageRecived.AddParam<SUIParameterDesc::eUIPT_String>("From", "Message sender nickname");
		m_EventMap[eUIGE_OnGlobalChatMessageRecived] = m_pGameEvents->RegisterEvent(OnGlobalChatMessageRecived);

		// On private chat message recived
		SUIEventDesc OnPrivateChatMessageRecived("Chat:OnPrivateChatMessageRecived", "Chat:OnPrivateChatMessageRecived", "Event when client recived private chat message from online server");
		OnPrivateChatMessageRecived.AddParam<SUIParameterDesc::eUIPT_String>("Message", "Chat message");
		OnPrivateChatMessageRecived.AddParam<SUIParameterDesc::eUIPT_String>("From", "Message sender nickname");
		m_EventMap[eUIGE_OnPrivateChatMessageRecived] = m_pGameEvents->RegisterEvent(OnPrivateChatMessageRecived);

		// On friend list need update
		SUIEventDesc OnFriendListNeedUpdate("Friends:OnFriendListNeedUpdate", "Friends:OnFriendListNeedUpdate", "Event when client need update friend list");
		m_EventMap[eUIGE_OnFriendListNeedUpdate] = m_pGameEvents->RegisterEvent(OnFriendListNeedUpdate);

		// On friend invite recived
		SUIEventDesc OnFriendInviteRecived("Invites:OnFriendInviteRecived", "Invites:OnFriendInviteRecived", "Event when client recived friend invite");
		OnFriendInviteRecived.AddParam<SUIParameterDesc::eUIPT_String>("Sender", "Invite sender");
		m_EventMap[eUIGE_OnFriendInviteRecived] = m_pGameEvents->RegisterEvent(OnFriendInviteRecived);

		// On game invite recived
		SUIEventDesc OnGameInviteRecived("Invites:OnGameInviteRecived", "Invites:OnGameInviteRecived", "Event when client recived game invite");
		OnGameInviteRecived.AddParam<SUIParameterDesc::eUIPT_String>("Sender", "Invite sender");
		OnGameInviteRecived.AddParam<SUIParameterDesc::eUIPT_String>("Map", "Map name");
		OnGameInviteRecived.AddParam<SUIParameterDesc::eUIPT_String>("ServerIP", "Server ip");
		m_EventMap[eUIGE_OnGameInviteRecived] = m_pGameEvents->RegisterEvent(OnGameInviteRecived);

		// On clan invite recived
		SUIEventDesc OnClanInviteRecived("Invites:OnClanInviteRecived", "Invites:OnClanInviteRecived", "Event when client recived game invite");
		OnClanInviteRecived.AddParam<SUIParameterDesc::eUIPT_String>("Sender", "Invite sender");
		OnClanInviteRecived.AddParam<SUIParameterDesc::eUIPT_String>("ClanName", "Clan name");
		m_EventMap[eUIGE_OnClanInviteRecived] = m_pGameEvents->RegisterEvent(OnClanInviteRecived);

		// On clan invite recived
		SUIEventDesc OnFriendUpdateStatus("Friends:OnFriendUpdateStatus", "Friends:OnFriendUpdateStatus", "Event when friend update status");
		OnFriendUpdateStatus.AddParam<SUIParameterDesc::eUIPT_String>("Nickname", "Friend nickname");
		OnFriendUpdateStatus.AddParam<SUIParameterDesc::eUIPT_String>("Status", "Status");
		m_EventMap[eUIGE_OnFriendUpdateStatus] = m_pGameEvents->RegisterEvent(OnFriendUpdateStatus);

		// On matchmaking success
		SUIEventDesc OnMatchmakingSuccess("Matchmaking:OnMatchmakingSuccess", "Matchmaking:OnMatchmakingSuccess", "Event when client start work with game server");
		m_EventMap[eUIGE_OnMatchmakingSuccess] = m_pGameEvents->RegisterEvent(OnMatchmakingSuccess);

		return true;
	}

	gEnv->pLog->LogError(TITLE "Register UI events failed!");
	return false;
}

void CModuleUIEvents::SendEvent(EUIGameEvents event, const SUIArguments& args)
{
	// send the event
	if (m_pGameEvents)
	{
		m_pGameEvents->SendEvent(SUIEvent(m_EventMap[event], args));
	}
}

void CModuleUIEvents::SendEmptyEvent(EUIGameEvents event)
{
	if (m_pGameEvents)
	{
		m_pGameEvents->SendEvent(event);
	}
}