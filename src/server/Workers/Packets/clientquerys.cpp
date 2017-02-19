// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "clientquerys.h"

#include "Core/remoteserver.h"
#include "Core/tcpserver.h"

#include "Workers/Databases/dbworker.h"

#include "Tools/settings.h"
#include "Tools/scripts.h"

#include <QRegExp>

#if !defined (QT_CREATOR_FIX_COMPILE)
#include "helper.cpp"
#endif

// Error types : 0 - Login not found, 1 - Account blocked, 2 - Incorrect password, 3 - Double authorization
void ClientQuerys::onLogin(CTcpPacket &packet)
{
	if (bAuthorizated)
	{
		qDebug() << "Client alredy authorizated!";

		// Auth failed (Double authorization)
		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::LoginFail);
		m_packet.WriteInt(3);
		m_Connection->SendMessage(m_packet);

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

	if (!(pDataBase->UserExists(login)))
	{
		qDebug() << "-----------------------Login not found------------------------";
		qDebug() << "---------------------AUTHORIZATION FAILED---------------------";

		// Auth failed
		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::LoginFail);
		m_packet.WriteInt(0);
		m_Connection->SendMessage( m_packet);

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
			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::LoginFail);
			m_packet.WriteInt(1);
			m_Connection->SendMessage( m_packet);
			return;
		}

		// Check passwords
		if (password == userData->password)
		{
			if (pDataBase->ProfileExists(userData->uid))
			{
				SProfile *dbProfile = pDataBase->GetUserProfile(userData->uid);
				if (dbProfile)
				{
					bAuthorizated = true;
					m_Client->profile = dbProfile;
					m_Client->status = 1;
					pServer->UpdateClient(m_Client);

					qDebug() << "-------------------------Profile found--------------------------";
					qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

					CTcpPacket m_packet(EFireNetTcpPacketType::Result);
					m_packet.WriteResult(EFireNetTcpResult::LoginCompleteWithProfile);
					m_Connection->SendMessage(m_packet);

					return;
				}
				else
				{
					qWarning() << "Profile finded, but can't get it from database!";
					return;
				}
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				CTcpPacket m_packet(EFireNetTcpPacketType::Result);
				m_packet.WriteResult(EFireNetTcpResult::LoginComplete);
				m_Connection->SendMessage(m_packet);

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

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::LoginFail);
			m_packet.WriteInt(2);
			m_Connection->SendMessage( m_packet);
			return;
		}
	}
}

// Error types : 0 - Login alredy register, 1 - Can't create account, 2 - Double registration
void ClientQuerys::onRegister(CTcpPacket &packet)
{
	if (bRegistered)
	{
		qDebug() << "Client alredy registered!";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterFail);
		m_packet.WriteInt(2);
		m_Connection->SendMessage( m_packet);
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

	DBWorker* pDataBase = gEnv->pDBWorker;

	if (!(pDataBase->UserExists(login)))
	{
		int uid = pDataBase->GetFreeUID();

		if (uid > 0)
		{
			if (pDataBase->CreateUser(uid, login, password))
			{
				qDebug() << "---------------------REGISTRATION COMPLETE---------------------";

				bRegistered = true;

				CTcpPacket m_packet(EFireNetTcpPacketType::Result);
				m_packet.WriteResult(EFireNetTcpResult::RegisterComplete);
				m_Connection->SendMessage( m_packet);
				return;
			}
			else
			{
				qDebug() << "--------------Can't create account in database!--------------";
				qDebug() << "---------------------REGISTRATION FAILED---------------------";

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::RegisterFail);
				m_packet.WriteInt(1);
				m_Connection->SendMessage( m_packet);
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------Login alredy register or some values empty!--------";
		qDebug() << "---------------------REGISTRATION FAILED---------------------";
		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RegisterFail);
		m_packet.WriteInt(0);
		m_Connection->SendMessage( m_packet);
		return;
	}
}

