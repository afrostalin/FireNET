// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include "Global.h"
#include <QXmlStreamReader>

void CXmlWorker::ReadXmlData(const char* data)
{
	CryLog(TITLE  "Parsing message data...");

	QXmlStreamReader xml(data);

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		QXmlStreamReader::TokenType token = xml.readNext();
		if (token == QXmlStreamReader::StartDocument)
			continue;
		if (token == QXmlStreamReader::StartElement)
		{
			if (xml.name() == "result")
			{
				QXmlStreamAttributes attributes = xml.attributes();
				string type = attributes.value("type").toString().toStdString().c_str();

				CryLog(TITLE  "Message type = %s", type);

				if (type == "profile_data")
					onProfileDataRecived(data);
#ifndef DEDICATED_SERVER
				if (type == "auth_complete")
					onAuthComplete(data);
				if (type == "reg_complete")
					onRegisterComplete(data);

				if (type == "game_server_data")
					onGameServerDataRecived(data);
#endif
			}

			if (xml.name() == "error")
				onError(data);
			if (xml.name() == "server")
				onServerMessageRecived(data);
#ifndef DEDICATED_SERVER
			if (xml.name() == "shop")
				onShopItemsRecived(data);
			if (xml.name() == "chat")
				onChatMessageRecived(data);
			if (xml.name() == "invite")
				onInvite(data);
#endif

			return;
		}
	}
}

void CXmlWorker::onError(const char* data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "error");
	QString type = attributes.value("type").toString();
	int reason = attributes.value("reason").toInt();

	CryLog(TITLE "Error type = '%s' reason = '%d'", type.toStdString().c_str(), reason);

	if (!type.isEmpty())
	{
#ifndef DEDICATED_SERVER

		if (type == "auth_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@loginNotFound");
			if (reason == 1)
				args.AddArgument("@accountBlocked");
			if (reason == 2)
				args.AddArgument("@incorrectPassword");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "reg_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@loginAlredyRegistered");
			if (reason == 1)
				args.AddArgument("@serverError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "create_profile_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@serverError");
			if (reason == 1)
				args.AddArgument("@nicknameAlredyRegister");
			if (reason == 2)
				args.AddArgument("@profileAlredyCreated");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "get_profile_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@serverError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);

			return;
		}

		if (type == "get_shop_items_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@serverError");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);

			return;
		}

		if (type == "buy_item_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@serverError");
			if (reason == 1)
				args.AddArgument("@itemNotFinded");
			if (reason == 2)
				args.AddArgument("@insufficientMoney");
			if (reason == 3)
				args.AddArgument("@serverError");
			if (reason == 4)
				args.AddArgument("@itemAlredyPurchased");
			if (reason == 5)
				args.AddArgument("@lowProfileLevel");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "remove_item_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@serverError");
			if (reason == 1)
				args.AddArgument("@serverError");
			if (reason == 2)
				args.AddArgument("@serverError");
			if (reason == 3)
				args.AddArgument("@itemNotFound");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "invite_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@userNotFound");
			if (reason == 1)
				args.AddArgument("@userNotOnline");
			if (reason == 2)
				args.AddArgument("@userDeclineInvite");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "add_friend_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@friendNotFound");
			if (reason == 1)
				args.AddArgument("@serverError");
			if (reason == 2)
				args.AddArgument("@serverError");
			if (reason == 3)
				args.AddArgument("@cantAddYourselfInFriends");
			if (reason == 4)
				args.AddArgument("@friendAlredyAdded");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "remove_friend_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@serverError");
			if (reason == 1)
				args.AddArgument("@serverError");
			if (reason == 2)
				args.AddArgument("@friendNotFound");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}

		if (type == "get_game_server_failed")
		{
			SUIArguments args;

			if (reason == 0)
				args.AddArgument("@noAnyServerOnline");
			if (reason == 1)
				args.AddArgument("@serverNotFound");

			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);

			return;
		}
#endif
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "Some attributes empty. See log");
	}
}

#ifndef DEDICATED_SERVER

void CXmlWorker::onRegisterComplete(const char* data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "data");
	int uid = attributes.value("uid").toInt();

	if (uid > 0)
	{
		gModuleEnv->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnRegComplete);
		CryLog(TITLE  "Register complete");
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE "Register failed");
	}
}

