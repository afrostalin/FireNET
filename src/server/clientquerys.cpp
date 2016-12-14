// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "clientquerys.h"
#include "tcpserver.h"
#include "dbworker.h"
#include "httpconnector.h"
#include "settings.h"
#include "remoteserver.h"
#include "scripts.h"

#include <QRegExp>

#if !defined (QT_CREATOR_FIX_COMPILE)
#include "helper.cpp"
#endif

// Error types : 0 - Login not found, 1 - Account blocked, 2 - Incorrect password, 3 - Double authorization
void ClientQuerys::onLogin(NetPacket &packet)
{
	if (bAuthorizated)
	{
		qDebug() << "Client alredy authorizated!";

		// Auth failed (Double authorization)
		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_auth_fail);
		m_packet.WriteInt(3);
		gEnv->pServer->sendMessageToClient(m_socket, m_packet);

		return;
	}

	QString login = packet.ReadString();
	QString password = packet.ReadString();

	if (login.isEmpty() || password.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Login = " << login << "Password = " << password;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	// Log in by HTTP
	if (gEnv->pSettings->GetVariable("bUseHttpAuth").toBool())
	{
		if (pDataBase->pHTTP->Login(login, password))
		{
			int uid = pDataBase->pHTTP->GetUID();

			SProfile *dbProfile = pDataBase->GetUserProfile(uid);

			if (dbProfile != nullptr)
			{
				bAuthorizated = true;
				m_Client->profile = dbProfile;
				m_Client->status = 1;
				pServer->UpdateClient(m_Client);

				qDebug() << "-------------------------Profile found--------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				// Auth complete
				NetPacket m_packet(net_Result);
				m_packet.WriteInt(net_result_auth_complete_with_profile);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				// Auth complete
				NetPacket m_packet(net_Result);
				m_packet.WriteInt(net_result_auth_complete);

				pServer->sendMessageToClient(m_socket, m_packet);

				bAuthorizated = true;
				m_Client->profile->uid = uid;
				m_Client->status = 0;
				pServer->UpdateClient(m_Client);

				return;
			}
		}
		else
		{
			// Auth failed
			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_auth_fail);
			m_packet.WriteInt(pDataBase->pHTTP->GetError());
			pServer->sendMessageToClient(m_socket, m_packet);

			return;
		}
	}

	// Default log in mode
	if (!(pDataBase->UserExists(login)))
	{
		qDebug() << "-----------------------Login not found------------------------";
		qDebug() << "---------------------AUTHORIZATION FAILED---------------------";

		// Auth failed
		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_auth_fail);
		m_packet.WriteInt(0);
		pServer->sendMessageToClient(m_socket, m_packet);

		return;
	}
	else
	{
		SUser *userData = pDataBase->GetUserData(login);
		if (userData == nullptr)
			return;

		// Check ban status
		if (userData->bBanStatus)
		{
			qDebug() << "-----------------------Account blocked------------------------";
			qDebug() << "---------------------AUTHORIZATION FAILED---------------------";

			// Auth failed
			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_auth_fail);
			m_packet.WriteInt(1);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}

		// Check passwords
		if (password == userData->password)
		{
			SProfile *dbProfile = pDataBase->GetUserProfile(userData->uid);

			if (dbProfile != nullptr)
			{
				bAuthorizated = true;
				m_Client->profile = dbProfile;
				m_Client->status = 1;
				pServer->UpdateClient(m_Client);

				qDebug() << "-------------------------Profile found--------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				NetPacket m_packet(net_Result);
				m_packet.WriteInt(net_result_auth_complete_with_profile);
				pServer->sendMessageToClient(m_socket, m_packet);

				return;
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				NetPacket m_packet(net_Result);
				m_packet.WriteInt(net_result_auth_complete);
				pServer->sendMessageToClient(m_socket, m_packet);

				bAuthorizated = true;
				m_Client->profile->uid = userData->uid;
				m_Client->status = 0;
				pServer->UpdateClient(m_Client);

				return;
			}

		}
		else
		{
			qDebug() << "----------------------Incorrect password----------------------";
			qDebug() << "---------------------AUTHORIZATION FAILED---------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_auth_fail);
			m_packet.WriteInt(2);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
	}
}