// Error types : 0 - Client alredy have profile, 1 - Nickname alredy registered, 2 - Can't create profile, 3 - Double profile creation
void ClientQuerys::onCreateProfile(CTcpPacket &packet)
{
	if (bProfileCreated)
	{
		qDebug() << "Client alredy create profile!";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::ProfileCreationFail);
		m_packet.WriteInt(3);
		m_Connection->SendMessage(m_packet);

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

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::ProfileCreationFail);
		m_packet.WriteInt(0);
		m_Connection->SendMessage(m_packet);

		return;
	}

	if (!(pDataBase->NicknameExists(nickname)))
	{
		m_Client->profile->nickname = nickname;
		m_Client->profile->fileModel = fileModel;
		m_Client->profile->money = 10000; // TODO
		m_Client->profile->xp = 0;
		m_Client->profile->lvl = 0;
		m_Client->profile->items = "";
		m_Client->profile->friends = "";

		if (pDataBase->CreateProfile(m_Client->profile))
		{
			qDebug() << "---------------------CREATE PROFILE COMPLETE---------------------";

			CTcpPacket m_paket(EFireNetTcpPacketType::Result);
			m_paket.WriteResult(EFireNetTcpResult::ProfileCreationComplete);
			m_paket.WriteInt(m_Client->profile->uid);
			m_paket.WriteString(m_Client->profile->nickname.toStdString());
			m_paket.WriteString(m_Client->profile->fileModel.toStdString());
			m_paket.WriteInt(m_Client->profile->lvl);
			m_paket.WriteInt(m_Client->profile->xp);
			m_paket.WriteInt(m_Client->profile->money);
			m_paket.WriteString(m_Client->profile->items.toStdString());
			m_paket.WriteString(m_Client->profile->friends.toStdString());

			m_Connection->SendMessage(m_paket);

			bProfileCreated = true;
			m_Client->status = 1;
			pServer->UpdateClient(m_Client);

			return;
		}
		else
		{
			qDebug() << "---------------------Database return error---------------------";
			qDebug() << "---------------------CREATE PROFILE FAILED---------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::ProfileCreationFail);
			m_packet.WriteInt(2);
			m_Connection->SendMessage(m_packet);
			return;
		}
	}
	else
	{
		qDebug() << "-------------------Nickname alredy registered!-------------------";
		qDebug() << "---------------------CREATE PROFILE FAILED-----------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::ProfileCreationFail);
		m_packet.WriteInt(1);
		m_Connection->SendMessage(m_packet);
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

	if (!m_Client->profile->nickname.isEmpty())
	{
		qDebug() << "-------------------------Profile found--------------------------";
		qDebug() << "----------------------GET PROFILE COMPLETE----------------------";

		CTcpPacket profile(EFireNetTcpPacketType::Result);
		profile.WriteResult(EFireNetTcpResult::GetProfileComplete);
		profile.WriteInt(m_Client->profile->uid);
		profile.WriteString(m_Client->profile->nickname.toStdString());
		profile.WriteString(m_Client->profile->fileModel.toStdString());
		profile.WriteInt(m_Client->profile->lvl);
		profile.WriteInt(m_Client->profile->xp);
		profile.WriteInt(m_Client->profile->money);
		profile.WriteString(m_Client->profile->items.toStdString());
		profile.WriteString(m_Client->profile->friends.toStdString());
		m_Connection->SendMessage( profile);
		return;
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "----------------------GET PROFILE FAILED----------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::GetProfileFail);
		m_packet.WriteInt(1);
		m_Connection->SendMessage( m_packet);
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

	QVector<SShopItem> m_shop = gEnv->pScripts->GetShop();

	if (m_shop.size() > 0)
	{
		QStringList m_shopList;

		for (auto it = m_shop.begin(); it != m_shop.end(); ++it)
		{
			int bCanbay;
			it->canBuy ? bCanbay = 1 : bCanbay = 0;

			// name-cost-minLvl-canBuy
			m_shopList.append(it->name + "-" + 
				QString::number(it->cost) + "-" + 
				QString::number(it->minLnl) + "-" + 
			    QString::number(bCanbay));
		}

		CTcpPacket m_packet(EFireNetTcpPacketType::Result);
		m_packet.WriteResult(EFireNetTcpResult::GetShopComplete);
		m_packet.WriteString(m_shopList.join(",").toStdString());
		m_Connection->SendMessage( m_packet);
	}
	else
	{
		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::GetShopFail);
		m_packet.WriteInt(0);

		return;
	}
}