void CXmlWorker::onAuthComplete(const char* data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "data");
	int uid = attributes.value("uid").toInt();

	if (uid > 0)
	{
		gModuleEnv->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnLoginComplete);
		CryLog(TITLE  "Authorization complete");
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_WARNING, TITLE  "Authorization failed");
	}
}
#endif

void CXmlWorker::onProfileDataRecived(const char*data)
{
#ifndef DEDICATED_SERVER
	if (!gModuleEnv->m_profile)
		gModuleEnv->m_profile = new FireNET::SProfile;

	FireNET::SProfile* m_profile = gModuleEnv->m_profile;

	QXmlStreamReader xml(data);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "profile")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			if (!attributes.isEmpty())
			{
				int uid = attributes.value("uid").toInt();
				string nickname = attributes.value("nickname").toString().toStdString().c_str();
				string fileModel = attributes.value("fileModel").toString().toStdString().c_str();
				int money = attributes.value("money").toInt();
				int xp = attributes.value("xp").toInt();
				int lvl = attributes.value("lvl").toInt();

				m_profile->uid = uid;
				m_profile->nickname = nickname;
				m_profile->fileModel = fileModel;
				m_profile->money = money;
				m_profile->xp = xp;
				m_profile->lvl = lvl;

				SUIArguments args;
				args.AddArgument(nickname);
				args.AddArgument(fileModel);
				args.AddArgument(lvl);
				args.AddArgument(xp);
				args.AddArgument(money);
				gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnProfileDataRecived, args);

				CryLog(TITLE "Recived profile data. Uid = %d, Nickname = %s, FileModel = %s, Lvl = %d, XP = %d, Money = %d", uid, nickname, fileModel, lvl, xp, money);
			}
		}

		if (xml.name() == "item")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			if (!attributes.isEmpty())
			{
				string name = attributes.value("name").toString().toStdString().c_str();
				string icon = attributes.value("icon").toString().toStdString().c_str();
				string description = attributes.value("description").toString().toStdString().c_str();

				SItem item;
				item.name = name;
				item.icon = icon;
				item.description = description;

//				m_profile->items = m_profile->items + "<item name='" + name.toStdString().c_str() + "' icon='" + icon.toStdString().c_str() + "' description='" + description.toStdString().c_str() + "'/>";

				SUIArguments args;
				args.AddArgument(name);
				args.AddArgument(icon);
				args.AddArgument(description);

				gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnInventoryItemRecived, args);

				CryLog(TITLE "Recived inventory item. Name = %s, Icon = %s, Decription = %s", name, icon, description);
			}
		}

		if (xml.name() == "friend")
		{

			QXmlStreamAttributes attributes = xml.attributes();
			if (!attributes.isEmpty())
			{
				string friendName = attributes.value("name").toString().toStdString().c_str();
				int friendUid = attributes.value("uid").toInt();

				SFriend m_friend;
				m_friend.nickname = friendName;
				m_friend.uid = friendUid;
				m_friend.status = 0;

//				m_profile->friends = m_profile->friends + "<friend name='" + friendName + "' uid='" + friendUid + "'/>";

				CryLog(TITLE "Recived friend. Name = %s, uid = %d", friendName, friendUid);
			}
		}
	}
#else
	FireNET::SProfile m_profile;

	QXmlStreamReader xml(data);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "profile")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			if (!attributes.isEmpty())
			{
				m_profile.uid = attributes.value("uid").toInt();
				m_profile.nickname = attributes.value("nickname").toString().toStdString().c_str();
				m_profile.fileModel = attributes.value("fileModel").toString().toStdString().c_str();
				m_profile.money = attributes.value("money").toInt();
				m_profile.xp = attributes.value("xp").toInt();
				m_profile.lvl = attributes.value("lvl").toInt();
			}
		}

		if (xml.name() == "item")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			if (!attributes.isEmpty())
			{
				string name = attributes.value("name").toString().toStdString().c_str();
				string icon = attributes.value("icon").toString().toStdString().c_str();
				string description = attributes.value("description").toString().toStdString().c_str();

				SItem item;
				item.name = name;
				item.icon = icon;
				item.description = description;

			//	m_profile.items = m_profile.items + "<item name='" + name + "' icon='" + icon + "' description='" + description + "'/>";
			}
		}
	}

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