// Error types : 0 - Login alredy register, 1 - Can't create account, 2 - Double registration
void ClientQuerys::onRegister(NetPacket &packet)
{
	if (bRegistered)
	{
		qDebug() << "Client alredy registered!";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_register_fail);
		m_packet.WriteInt(2);
		gEnv->pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}

	QString login = packet.ReadString();
	QString password = packet.ReadString();

	if (login.isEmpty() || password.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Login = " << login << "Password = " << password;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	// Login by HTTP
	if (gEnv->pSettings->GetVariable("bUseHttpAuth").toBool())
	{
		if (pDataBase->pHTTP->Register(login, password))
		{
			qDebug() << "---------------------REGISTRATION COMPLETE---------------------";
			bRegistered = true;

			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_register_complete);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
		else
		{
			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_register_fail);
			m_packet.WriteInt(pDataBase->pHTTP->GetError());
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
	}

	// Default register
	if (!(pDataBase->UserExists(login)))
	{
		int uid = pDataBase->GetFreeUID();

		if (uid > 0)
		{
			if (pDataBase->CreateUser(uid, login, password))
			{
				qDebug() << "---------------------REGISTRATION COMPLETE---------------------";

				bRegistered = true;

				NetPacket m_packet(net_Result);
				m_packet.WriteInt(net_result_register_complete);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
			else
			{
				qDebug() << "--------------Can't create account in database!--------------";
				qDebug() << "---------------------REGISTRATION FAILED---------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_register_fail);
				m_packet.WriteInt(1);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------Login alredy register or some values empty!--------";
		qDebug() << "---------------------REGISTRATION FAILED---------------------";
		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_register_fail);
		m_packet.WriteInt(0);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}
}

// Error types : 0 - Client alredy have profile, 1 - Nickname alredy registered, 2 - Can't create profile, 3 - Double profile creation
void ClientQuerys::onCreateProfile(NetPacket &packet)
{
	if (bProfileCreated)
	{
		qDebug() << "Client alredy create profile!";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_profile_creation_fail);
		m_packet.WriteInt(3);
		gEnv->pServer->sendMessageToClient(m_socket, m_packet);

		return;
	}

	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't create profile without authorization!!! Uid = " << m_Client->profile->uid;
		return;
	}

	QString nickname = packet.ReadString();
	QString fileModel = packet.ReadString();

	if (nickname.isEmpty() || fileModel.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Nickname = " << nickname << "fileModel = " << fileModel;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	if (!(m_Client->profile->nickname.isEmpty()))
	{
		qDebug() << "------------------Client alredy have profile-------------------";
		qDebug() << "---------------------CREATE PROFILE FAILED---------------------";

		NetPacket m_paket(net_Error);
		m_paket.WriteInt(net_result_profile_creation_fail);
		m_paket.WriteInt(0);
		pServer->sendMessageToClient(m_socket, m_paket);

		return;
	}

	if (!(pDataBase->NicknameExists(nickname)))
	{
		m_Client->profile->nickname = nickname;
		m_Client->profile->fileModel = fileModel;
		m_Client->profile->money = startMoney;
		m_Client->profile->xp = 0;
		m_Client->profile->lvl = 0;
		m_Client->profile->items = "";
		m_Client->profile->friends = "";

		if (pDataBase->CreateProfile(m_Client->profile))
		{
			qDebug() << "---------------------CREATE PROFILE COMPLETE---------------------";

			NetPacket m_paket(net_Result);
			m_paket.WriteInt(net_result_profile_creation_complete);
			m_paket.WriteInt(m_Client->profile->uid);
			m_paket.WriteString(m_Client->profile->nickname.toStdString());
			m_paket.WriteString(m_Client->profile->fileModel.toStdString());
			m_paket.WriteInt(m_Client->profile->lvl);
			m_paket.WriteInt(m_Client->profile->xp);
			m_paket.WriteInt(m_Client->profile->money);
			m_paket.WriteString(m_Client->profile->items.toStdString());
			m_paket.WriteString(m_Client->profile->friends.toStdString());

			pServer->sendMessageToClient(m_socket, m_paket);

			bProfileCreated = true;
			m_Client->status = 1;
			pServer->UpdateClient(m_Client);

			return;
		}
		else
		{
			qDebug() << "---------------------Database return error---------------------";
			qDebug() << "---------------------CREATE PROFILE FAILED---------------------";

			NetPacket m_paket(net_Error);
			m_paket.WriteInt(net_result_profile_creation_fail);
			m_paket.WriteInt(2);
			pServer->sendMessageToClient(m_socket, m_paket);
			return;
		}
	}
	else
	{
		qDebug() << "-------------------Nickname alredy registered!-------------------";
		qDebug() << "---------------------CREATE PROFILE FAILED-----------------------";

		NetPacket m_paket(net_Error);
		m_paket.WriteInt(net_result_profile_creation_fail);
		m_paket.WriteInt(1);
		pServer->sendMessageToClient(m_socket, m_paket);
		return;
	}
}

// Error types : 0  - Profile not found
void ClientQuerys::onGetProfile()
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't get profile without authorization!!!";
		return;
	}

	TcpServer* pServer = gEnv->pServer;

	if (!m_Client->profile->nickname.isEmpty())
	{
		qDebug() << "-------------------------Profile found--------------------------";
		qDebug() << "----------------------GET PROFILE COMPLETE----------------------";

		NetPacket profile(net_Result);
		profile.WriteInt(net_result_get_profile_complete);
		profile.WriteInt(m_Client->profile->uid);
		profile.WriteString(m_Client->profile->nickname.toStdString());
		profile.WriteString(m_Client->profile->fileModel.toStdString());
		profile.WriteInt(m_Client->profile->lvl);
		profile.WriteInt(m_Client->profile->xp);
		profile.WriteInt(m_Client->profile->money);
		profile.WriteString(m_Client->profile->items.toStdString());
		profile.WriteString(m_Client->profile->friends.toStdString());
		pServer->sendMessageToClient(m_socket, profile);
		return;
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "----------------------GET PROFILE FAILED----------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_get_profile_fail);
		m_packet.WriteInt(1);
		pServer->sendMessageToClient(m_socket, m_packet);
	}
}