// Error types : 0 - Item alredy purchased, 1 - Player lvl < minimal lvl for buy this item, 2 - Insufficient money to buy, 3 - Item not found,
// 4 - Can't get profile,  5 - Can't update profile
void ClientQuerys::onBuyItem(CTcpPacket &packet)
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

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::BuyItemFail);
				m_packet.WriteInt(0);
				m_Connection->SendMessage( m_packet);
				return;
			}

			// Check minimal player level for buy this item
			if (m_Client->profile->lvl < item.minLnl)
			{
				qDebug() << "-----------------Profile level < minimal level----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::BuyItemFail);
				m_packet.WriteInt(1);
				m_Connection->SendMessage( m_packet);
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

					CTcpPacket profile(EFireNetTcpPacketType::Result);
					profile.WriteResult(EFireNetTcpResult::UpdateProfileComplete);
					profile.WriteInt(m_Client->profile->uid);
					profile.WriteString(m_Client->profile->nickname.toStdString());
					profile.WriteString(m_Client->profile->fileModel.toStdString());
					profile.WriteInt(m_Client->profile->lvl);
					profile.WriteInt(m_Client->profile->xp);
					profile.WriteInt(m_Client->profile->money);
					profile.WriteString(m_Client->profile->items.toStdString());
					profile.WriteString(m_Client->profile->friends.toStdString());
					m_Connection->SendMessage( profile);
					return;
				}
				else
				{
					qDebug() << "---------------------Can't update profile---------------------";
					qDebug() << "------------------------BUY ITEM FAILED-----------------------";

					CTcpPacket m_packet(EFireNetTcpPacketType::Error);
					m_packet.WriteError(EFireNetTcpError::BuyItemFail);
					m_packet.WriteInt(5);
					m_Connection->SendMessage( m_packet);
					return;
				}
			}
			else
			{
				qDebug() << "-------------------Insufficient money to buy-----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::BuyItemFail);
				m_packet.WriteInt(2);
				m_Connection->SendMessage( m_packet);
				return;
			}
		}
		else
		{
			qDebug() << "------------------------Item not found------------------------";
			qDebug() << "------------------------BUY ITEM FAILED-----------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::BuyItemFail);
			m_packet.WriteInt(3);
			m_Connection->SendMessage( m_packet);
			return;
		}
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "------------------------BUY ITEM FAILED-----------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::BuyItemFail);
		m_packet.WriteInt(4);
		m_Connection->SendMessage( m_packet);

		return;
	}
}

// Error types : 0 - Item not found in shop list, 1 - Item not found in inventory, 2 - Can't get profile, 3 - Can't update profile
void ClientQuerys::onRemoveItem(CTcpPacket &packet)
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

	if (!m_Client->profile->nickname.isEmpty())
	{
		// Search item in shop list
		SShopItem item = GetShopItemByName(itemName);
		if (item.name.isEmpty())
		{
			qDebug() << "-------------------Item not found in shop list-------------------";
			qDebug() << "------------------------REMOVE ITEM FAILED-----------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::RemoveItemFail);
			m_packet.WriteInt(0);
			m_Connection->SendMessage( m_packet);
			return;
		}

		// Check item if it is there in item list
		QStringList itemList = m_Client->profile->items.split(",");

		if (!itemList.contains(item.name))
		{
			qDebug() << "-------------------Item not found in inventory--------------------";
			qDebug() << "------------------------REMOVE ITEM FAILED-----------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::RemoveItemFail);
			m_packet.WriteInt(1);
			m_Connection->SendMessage( m_packet);
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

				CTcpPacket profile(EFireNetTcpPacketType::Result);
				profile.WriteResult(EFireNetTcpResult::RemoveItemComplete);
				profile.WriteInt(m_Client->profile->uid);
				profile.WriteString(m_Client->profile->nickname.toStdString());
				profile.WriteString(m_Client->profile->fileModel.toStdString());
				profile.WriteInt(m_Client->profile->lvl);
				profile.WriteInt(m_Client->profile->xp);
				profile.WriteInt(m_Client->profile->money);
				profile.WriteString(m_Client->profile->items.toStdString());
				profile.WriteString(m_Client->profile->friends.toStdString());
				m_Connection->SendMessage( profile);
				return;
			}
			else
			{
				qDebug() << "--------------------Can't update profile--------------------";
				qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::RemoveItemFail);
				m_packet.WriteInt(3);
				m_Connection->SendMessage( m_packet);
				return;
			}
		}
	}
	else
	{
		qDebug() << "---------------------Error get profile----------------------";
		qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RemoveItemFail);
		m_packet.WriteInt(2);
		m_Connection->SendMessage( m_packet);
		return;
	}
}

