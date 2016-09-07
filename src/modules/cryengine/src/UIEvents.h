/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2016

*************************************************************************/

#ifndef __UICryModuleEvents_H__
#define __UICryModuleEvents_H__

#include <CrySystem\Scaleform\IFlashUI.h>

class CModuleUIEvents
{
public:
	bool RegisterUIEvents();

	// events
	enum EUIGameEvents
	{
		eUIGE_OnError,
		eUIGE_OnLoginComplete,
		eUIGE_OnRegComplete,
		eUIGE_OnProfileDataRecived,
		eUIGE_OnShopItemRecived,
		eUIGE_OnInventoryItemRecived,
		eUIGE_OnFriendRecived,
		eUIGE_OnStatsRecived,
		eUIGE_OnConnectionEstablishment,
		eUIGE_OnGlobalChatMessageRecived,
		eUIGE_OnPrivateChatMessageRecived,
		eUIGE_OnFriendListNeedUpdate,
		eUIGE_OnFriendInviteRecived,
		eUIGE_OnGameInviteRecived,
		eUIGE_OnClanInviteRecived,
		eUIGE_OnFriendUpdateStatus,
	};
	void SendEvent(EUIGameEvents event, const SUIArguments& args);
	void SendEmptyEvent(EUIGameEvents);

	CModuleUIEvents() : m_pGameEvents(NULL) {};

private:
	~CModuleUIEvents() {};

	IUIEventSystem* m_pGameEvents;
	std::map<EUIGameEvents, uint> m_EventMap;
};

#endif