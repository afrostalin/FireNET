// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

// Item types for shop/loadout items
enum EFireNetItemType
{
	EItem_None,
	EItem_Weapon,
	EItem_Booster,
	EItem_Other,
};

// Invite types
enum EFireNetInviteType
{
	EInvite_AddToFriends,
	EInvite_PlayInGame,
	EInvite_AddToClan,
};

// Chat message types
enum EFireNetChatMsgType
{
	EMsg_GlobalChat,
	EMsg_PrivateChat,
	EMsg_ClanChat,
};

// Player status
enum EFireNetPlayerStatus
{
	EStatus_Online,
	EStatus_InGame,
	EStatus_InRoom,
	EStatus_AFK,
	EStatus_Offline,
};

// Item structure
struct SFireNetItem
{
	std::string name;
	EFireNetItemType type;
	int cost;
	int minLvlForBuy;
};

// Friend stucture
struct SFireNetFriend
{
	int uid;
	std::string name;
	EFireNetPlayerStatus status;
};

// Player statistic
struct SFireNetStats
{
	int kills;
	int deaths;
	double kd;
};

// Player profile
struct SFireNetProfile
{
	int uid;
	std::string nickname;
	std::string fileModel;
	int lvl;
	int xp;
	int money;
	std::vector<SFireNetItem> items;
	std::vector<SFireNetFriend> friends;
	std::vector<SFireNetStats> stats;
};

// Game server structure
struct SFireNetGameServer
{
	std::string name;
	const char* ip;
	int port;
	std::string map;
	std::string gamerules;
	int online;
	int maxPlayers;
};