// Error types : 0 - User not found, 1 - User not online
void ClientQuerys::onInvite(CTcpPacket &packet)
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

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::SendInviteFail);
			m_packet.WriteInt(0);
			m_Connection->SendMessage( m_packet);
			return;
		}

		QSslSocket* reciverSocket = pServer->GetSocketByUid(friendUID);

		if (reciverSocket != nullptr)
		{
			// Send result to client
			CTcpPacket m_packet(EFireNetTcpPacketType::Result);
			m_packet.WriteResult(EFireNetTcpResult::SendInviteComplete);
			m_Connection->SendMessage( m_packet);

			// Send invite to user
			CTcpPacket invite(EFireNetTcpPacketType::Query);
			invite.WriteQuery(EFireNetTcpQuery::SendInvite);
			invite.WriteInt(0); // Friend invite
			invite.WriteString(m_Client->profile->nickname.toStdString()); // From
			pServer->sendMessageToClient(reciverSocket, invite);
			return;
		}
		else
		{
			qDebug() << "----------------------Reciver not online----------------------";
			qDebug() << "---------------------INVITE FRIEND FAILED---------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::SendInviteFail);
			m_packet.WriteInt(1);
			m_Connection->SendMessage( m_packet);
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

void ClientQuerys::onDeclineInvite(CTcpPacket &packet)
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
		return;
	}

	QSslSocket* reciverSocket = pServer->GetSocketByUid(friendUID);

	if (reciverSocket != nullptr)
	{
		// Send decline invite to invite sender
		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::SendInviteFail);
		pServer->sendMessageToClient(reciverSocket, m_packet);
		return;
	}
	else
	{
		qDebug() << "----------------------Reciver not online-----------------------";
		qDebug() << "---------------------DECLINE INVITE FAILED---------------------";
		return;
	}
}

// Error types : 0 - Friend alredy exist, 1 - Can't add yourself to friend, 2 - Friend not found,  3 - Can't get profile, 4 - Can't update profile
void ClientQuerys::onAddFriend(CTcpPacket &packet)
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

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::AcceptInviteFail);
				m_packet.WriteInt(0);
				m_Connection->SendMessage( m_packet);
				return;
			} 
			else if (m_Client->profile->uid == friendUID) // Block add yourself in friends
			{
				qDebug() << "----------------Can't add yourself to friends--------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::AcceptInviteFail);
				m_packet.WriteInt(1);
				m_Connection->SendMessage( m_packet);
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

				CTcpPacket profile(EFireNetTcpPacketType::Result);
				profile.WriteResult(EFireNetTcpResult::AcceptInviteComplete);
				profile.WriteInt(m_Client->profile->uid);
				profile.WriteString(m_Client->profile->nickname.toStdString());
				profile.WriteString(m_Client->profile->fileModel.toStdString());
				profile.WriteInt(m_Client->profile->lvl);
				profile.WriteInt(m_Client->profile->xp);
				profile.WriteInt(m_Client->profile->money);
				profile.WriteString(m_Client->profile->items.toStdString());
				profile.WriteString(m_Client->profile->friends.toStdString());

				m_Connection->SendMessage( profile);

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					CTcpPacket profile(EFireNetTcpPacketType::Result);
					profile.WriteResult(EFireNetTcpResult::AcceptInviteComplete);
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

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::AcceptInviteFail);
				m_packet.WriteInt(4);
				m_Connection->SendMessage( m_packet);
				return;
			}
		}
		else
		{
			qDebug() << "---------------------Error get profile---------------------";
			qDebug() << "---------------------ADD FRIEND FAILED---------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::AcceptInviteFail);
			m_packet.WriteInt(3);
			m_Connection->SendMessage( packet);
			return;
		}
	}
	else
	{
		qDebug() << "---------------------Friend not found----------------------";
		qDebug() << "---------------------ADD FRIEND FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::AcceptInviteFail);
		m_packet.WriteInt(2);
		m_Connection->SendMessage( m_packet);
		return;
	}
}