// Error types : 0 - Can't get shop from shop.xml
void ClientQuerys::onGetShopItems()
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't get shop without authorization!!!";
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	QVector<SShopItem> m_shop = gEnv->pScripts->GetShop();

	if (m_shop.size() > 0)
	{
		QStringList m_shopList;

		for (auto it = m_shop.begin(); it != m_shop.end(); ++it)
		{
			// name-cost-minLvl-canBuy
			m_shopList.append(it->name + "-" + 
				QString::number(it->cost) + "-" + 
				QString::number(it->minLnl) + "-" + 
			    QString::number(it->canBuy));
		}

		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_get_shop_complete);
		m_packet.WriteString(m_shopList.join(",").toStdString());
		pServer->sendMessageToClient(m_socket, m_packet);
	}
	else
	{
		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_get_shop_fail);
		m_packet.WriteInt(0);

		return;
	}
}

// Error types : 0 - Item alredy purchased, 1 - Player lvl < minimal lvl for buy this item, 2 - Insufficient money to buy, 3 - Item not found,
// 4 - Can't get profile,  5 - Can't update profile
void ClientQuerys::onBuyItem(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't buy item without authorization!!!";
		return;
	}

	QString itemName = packet.ReadString();

	if (itemName.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Item = " << itemName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	SShopItem item = GetShopItemByName(itemName);

	if (!m_Client->profile->nickname.isEmpty())
	{
		if (!item.name.isEmpty())
		{
			QStringList itemList = m_Client->profile->items.split(",");

			// Check if it is there in inventory
			if (itemList.contains(item.name))
			{
				qDebug() << "------------------This item alredy purchased------------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_buy_item_fail);
				m_packet.WriteInt(0);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}

			// Check minimal player level for buy this item
			if (m_Client->profile->lvl < item.minLnl)
			{
				qDebug() << "-----------------Profile level < minimal level----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_buy_item_fail);
				m_packet.WriteInt(1);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}

			if (m_Client->profile->money - item.cost >= 0)
			{
				qDebug() << "Client can buy item" << item.name;

				// Add item and update money
				m_Client->profile->money = m_Client->profile->money - item.cost;
				itemList.append(item.name);
				m_Client->profile->items = itemList.join(",");

				// Update profile
				if (UpdateProfile(m_Client->profile))
				{
					qDebug() << "----------------------Profile updated----------------------";
					qDebug() << "---------------------BUI ITEM COMPLETE---------------------";

					NetPacket profile(net_Result);
					profile.WriteInt(net_result_buy_item_complete);
					profile.WriteInt(m_Client->profile->uid);
					profile.WriteString(m_Client->profile->nickname.toStdString());
					profile.WriteString(m_Client->profile->fileModel.toStdString());
					profile.WriteInt(m_Client->profile->lvl);
					profile.WriteInt(m_Client->profile->xp);
					profile.WriteInt(m_Client->profile->money);
					profile.WriteString(m_Client->profile->items.toStdString());
					profile.WriteString(m_Client->profile->friends.toStdString());
					pServer->sendMessageToClient(m_socket, profile);
					return;
				}
				else
				{
					qDebug() << "---------------------Can't update profile---------------------";
					qDebug() << "------------------------BUY ITEM FAILED-----------------------";

					NetPacket m_packet(net_Error);
					m_packet.WriteInt(net_result_buy_item_fail);
					m_packet.WriteInt(5);
					pServer->sendMessageToClient(m_socket, m_packet);
					return;
				}
			}
			else
			{
				qDebug() << "-------------------Insufficient money to buy-----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_buy_item_fail);
				m_packet.WriteInt(2);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
		}
		else
		{
			qDebug() << "------------------------Item not found------------------------";
			qDebug() << "------------------------BUY ITEM FAILED-----------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_buy_item_fail);
			m_packet.WriteInt(3);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "------------------------BUY ITEM FAILED-----------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_buy_item_fail);
		m_packet.WriteInt(4);
		pServer->sendMessageToClient(m_socket, m_packet);

		return;
	}
}

// Error types : 0 - Item not found in shop list, 1 - Item not found in inventory, 2 - Can't get profile, 3 - Can't update profile
void ClientQuerys::onRemoveItem(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't remove item without authorization!!!";
		return;
	}

	QString itemName = packet.ReadString();

	if (itemName.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Item = " << itemName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;

	if (!m_Client->profile->nickname.isEmpty())
	{
		// Search item in shop list
		SShopItem item = GetShopItemByName(itemName);
		if (item.name.isEmpty())
		{
			qDebug() << "-------------------Item not found in shop list-------------------";
			qDebug() << "------------------------REMOVE ITEM FAILED-----------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_remove_item_fail);
			m_packet.WriteInt(0);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}

		// Check item if it is there in item list
		QStringList itemList = m_Client->profile->items.split(",");

		if (!itemList.contains(item.name))
		{
			qDebug() << "-------------------Item not found in inventory--------------------";
			qDebug() << "------------------------REMOVE ITEM FAILED-----------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_remove_item_fail);
			m_packet.WriteInt(1);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
		else
		{
			// Remove item
			bool result = itemList.removeOne(item.name);
			m_Client->profile->items = itemList.join(",");

			// Update profile
			if (result && UpdateProfile(m_Client->profile))
			{
				qDebug() << "-----------------------Profile updated------------------------";
				qDebug() << "---------------------REMOVE ITEM COMPLETE---------------------";

				NetPacket profile(net_Result);
				profile.WriteInt(net_result_remove_item_complete);
				profile.WriteInt(m_Client->profile->uid);
				profile.WriteString(m_Client->profile->nickname.toStdString());
				profile.WriteString(m_Client->profile->fileModel.toStdString());
				profile.WriteInt(m_Client->profile->lvl);
				profile.WriteInt(m_Client->profile->xp);
				profile.WriteInt(m_Client->profile->money);
				profile.WriteString(m_Client->profile->items.toStdString());
				profile.WriteString(m_Client->profile->friends.toStdString());
				pServer->sendMessageToClient(m_socket, profile);
				return;
			}
			else
			{
				qDebug() << "--------------------Can't update profile--------------------";
				qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_remove_item_fail);
				m_packet.WriteInt(3);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
		}
	}
	else
	{
		qDebug() << "---------------------Error get profile----------------------";
		qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remove_item_fail);
		m_packet.WriteInt(2);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}
}

// Error types : 0 - User not found, 1 - User not online
void ClientQuerys::onInvite(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't send invite without authorization!!!";
		return;
	}

	QString inviteType = packet.ReadString();
	QString reciver = packet.ReadString();

	if (m_Client->profile->nickname.isEmpty() || reciver.isEmpty() || inviteType.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Invite type  = " << inviteType << "Client = " << m_Client->profile->nickname << "Reciver = " << reciver;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	// Friend invite
	if (inviteType == "friend_invite")
	{
		int friendUID = pDataBase->GetUIDbyNick(reciver);

		if (!pDataBase->ProfileExists(friendUID))
		{
			qDebug() << "------------------------User not found------------------------";
			qDebug() << "---------------------INVITE FRIEND FAILED---------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_send_invite_fail);
			m_packet.WriteInt(0);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}

		QSslSocket* reciverSocket = pServer->GetSocketByUid(friendUID);

		if (reciverSocket != nullptr)
		{
			// Send result to client
			NetPacket m_packet(net_Result);
			m_packet.WriteInt(net_result_send_invite_complete);
			pServer->sendMessageToClient(m_socket, m_packet);

			// Send invite to user
			NetPacket invite(net_Query);
			invite.WriteInt(net_query_send_invite);
			invite.WriteInt(0); // Friend invite
			invite.WriteString(m_Client->profile->nickname.toStdString()); // From
			pServer->sendMessageToClient(reciverSocket, invite);
			return;
		}
		else
		{
			qDebug() << "----------------------Reciver not online----------------------";
			qDebug() << "---------------------INVITE FRIEND FAILED---------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_send_invite_fail);
			m_packet.WriteInt(1);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
	}

	// Game invite
	if (inviteType == "game_invite")
	{
		// do smth
	}

	// Clan invite
	if (inviteType == "clan_invite")
	{
		// do smth
	}
}

// Error types : 0 - User not found, 1 - User not online
void ClientQuerys::onDeclineInvite(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't decline invite without authorization!!!";
		return;
	}

	QString reciver = packet.ReadString();

	if (m_Client->profile->nickname.isEmpty() || reciver.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Client = " << m_Client->profile->nickname << "Reciver = " << reciver;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	int friendUID = pDataBase->GetUIDbyNick(reciver);

	if (!pDataBase->ProfileExists(friendUID))
	{
		qDebug() << "------------------------User not found-------------------------";
		qDebug() << "---------------------DECLINE INVITE FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_decline_invite_fail);
		m_packet.WriteInt(1);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}

	QSslSocket* reciverSocket = pServer->GetSocketByUid(friendUID);

	if (reciverSocket != nullptr)
	{
		// Send decline invite to invite sender
		NetPacket m_packet(net_Result);
		m_packet.WriteInt(net_result_send_invite_fail);
		pServer->sendMessageToClient(reciverSocket, m_packet);
		return;
	}
	else
	{
		qDebug() << "----------------------Reciver not online-----------------------";
		qDebug() << "---------------------DECLINE INVITE FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_decline_invite_fail);
		m_packet.WriteInt(1);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}
}

// Error types : 0 - Friend alredy exist, 1 - Can't add yourself to friend, 2 - Friend not found,  3 - Can't get profile, 4 - Can't update profile
void ClientQuerys::onAddFriend(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't add friend without authorization!!!";
		return;
	}

	QString friendName = packet.ReadString();

	if (friendName.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Friend = " << friendName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	int friendUID = pDataBase->GetUIDbyNick(friendName);

	if (pDataBase->ProfileExists(friendUID))
	{
		SProfile *friendProfile = pDataBase->GetUserProfile(friendUID);

		if (!m_Client->profile->nickname.isEmpty() && friendProfile != nullptr)
		{
			QStringList friendList = m_Client->profile->friends.split(",");
			QStringList friendFriendList = friendProfile->friends.split(",");

			// Check friend is there in friends list
			if (friendList.contains(friendProfile->nickname))
			{
				qDebug() << "--------------------This friend alredy added--------------------";
				qDebug() << "------------------------ADD FRIEND FAILED-----------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_add_friend_fail);
				m_packet.WriteInt(0);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			} else if (m_Client->profile->uid == friendUID) // Block add yourself in friends
			{
				qDebug() << "----------------Can't add yourself to friends--------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_add_friend_fail);
				m_packet.WriteInt(1);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}

			// Update friend list
			friendList.append(QString::number(friendProfile->uid));
			friendFriendList.append(QString::number(m_Client->profile->uid));
			m_Client->profile->friends = friendList.join(",");
			friendProfile->friends = friendFriendList.join(",");

			QSslSocket* friendSocket = pServer->GetSocketByUid(friendUID);

			if (UpdateProfile(m_Client->profile) && UpdateProfile(friendProfile))
			{
				qDebug() << "-----------------------Profile updated-----------------------";
				qDebug() << "---------------------ADD FRIEND COMPLETE---------------------";

				NetPacket profile(net_Result);
				profile.WriteInt(net_result_add_friend_complete);
				profile.WriteInt(m_Client->profile->uid);
				profile.WriteString(m_Client->profile->nickname.toStdString());
				profile.WriteString(m_Client->profile->fileModel.toStdString());
				profile.WriteInt(m_Client->profile->lvl);
				profile.WriteInt(m_Client->profile->xp);
				profile.WriteInt(m_Client->profile->money);
				profile.WriteString(m_Client->profile->items.toStdString());
				profile.WriteString(m_Client->profile->friends.toStdString());

				pServer->sendMessageToClient(m_socket, profile);

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					NetPacket profile(net_Result);
					profile.WriteInt(net_result_add_friend_complete);
					profile.WriteInt(m_Client->profile->uid);
					profile.WriteString(m_Client->profile->nickname.toStdString());
					profile.WriteString(m_Client->profile->fileModel.toStdString());
					profile.WriteInt(m_Client->profile->lvl);
					profile.WriteInt(m_Client->profile->xp);
					profile.WriteInt(m_Client->profile->money);
					profile.WriteString(m_Client->profile->items.toStdString());
					profile.WriteString(m_Client->profile->friends.toStdString());

					pServer->sendMessageToClient(friendSocket, profile);
				}

				return;
			}
			else
			{
				qDebug() << "-------------------Can't update profile--------------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_add_friend_fail);
				m_packet.WriteInt(4);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
		}
		else
		{
			qDebug() << "---------------------Error get profile---------------------";
			qDebug() << "---------------------ADD FRIEND FAILED---------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_add_friend_fail);
			m_packet.WriteInt(3);
			pServer->sendMessageToClient(m_socket, packet);
			return;
		}
	}
	else
	{
		qDebug() << "---------------------Friend not found----------------------";
		qDebug() << "---------------------ADD FRIEND FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_add_friend_fail);
		m_packet.WriteInt(2);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}
}

// Error types : 0 - Friend not found, 1 - Can't get profile, 2 - Can't update profile
void ClientQuerys::onRemoveFriend(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't remove friend without authorization!!!";
		return;
	}

	QString friendName = packet.ReadString();

	if (friendName.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Friend = " << friendName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	if (!m_Client->profile->nickname.isEmpty())
	{
		SProfile *friendProfile = pDataBase->GetUserProfile(pDataBase->GetUIDbyNick(friendName));
		QStringList friendList = m_Client->profile->friends.split(",");

		// Check friend is there in friends list
		if (!friendProfile || friendList.contains(QString::number(friendProfile->uid)))
		{
			qDebug() << "--------------------------Friend not found-------------------------";
			qDebug() << "------------------------REMOVE FRIEND FAILED-----------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_remove_friend_fail);
			m_packet.WriteInt(0);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
		else
		{
			QStringList friendFriendList = friendProfile->friends.split(",");

			// Delete friend from client's profile
			friendList.removeOne(QString::number(friendProfile->uid));
			m_Client->profile->friends = friendList.join(",");
			// Delete client from friend's profile
			friendFriendList.removeOne(QString::number(m_Client->profile->uid));
			friendProfile->friends = friendFriendList.join(",");

			QSslSocket* friendSocket = pServer->GetSocketByUid(friendProfile->uid);

			if (UpdateProfile(m_Client->profile) && UpdateProfile(friendProfile))
			{
				qDebug() << "------------------------Profile updated-------------------------";
				qDebug() << "---------------------REMOVE FRIEND COMPLETE---------------------";

				NetPacket profile (net_Result);
				profile.WriteInt(net_result_add_friend_complete);
				profile.WriteInt(m_Client->profile->uid);
				profile.WriteString(m_Client->profile->nickname.toStdString());
				profile.WriteString(m_Client->profile->fileModel.toStdString());
				profile.WriteInt(m_Client->profile->lvl);
				profile.WriteInt(m_Client->profile->xp);
				profile.WriteInt(m_Client->profile->money);
				profile.WriteString(m_Client->profile->items.toStdString());
				profile.WriteString(m_Client->profile->friends.toStdString());

				pServer->sendMessageToClient(m_socket, profile);

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					NetPacket profile(net_Result);
					profile.WriteInt(net_result_add_friend_complete);
					profile.WriteInt(m_Client->profile->uid);
					profile.WriteString(m_Client->profile->nickname.toStdString());
					profile.WriteString(m_Client->profile->fileModel.toStdString());
					profile.WriteInt(m_Client->profile->lvl);
					profile.WriteInt(m_Client->profile->xp);
					profile.WriteInt(m_Client->profile->money);
					profile.WriteString(m_Client->profile->items.toStdString());
					profile.WriteString(m_Client->profile->friends.toStdString());

					pServer->sendMessageToClient(friendSocket, profile);
				}

				return;
			}
			else
			{
				qDebug() << "---------------------Can't update profile---------------------";
				qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

				NetPacket m_packet(net_Error);
				m_packet.WriteInt(net_result_remove_friend_fail);
				m_packet.WriteInt(2);
				pServer->sendMessageToClient(m_socket, m_packet);
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------------------Error get profile-----------------------";
		qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_remove_friend_fail);
		m_packet.WriteInt(1);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}
}

// Error types : 0 - Can't send message to yourself, 1 - Reciver not online
void ClientQuerys::onChatMessage(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't remove friend without authorization!!!";
		return;
	}

	QString message = packet.ReadString();
	QString reciver = packet.ReadString();

	if (message.isEmpty() || reciver.isEmpty())
	{
		qDebug() << "Wrong packet data! Some values empty!";
		qDebug() << "Message = " << message << "Reciver = " << reciver;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	if (reciver == m_Client->profile->nickname)
	{
		qDebug() << "--------------Client cannot send message to himself---------------";
		qDebug() << "---------------------SEND CHAT MESSAGE FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_send_chat_msg_fail);
		m_packet.WriteInt(0);
		pServer->sendMessageToClient(m_socket, m_packet);
		return;
	}

	if (!m_Client->profile->nickname.isEmpty() && reciver == "all")
	{
		if (gEnv->pSettings->GetVariable("bUseGlobalChat").toBool())
		{
			NetPacket msg(net_Server);
			msg.WriteInt(net_server_global_chat_msg);
			msg.WriteString(m_Client->profile->nickname.toStdString());
			msg.WriteString(message.toStdString());
			pServer->sendGlobalMessage(msg);

			return;
		}
		else
		{
			qWarning() << "Client send message to global chat, but global chat now disabled, see server.cfg";
			return;
		}	
	}
	else
	{
		int reciverUID = pDataBase->GetUIDbyNick(reciver);

		QSslSocket* reciverSocket = pServer->GetSocketByUid(reciverUID);

		if (reciverSocket != nullptr)
		{
			NetPacket msg(net_Server);
			msg.WriteInt(net_server_private_chat_msg);
			msg.WriteString(m_Client->profile->nickname.toStdString());
			msg.WriteString(message.toStdString());
			pServer->sendMessageToClient(reciverSocket, msg);

			return;
		}
		else
		{
			qDebug() << "-------------------Reciver not found or offline-------------------";
			qDebug() << "---------------------SEND CHAT MESSAGE FAILED---------------------";

			NetPacket m_packet(net_Error);
			m_packet.WriteInt(net_result_send_chat_msg_fail);
			m_packet.WriteInt(1);
			pServer->sendMessageToClient(m_socket, m_packet);
			return;
		}
	}
}

// Error types : 0 - Not any online servers, 1 - Server not found
void ClientQuerys::onGetGameServer(NetPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't get game server without authorization!!!";
		return;
	}
	TcpServer* pServer = gEnv->pServer;

	int gameServersCount = 0;
	gEnv->pRemoteServer->bHaveAdmin ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

	if (gameServersCount <= 0)
	{
		qDebug() << "---------------------Not any online server----------------------";
		qDebug() << "---------------------GET GAME SERVER FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_get_server_fail);
		m_packet.WriteInt(0);
		pServer->sendMessageToClient(m_socket, m_packet);

		return;
	}

	QString map = packet.ReadString();
	QString gamerules = packet.ReadString();
	QString serverName = packet.ReadString();

	SGameServer* pGameServer = gEnv->pRemoteServer->GetGameServer(serverName, map, gamerules);

	if (pGameServer != nullptr)
	{
		NetPacket gameServer(net_Result);
		gameServer.WriteInt(net_result_get_server_complete);
		gameServer.WriteString(pGameServer->name.toStdString());
		gameServer.WriteString(pGameServer->ip.toStdString());
		gameServer.WriteInt(pGameServer->port);
		gameServer.WriteString(pGameServer->map.toStdString());
		gameServer.WriteString(pGameServer->gamerules.toStdString());
		gameServer.WriteInt(pGameServer->online);
		gameServer.WriteInt(pGameServer->maxPlayers);

		pServer->sendMessageToClient(m_socket, gameServer);

		return;
	}
	else
	{
		qDebug() << "-----------------------Server not found-------------------------";
		qDebug() << "---------------------GET GAME SERVER FAILED---------------------";

		NetPacket m_packet(net_Error);
		m_packet.WriteInt(net_result_get_server_fail);
		m_packet.WriteInt(1);
		pServer->sendMessageToClient(m_socket, m_packet);
	}
}