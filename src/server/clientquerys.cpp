// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "clientquerys.h"
#include "tcpserver.h"
#include "dbworker.h"
#include "httpconnector.h"
#include "settings.h"

#include <QRegExp>

#if !defined (QT_CREATOR_FIX_COMPILE)
#include "helper.cpp"
#endif

void ClientQuerys::onLogin(QByteArray &bytes)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);

	QString login = attributes.value("login").toString();
	QString password = attributes.value("password").toString();

	if (login.isEmpty() || password.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
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
				m_Client->profile = dbProfile;
				m_Client->status = 1;
				pServer->UpdateClient(m_Client);

				qDebug() << "-------------------------Profile found--------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				QString result = "<result type='auth_complete'><data uid='" + QString::number(m_Client->profile->uid) + "'/></result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				result.clear();
				result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				QString result = "<result type='auth_complete'><data uid='" + QString::number(uid) + "'/></result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());


				m_Client->profile->uid = uid;
				m_Client->status = 0;
				pServer->UpdateClient(m_Client);

				return;
			}
		}
		else
		{
			int errorType = pDataBase->pHTTP->GetError();

			QString result = "<error type='auth_failed' reason = '" + QString::number(errorType) + "'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
	}

	// Default log in mode
	if (!(pDataBase->UserExists(login)))
	{
		qDebug() << "-----------------------Login not found------------------------";
		qDebug() << "---------------------AUTHORIZATION FAILED---------------------";

		QString result = "<error type='auth_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
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

			QString result = "<error type='auth_failed' reason = '1'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}

		// Check passwords
		if (password == userData->password)
		{
			SProfile *dbProfile = pDataBase->GetUserProfile(userData->uid);

			if (dbProfile != nullptr)
			{
				m_Client->profile = dbProfile;
				m_Client->status = 1;
				pServer->UpdateClient(m_Client);

				qDebug() << "-------------------------Profile found--------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				QString result = "<result type='auth_complete'><data uid='" + QString::number(m_Client->profile->uid) + "'/></result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				result.clear();
				result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				QString result = "<result type='auth_complete'><data uid='" + QString::number(userData->uid) + "'/></result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());


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

			QString result = "<error type='auth_failed' reason = '2'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
	}
}

void ClientQuerys::onRegister(QByteArray &bytes)
{
	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString login = attributes.value("login").toString();
	QString password = attributes.value("password").toString();

	if (login.isEmpty() || password.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
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

			int uid = pDataBase->pHTTP->GetUID();

			QString result = "<result type='reg_complete'><data uid='" + QString::number(uid) + "'/></result>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
		else
		{
			int errorType = pDataBase->pHTTP->GetError();
			QString result = "<error type='reg_failed' reason = '" + QString::number(errorType) + "'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
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

				QString result = "<result type='reg_complete'><data uid='" + QString::number(uid) + "'/></result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
			else
			{
				qDebug() << "--------------Can't create account in database!--------------";
				qDebug() << "---------------------REGISTRATION FAILED---------------------";

				QString result = "<error type='reg_failed' reason = '1'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------Login alredy register or some values empty!--------";
		qDebug() << "---------------------REGISTRATION FAILED---------------------";
		QString result = "<error type='reg_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
		return;
	}
}

void ClientQuerys::onCreateProfile(QByteArray &bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't create profile without authorization!!! Uid = " << m_Client->profile->uid;
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString nickname = attributes.value("nickname").toString();
	QString fileModel = attributes.value("fileModel").toString();

	if (nickname.isEmpty() || fileModel.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Nickname = " << nickname << "fileModel = " << fileModel;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	if (!(m_Client->profile->nickname.isEmpty()))
	{
		qDebug() << "------------------Client alredy have profile-------------------";
		qDebug() << "---------------------CREATE PROFILE FAILED---------------------";

		QString result = "<error type='create_profile_failed' reason = '2'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
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

		QString stringProfile = ProfileToString(m_Client->profile);

		if (pDataBase->CreateProfile(m_Client->profile))
		{
			qDebug() << "---------------------CREATE PROFILE COMPLETE---------------------";

			QString result = "<result type='profile_data'>" + stringProfile + "</result>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

			m_Client->status = 1;
			pServer->UpdateClient(m_Client);

			return;
		}
		else
		{
			qDebug() << "---------------------Database return error---------------------";
			qDebug() << "---------------------CREATE PROFILE FAILED---------------------";

			QString result = "<error type='create_profile_failed' reason = '0'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
	}
	else
	{
		qDebug() << "-------------------Nickname alredy registered!-------------------";
		qDebug() << "---------------------CREATE PROFILE FAILED-----------------------";

		QString result = "<error type='create_profile_failed' reason = '1'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
		return;
	}
}

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

		QString result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "----------------------GET PROFILE FAILED----------------------";

		QString result = "<error type='get_profile_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
	}
}

void ClientQuerys::onGetShopItems()
{
	QFile file("scripts/shop.xml");
	QString cleanShop;
	QRegExp reg("\r\n");

	TcpServer* pServer = gEnv->pServer;

	if (!file.open(QIODevice::ReadOnly))
	{
		qCritical() << "Can't get shop.xml!!!";

		QString result = "<error type='get_shop_items_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

		return;
	}

	cleanShop = file.readAll();
	file.close();
	file.deleteLater();
	cleanShop.replace(reg, "");

	pServer->sendMessageToClient(m_socket, cleanShop.toStdString().c_str());
}

void ClientQuerys::onBuyItem(QByteArray &bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't buy item without authorization!!!";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString itemName = attributes.value("item").toString();

	if (itemName.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Item = " << itemName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	SShopItem item = GetShopItemByName(itemName);

	if (!m_Client->profile->nickname.isEmpty())
	{
		if (!item.name.isEmpty())
		{
			// Check if it is there in inventory
			if (CheckAttributeInRow(m_Client->profile->items, "item", "name", item.name))
			{
				qDebug() << "------------------This item alredy purchased------------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				QString result = "<error type='buy_item_failed' reason = '4'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			// Check minimal player level for buy this item
			if (m_Client->profile->lvl < item.minLnl)
			{
				qDebug() << "-----------------Profile level < minimal level----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				QString result = "<error type='buy_item_failed' reason = '5'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			if (m_Client->profile->money - item.cost >= 0)
			{
				//qDebug() << "[ClientQuerys] Client can buy this item";

				// Add item and update money
				m_Client->profile->money = m_Client->profile->money - item.cost;
				m_Client->profile->items = m_Client->profile->items + "<item name='" + item.name +
					"' icon='" + item.icon +
					"' description='" + item.description + "'/>";

				// Update profile
				if (UpdateProfile(m_Client->profile))
				{
					qDebug() << "----------------------Profile updated----------------------";
					qDebug() << "---------------------BUI ITEM COMPLETE---------------------";

					QString result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
				else
				{
					qDebug() << "---------------------Can't update profile---------------------";
					qDebug() << "------------------------BUY ITEM FAILED-----------------------";

					QString result = "<error type='buy_item_failed' reason = '3'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
			}
			else
			{
				qDebug() << "-------------------Insufficient money to buy-----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				QString result = "<error type='buy_item_failed' reason = '2'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
		else
		{
			qDebug() << "------------------------Item not found------------------------";
			qDebug() << "------------------------BUY ITEM FAILED-----------------------";

			QString result = "<error type='buy_item_failed' reason = '1'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "------------------------BUY ITEM FAILED-----------------------";

		QString result = "<error type='buy_item_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

		return;
	}
}

void ClientQuerys::onRemoveItem(QByteArray &bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't remove item without authorization!!!";
		return;
	}

	QString uid = QString::number(m_Client->profile->uid);
	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString itemName = attributes.value("name").toString();

	if (itemName.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Item = " << itemName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;

	if (!m_Client->profile->nickname.isEmpty())
	{
		// Check item if it is there in item list
		if (!CheckAttributeInRow(m_Client->profile->items, "item", "name", itemName))
		{
			qDebug() << "-------------------Item not found in inventory--------------------";
			qDebug() << "------------------------REMOVE ITEM FAILED-----------------------";

			QString result = "<error type='remove_item_failed' reason = '3'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
		else
		{
			// Search item in shop list
			SShopItem item = GetShopItemByName(itemName);
			if (item.name.isEmpty())
			{
				qDebug() << "-------------------Item not found in shop list-------------------";
				qDebug() << "------------------------REMOVE ITEM FAILED-----------------------";

				QString result = "<error type='remove_item_failed' reason = '2'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			// Remove item
			QString removeItem = "<item name='" + item.name + "' icon='" + item.icon + "' description='" + item.description + "'/>";
			m_Client->profile->items = RemoveElementFromRow(m_Client->profile->items, removeItem);

			// Update profile
			if (UpdateProfile(m_Client->profile))
			{
				qDebug() << "-----------------------Profile updated------------------------";
				qDebug() << "---------------------REMOVE ITEM COMPLETE---------------------";

				QString result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
			else
			{
				qDebug() << "--------------------Can't update profile--------------------";
				qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

				QString result = "<error type='remove_item_failed' reason = '1'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
	}
	else
	{
		qDebug() << "---------------------Error get profile----------------------";
		qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

		QString result = "<error type='remove_item_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
		return;
	}
}

void ClientQuerys::onInvite(QByteArray & bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't send invite without authorization!!!";
		return;
	}

	QString uid = QString::number(m_Client->profile->uid);
	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString inviteType = attributes.value("invite_type").toString();
	QString reciver = attributes.value("to").toString();

	if (m_Client->profile->nickname.isEmpty() || reciver.isEmpty() || inviteType.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
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

			QString result = "<error type='invite_failed' reason = '0'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}

		QSslSocket* reciverSocket = pServer->GetSocketByUid(friendUID);

		if (reciverSocket != nullptr)
		{
			// Send invite to client
			QString query = "<invite type='friend_invite' from='" + m_Client->profile->nickname + "'/>";
			pServer->sendMessageToClient(reciverSocket, query.toStdString().c_str());
			return;
		}
		else
		{
			qDebug() << "----------------------Reciver not online----------------------";
			qDebug() << "---------------------INVITE FRIEND FAILED---------------------";

			QString result = "<error type='invite_failed' reason = '1'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
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

void ClientQuerys::onDeclineInvite(QByteArray & bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't decline invite without authorization!!!";
		return;
	}

	QString uid = QString::number(m_Client->profile->uid);
	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString reciver = attributes.value("to").toString();

	if (m_Client->profile->nickname.isEmpty() || reciver.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
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
		QString result = "<error type='invite_failed' reason = '2'/>";
		pServer->sendMessageToClient(reciverSocket, result.toStdString().c_str());
		return;
	}
	else
	{
		qDebug() << "----------------------Reciver not online-----------------------";
		qDebug() << "---------------------DECLINE INVITE FAILED---------------------";
		return;
	}
}

void ClientQuerys::onAddFriend(QByteArray &bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't add friend without authorization!!!";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString friendName = attributes.value("name").toString();

	if (friendName.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
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
			// Check friend is there in friends list
			if (CheckAttributeInRow(m_Client->profile->friends, "friend", "name", friendProfile->nickname))
			{
				qDebug() << "--------------------This friend alredy added--------------------";
				qDebug() << "------------------------ADD FRIEND FAILED-----------------------";

				QString result = "<error type='add_friend_failed' reason = '4'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			// Block add yourself in friends
			if (m_Client->profile->uid == friendUID)
			{
				qDebug() << "----------------Can't add yourself to friends--------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				QString result = "<error type='add_friend_failed' reason = '3'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			m_Client->profile->friends = m_Client->profile->friends + "<friend name='" + friendProfile->nickname + "' uid='" + QString::number(friendUID) + "' status='0'/>";
			friendProfile->friends = friendProfile->friends + "<friend name='" + m_Client->profile->nickname + "' uid='" + QString::number(m_Client->profile->uid) + "' status='0'/>";

			QSslSocket* friendSocket = pServer->GetSocketByUid(friendUID);

			if (UpdateProfile(m_Client->profile) && UpdateProfile(friendProfile))
			{
				qDebug() << "-----------------------Profile updated-----------------------";
				qDebug() << "---------------------ADD FRIEND COMPLETE---------------------";

				QString result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					QString friendResult = "<result type='profile_data'>" + ProfileToString(friendProfile) + "</result>";
					pServer->sendMessageToClient(friendSocket, friendResult.toStdString().c_str());
				}
				//

				return;
			}
			else
			{
				qDebug() << "-------------------Can't update profile--------------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				QString result = "<error type='add_friend_failed' reason = '2'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
		else
		{
			qDebug() << "---------------------Error get profile---------------------";
			qDebug() << "---------------------ADD FRIEND FAILED---------------------";

			QString result = "<error type='add_friend_failed' reason = '1'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
	}
	else
	{
		qDebug() << "---------------------Friend not found----------------------";
		qDebug() << "---------------------ADD FRIEND FAILED---------------------";

		QString result = "<error type='add_friend_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
		return;
	}
}

void ClientQuerys::onRemoveFriend(QByteArray &bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't remove friend without authorization!!!";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString friendName = attributes.value("name").toString();

	if (friendName.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Friend = " << friendName;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	if (!m_Client->profile->nickname.isEmpty())
	{
		int friendUID = pDataBase->GetUIDbyNick(friendName);
		SProfile *friendProfile = pDataBase->GetUserProfile(friendUID);

		// Check friend is there in friends list
		if (!CheckAttributeInRow(m_Client->profile->friends, "friend", "name", friendName) || friendProfile == nullptr)
		{
			qDebug() << "--------------------------Friend not found-------------------------";
			qDebug() << "------------------------REMOVE FRIEND FAILED-----------------------";

			QString result = "<error type='remove_friend_failed' reason = '2'/>";
			pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
			return;
		}
		else
		{
			// Delete friend from client's profile
			QString removeFriend = "<friend name='" + friendProfile->nickname + "' uid='" + QString::number(friendUID) + "' status='0'/>";
			m_Client->profile->friends = RemoveElementFromRow(m_Client->profile->friends, removeFriend);
			// Delete client from friend's profile
			removeFriend.clear();
			removeFriend = "<friend name='" + m_Client->profile->nickname + "' uid='" + QString::number(m_Client->profile->uid) + "' status='0'/>";
			friendProfile->friends = RemoveElementFromRow(friendProfile->friends, removeFriend);

			QSslSocket* friendSocket = pServer->GetSocketByUid(friendUID);

			if (UpdateProfile(m_Client->profile) && UpdateProfile(friendProfile))
			{
				qDebug() << "------------------------Profile updated-------------------------";
				qDebug() << "---------------------REMOVE FRIEND COMPLETE---------------------";

				QString result = "<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					QString friendResult = "<result type='profile_data'>" + ProfileToString(friendProfile) + "</result>";
					pServer->sendMessageToClient(friendSocket, friendResult.toStdString().c_str());
				}
				//
				return;
			}
			else
			{
				qDebug() << "---------------------Can't update profile---------------------";
				qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

				QString result = "<error type='remove_friend_failed' reason = '1'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------------------Error get profile-----------------------";
		qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

		QString result = "<error type='remove_friend_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
		return;
	}
}

void ClientQuerys::onChatMessage(QByteArray &bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't remove friend without authorization!!!";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString message = attributes.value("message").toString();
	QString reciver = attributes.value("to").toString();

	if (message.isEmpty() || reciver.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "Message = " << message << "Reciver = " << reciver;
		return;
	}

	TcpServer* pServer = gEnv->pServer;
	DBWorker* pDataBase = gEnv->pDBWorker;

	if (reciver == m_Client->profile->nickname)
	{
		qDebug() << "--------------Client cannot send message to himself---------------";
		qDebug() << "---------------------SEND CHAT MESSAGE FAILED---------------------";
		return;
	}

	if (!m_Client->profile->nickname.isEmpty() && reciver == "all")
	{
		if (gEnv->pSettings->GetVariable("bUseGlobalChat").toBool())
		{
			QString chat = "<chat><message type='global' message='" + message + "' from='" + m_Client->profile->nickname + "'/></chat>";
			pServer->sendGlobalMessage(chat.toStdString().c_str());
		}
		else
		{
			qWarning() << "Client send message to global chat, but global chat now disabled, see server.cfg";
		}

		return;
	}
	else
	{
		int reciverUID = pDataBase->GetUIDbyNick(reciver);

		QSslSocket* reciverSocket = pServer->GetSocketByUid(reciverUID);

		if (reciverSocket != nullptr)
		{
			QString chat = "<chat><message type='private' message='" + message + "' from='" + m_Client->profile->nickname + "'/></chat>";
			pServer->sendMessageToClient(reciverSocket, chat.toStdString().c_str());
		}
		else
		{
			qDebug() << "-------------------Reciver not found or offline-------------------";
			qDebug() << "---------------------SEND CHAT MESSAGE FAILED---------------------";
		}

		return;
	}
}

void ClientQuerys::onGetGameServer(QByteArray & bytes)
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't get game server without authorization!!!";
		return;
	}
	TcpServer* pServer = gEnv->pServer;

	/*if (vServers.size() == 0)
	{
		qWarning() << "Not any online servers";

		// Send error to client
		QString result = "<error type='get_game_server_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

		return;
	}*/

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);

	QString map = attributes.value("map").toString();
	QString gamerules = attributes.value("gamerules").toString();
	QString serverName = attributes.value("name").toString();

	bool byMap = false;
	bool byGameRules = false;
	bool byName = false;

	if (!map.isEmpty())
		byMap = true;

	if (!gamerules.isEmpty() && !byMap)
		byGameRules = true;

	if (!serverName.isEmpty() && !byMap && !byGameRules)
		byName = true;

	/*QVector<SGameServer>::iterator it;
	for (it = vServers.begin(); it != vServers.end(); ++it)
	{
		if (byMap)
		{
			qDebug() << "Searching server by map " << map;

			if (it->map == "Multiplayer/" + map)
			{
				qDebug() << "Server found";

				QString result = "<result type='game_server_data'><data name = '" + it->name +
					"' ip = '" + it->ip +
					"' port = '" + QString::number(it->port) +
					"' map = '" + it->map +
					"' gamerules = '" + it->gamerules +
					"' online = '" + QString::number(it->online) +
					"' maxPlayers = '" + QString::number(it->maxPlayers) + "'/></result>";

				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

				return;
			}
		}

		if (byGameRules)
		{
			qDebug() << "Searching server by gamerules " << gamerules;

			if (it->gamerules == gamerules)
			{
				qDebug() << "Server found";

				QString result = "<result type='game_server_data'><data name = '" + it->name +
					"' ip = '" + it->ip +
					"' port = '" + QString::number(it->port) +
					"' map = '" + it->map +
					"' gamerules = '" + it->gamerules +
					"' online = '" + QString::number(it->online) +
					"' maxPlayers = '" + QString::number(it->maxPlayers) + "'/></result>";

				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

				return;
			}
		}

		if (byName)
		{
			qDebug() << "Searching server by server name " << serverName;

			if (it->name == serverName)
			{
				qDebug() << "Server found";

				QString result = "<result type='game_server_data'><data name = '" + it->name +
					"' ip = '" + it->ip +
					"' port = '" + QString::number(it->port) +
					"' map = '" + it->map +
					"' gamerules = '" + it->gamerules +
					"' online = '" + QString::number(it->online) +
					"' maxPlayers = '" + QString::number(it->maxPlayers) + "'/></result>";

				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

				return;
			}
		}
	}
	*/

	qDebug() << "Server not found!";

	// Send error to client
	QString result = "<error type='get_game_server_failed' reason = '1'/>";
	pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
}

// Game server functionality
void ClientQuerys::onGameServerRegister(QByteArray & bytes)
{
	return;// !!!

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString serverName = attributes.value("name").toString();
	QString serverIp = attributes.value("ip").toString();
	int serverPort = attributes.value("port").toInt();
	QString mapName = attributes.value("map").toString();
	QString gamerules = attributes.value("gamerules").toString();
	int online = attributes.value("online").toInt();
	int maxPlayers = attributes.value("maxPlayers").toInt();

	if (serverName.isEmpty() || serverIp.isEmpty() || mapName.isEmpty() || gamerules.isEmpty())
	{
		qWarning() << "Wrong packet data! Some values empty!";
		qDebug() << "ServerName = " << serverName << "ServerIp = " << serverIp << "MapName = " << mapName << "Gamerules = " << gamerules;
		return;
	}

	/*QVector<SGameServer>::iterator it;
	for (it = vServers.begin(); it != vServers.end(); ++it)
	{
		if (it->name == serverName)
		{
			qDebug() << "---------------Server with this name alredy registered---------------";
			qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";
			return;
		}
		if (it->ip == serverIp && it->port == serverPort)
		{
			qDebug() << "-------------Server with this address alredy registered--------------";
			qDebug() << "---------------------REGISTER GAME SERVER FAILED---------------------";
			return;
		}
	}*/

	// Register new game server here
	SGameServer gameServer;
	gameServer.socket = m_socket;
	gameServer.name = serverName;
	gameServer.ip = serverIp;
	gameServer.port = serverPort;
	gameServer.map = mapName;
	gameServer.gamerules = gamerules;
	gameServer.online = online;
	gameServer.maxPlayers = maxPlayers;

	//vServers.push_back(gameServer);

	// Update client info
	/*QVector<SClient>::iterator clientIt;
	for (clientIt = vClients.begin(); clientIt != vClients.end(); ++it)
	{
		if (clientIt->socket == m_socket)
		{
			clientIt->isGameServer = true;
			break;
		}
	}*/

	qDebug() << "Game server [" << serverName << "] registered!";
	//qDebug() << "Connected game servers count = " << vServers.size();
}

void ClientQuerys::onGameServerUpdateInfo(QByteArray & bytes)
{
	qDebug() << "Raw bytes = " << bytes;
}

void ClientQuerys::onGameServerGetOnlineProfile(QByteArray & bytes)
{
	qDebug() << "Raw bytes = " << bytes;
}

void ClientQuerys::onGameServerUpdateOnlineProfile(QByteArray & bytes)
{
	qDebug() << "Raw bytes = " << bytes;
}