void CXmlWorker::onShopItemsRecived(const char* data)
{
	QXmlStreamReader xml(data);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "item")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			if (!attributes.isEmpty())
			{
				string name = attributes.value("name").toString().toStdString().c_str();
				string icon = attributes.value("icon").toString().toStdString().c_str();
				string description = attributes.value("description").toString().toStdString().c_str();
				int cost = attributes.value("cost").toInt();
				int minLvl = attributes.value("minLvl").toInt();

				SUIArguments args;
				args.AddArgument(name);
				args.AddArgument(icon);
				args.AddArgument(description);
				args.AddArgument(cost);
				args.AddArgument(minLvl);

				gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnShopItemRecived, args);

				CryLog(TITLE "Recived shop item. Name = %s, Icon = %s, Description = %s, Cost = %d, MinLvl = %d", name, icon, description, cost, minLvl);
				return;
			}
		}
	}
}

void CXmlWorker::onChatMessageRecived(const char * data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "message");
	if (!attributes.isEmpty())
	{
		QString type = attributes.value("type").toString();
		QString message = attributes.value("message").toString();
		QString from = attributes.value("from").toString();

		if (type == "global")
		{
			QString complete = message.replace("#comma#", ",");

			SUIArguments args;
			args.AddArgument(complete.toStdString().c_str());
			args.AddArgument(from.toStdString().c_str());
			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnGlobalChatMessageRecived, args);

			CryLog(TITLE  "Recived global chat message (%s) from (%s)", message.toStdString().c_str(), from.toStdString().c_str());
			return;
		}

		if (type == "private")
		{
			QString complete = message.replace("#comma#", ",");

			SUIArguments args;
			args.AddArgument(complete.toStdString().c_str());
			args.AddArgument(from.toStdString().c_str());
			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnPrivateChatMessageRecived, args);

			CryLog(TITLE  "Recived private chat message (%s) from (%s)", message.toStdString().c_str(), from.toStdString().c_str());
			return;
		}

	}
}

void CXmlWorker::onInvite(const char * data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "invite");
	QString type = attributes.value("type").toString();
	QString from = attributes.value("from").toString();

	CryLog(TITLE  "Invite type = '%s' From = '%s'", type.toStdString().c_str(), from.toStdString().c_str());

	if (!type.isEmpty() && !from.isEmpty())
	{
		if (type == "friend_invite")
		{
			SUIArguments args;
			args.AddArgument(from.toStdString().c_str());
			gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnFriendInviteRecived, args);

			CryLog(TITLE  "Recived friend invite from (%s)", from.toStdString().c_str());

			return;
		}
	}
}

void CXmlWorker::onGameServerDataRecived(const char * data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "data");

	QString serverName = attributes.value("name").toString();
	QString serverIp = attributes.value("ip").toString();
	int serverPort = attributes.value("port").toInt();
	QString mapName = attributes.value("map").toString();
	QString gamerules = attributes.value("gamerules").toString();
	int online = attributes.value("online").toInt();
	int maxPlayers = attributes.value("maxPlayers").toInt();

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

void CXmlWorker::onServerMessageRecived(const char * data)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(data, "data");
	string type = attributes.value("type").toString().toStdString().c_str();
	string m_value = attributes.value("value").toString().toStdString().c_str();

	CryLog(TITLE  "Server message recived. Type = %s, Value = %s", type, m_value);

#ifndef DEDICATED_SERVER
	if (type == "message")
	{
		CryLog(TITLE "SERVER MESSAGE : %s", m_value);

		SUIArguments args;
		args.AddArgument(m_value);

		gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerMessageRecive, args);
	}
#endif

	if (type == "command")
	{
		CryLog(TITLE "Execute server command");
		gEnv->pConsole->ExecuteString(m_value);
	}
}

QXmlStreamAttributes CXmlWorker::GetAttributesFromArray(const char* data, const char* name)
{
	QXmlStreamReader xml(data);
	QXmlStreamAttributes attributes;

	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == name)
		{
			return xml.attributes();
		}
	}

	return attributes;
}