// Error types : 0 - Friend not found, 1 - Can't get profile, 2 - Can't update profile
void ClientQuerys::onRemoveFriend(CTcpPacket &packet)
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

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::RemoveFriendFail);
			m_packet.WriteInt(0);
			m_Connection->SendMessage( m_packet);
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

				CTcpPacket profile (EFireNetTcpPacketType::Result);
				profile.WriteResult(EFireNetTcpResult::RemoveFriendComplete);
				profile.WriteInt(m_Client->profile->uid);
				profile.WriteString(m_Client->profile->nickname.toStdString());
				profile.WriteString(m_Client->profile->fileModel.toStdString());
				profile.WriteInt(m_Client->profile->lvl);
				profile.WriteInt(m_Client->profile->xp);
				profile.WriteInt(m_Client->profile->money);
				profile.WriteString(m_Client->profile->items.toStdString());
				profile.WriteString(m_Client->profile->friends.toStdString());

				m_Connection->SendMessage( profile);

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					CTcpPacket profile(EFireNetTcpPacketType::Result);
					profile.WriteResult(EFireNetTcpResult::RemoveFriendComplete);
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

				CTcpPacket m_packet(EFireNetTcpPacketType::Error);
				m_packet.WriteError(EFireNetTcpError::RemoveFriendFail);
				m_packet.WriteInt(2);
				m_Connection->SendMessage( m_packet);
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------------------Error get profile-----------------------";
		qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::RemoveFriendFail);
		m_packet.WriteInt(1);
		m_Connection->SendMessage( m_packet);
		return;
	}
}

// Error types : 0 - Can't send message to yourself, 1 - Reciver not online
void ClientQuerys::onChatMessage(CTcpPacket &packet)
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

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::SendChatMsgFail);
		m_packet.WriteInt(0);
		m_Connection->SendMessage( m_packet);
		return;
	}

	if (!m_Client->profile->nickname.isEmpty() && reciver == "all")
	{
		if (gEnv->pSettings->GetVariable("bUseGlobalChat").toBool())
		{
			CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
			msg.WriteServerMessage(EFireNetTcpSMessage::GlobalChatMsg);
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
			CTcpPacket msg(EFireNetTcpPacketType::ServerMessage);
			msg.WriteServerMessage(EFireNetTcpSMessage::PrivateChatMsg);
			msg.WriteString(m_Client->profile->nickname.toStdString());
			msg.WriteString(message.toStdString());
			pServer->sendMessageToClient(reciverSocket, msg);

			return;
		}
		else
		{
			qDebug() << "-------------------Reciver not found or offline-------------------";
			qDebug() << "---------------------SEND CHAT MESSAGE FAILED---------------------";

			CTcpPacket m_packet(EFireNetTcpPacketType::Error);
			m_packet.WriteError(EFireNetTcpError::SendChatMsgFail);
			m_packet.WriteInt(1);
			m_Connection->SendMessage( m_packet);
			return;
		}
	}
}

// Error types : 0 - Not any online servers, 1 - Server not found
void ClientQuerys::onGetGameServer(CTcpPacket &packet)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't get game server without authorization!!!";
		return;
	}

	int gameServersCount = 0;
	gEnv->pRemoteServer->IsHaveAdmin() ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

	if (gameServersCount <= 0)
	{
		qDebug() << "---------------------Not any online server----------------------";
		qDebug() << "---------------------GET GAME SERVER FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::GetServerFail);
		m_packet.WriteInt(0);
		m_Connection->SendMessage( m_packet);

		return;
	}

	QString map = packet.ReadString();
	QString gamerules = packet.ReadString();
	QString serverName = packet.ReadString();

	SGameServer* pGameServer = gEnv->pRemoteServer->GetGameServer(serverName, map, gamerules);

	if (pGameServer != nullptr)
	{
		CTcpPacket gameServer(EFireNetTcpPacketType::Result);
		gameServer.WriteResult(EFireNetTcpResult::GetServerComplete);
		gameServer.WriteString(pGameServer->name.toStdString());
		gameServer.WriteString(pGameServer->ip.toStdString());
		gameServer.WriteInt(pGameServer->port);
		gameServer.WriteString(pGameServer->map.toStdString());
		gameServer.WriteString(pGameServer->gamerules.toStdString());
		gameServer.WriteInt(pGameServer->online);
		gameServer.WriteInt(pGameServer->maxPlayers);

		m_Connection->SendMessage( gameServer);

		return;
	}
	else
	{
		qDebug() << "-----------------------Server not found-------------------------";
		qDebug() << "---------------------GET GAME SERVER FAILED---------------------";

		CTcpPacket m_packet(EFireNetTcpPacketType::Error);
		m_packet.WriteError(EFireNetTcpError::GetServerFail);
		m_packet.WriteInt(1);
		m_Connection->SendMessage( m_packet);
	}
}