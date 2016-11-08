// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "clientquerys.h"
#include "tcpserver.h"
#include "dbworker.h"
#include "httpconnector.h"
#include "settings.h"
#include "remoteserver.h"

#include <QRegExp>

#if !defined (QT_CREATOR_FIX_COMPILE)
#include "helper.cpp"
#endif

void ClientQuerys::onLogin(QByteArray &bytes)
{
	if (bAuthorizated)
	{
		qDebug() << "Client alredy authorizated!";
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);

	QString login = attributes.value("login").toString();
	QString password = attributes.value("password").toString();

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

				QByteArray result;
				result.append("<result type='auth_complete'><data uid='" + QString::number(m_Client->profile->uid) + "'/></result>");
				pServer->sendMessageToClient(m_socket, result);
				result.clear();

				result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				QByteArray result;
				result.append("<result type='auth_complete'><data uid='" + QString::number(uid) + "'/></result>");
				pServer->sendMessageToClient(m_socket, result);

				bAuthorizated = true;
				m_Client->profile->uid = uid;
				m_Client->status = 0;
				pServer->UpdateClient(m_Client);

				return;
			}
		}
		else
		{
			int errorType = pDataBase->pHTTP->GetError();

			QByteArray result;
			result.append("<error type='auth_failed' reason = '" + QString::number(errorType) + "'/>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}
	}

	// Default log in mode
	if (!(pDataBase->UserExists(login)))
	{
		qDebug() << "-----------------------Login not found------------------------";
		qDebug() << "---------------------AUTHORIZATION FAILED---------------------";

		QByteArray result;
		result.append("<error type='auth_failed' reason = '0'/>");

		pServer->sendMessageToClient(m_socket, result);
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

			QByteArray result;
			result.append( "<error type='auth_failed' reason = '1'/>");
			pServer->sendMessageToClient(m_socket, result);
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

				QByteArray result;
				result.append("<result type='auth_complete'><data uid='" + QString::number(m_Client->profile->uid) + "'/></result>");
				pServer->sendMessageToClient(m_socket, result);
				result.clear();

				result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
			else
			{
				qDebug() << "-----------------------Profile not found------------------------";
				qDebug() << "---------------------AUTHORIZATION COMPLETE---------------------";

				QByteArray result;
				result.append("<result type='auth_complete'><data uid='" + QString::number(userData->uid) + "'/></result>");
				pServer->sendMessageToClient(m_socket, result);

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

			QByteArray result ("<error type='auth_failed' reason = '2'/>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}
	}
}

void ClientQuerys::onRegister(QByteArray &bytes)
{
	if (bRegistered)
	{
		qDebug() << "Client alredy registered!";

		QByteArray result;
		result.append("<error type='reg_failed' reason = '1'/>");
		gEnv->pServer->sendMessageToClient(m_socket, result);
		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);
	QString login = attributes.value("login").toString();
	QString password = attributes.value("password").toString();

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

			int uid = pDataBase->pHTTP->GetUID();

			bRegistered = true;

			QByteArray result;
			result.append("<result type='reg_complete'><data uid='" + QString::number(uid) + "'/></result>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}
		else
		{
			int errorType = pDataBase->pHTTP->GetError();

			QByteArray result;
			result.append("<error type='reg_failed' reason = '" + QString::number(errorType) + "'/>");
			pServer->sendMessageToClient(m_socket, result);
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

				QByteArray result;
				result.append("<result type='reg_complete'><data uid='" + QString::number(uid) + "'/></result>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
			else
			{
				qDebug() << "--------------Can't create account in database!--------------";
				qDebug() << "---------------------REGISTRATION FAILED---------------------";

				QByteArray result("<error type='reg_failed' reason = '1'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------Login alredy register or some values empty!--------";
		qDebug() << "---------------------REGISTRATION FAILED---------------------";
		QByteArray result;
		result.append("<error type='reg_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);
		return;
	}
}

void ClientQuerys::onCreateProfile(QByteArray &bytes)
{
	if (bProfileCreated)
	{
		qDebug() << "Client alredy create profile!";
		return;
	}

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

		QByteArray result ("<error type='create_profile_failed' reason = '2'/>");
		pServer->sendMessageToClient(m_socket, result);
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

			QByteArray result;
			result.append("<result type='profile_data'>" + stringProfile + "</result>");
			pServer->sendMessageToClient(m_socket, result);

			bProfileCreated = true;
			m_Client->status = 1;
			pServer->UpdateClient(m_Client);

			return;
		}
		else
		{
			qDebug() << "---------------------Database return error---------------------";
			qDebug() << "---------------------CREATE PROFILE FAILED---------------------";

			QByteArray result("<error type='create_profile_failed' reason = '0'/>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}
	}
	else
	{
		qDebug() << "-------------------Nickname alredy registered!-------------------";
		qDebug() << "---------------------CREATE PROFILE FAILED-----------------------";

		QByteArray result("<error type='create_profile_failed' reason = '1'/>");
		pServer->sendMessageToClient(m_socket, result);
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

		QByteArray result;
		result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
		pServer->sendMessageToClient(m_socket, result);
		return;
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "----------------------GET PROFILE FAILED----------------------";

		QByteArray result ("<error type='get_profile_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);
	}
}

void ClientQuerys::onGetShopItems()
{
	if (m_Client->profile->uid <= 0)
	{
		qWarning() << "Client can't get shop without authorization!!!";
		return;
	}

	QFile file("scripts/shop.xml");
	QString cleanShop;
	QRegExp reg("\r\n");

	TcpServer* pServer = gEnv->pServer;

	if (!file.open(QIODevice::ReadOnly))
	{
		qCritical() << "Can't get shop.xml!!!";

		QByteArray result ("<error type='get_shop_items_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);

		return;
	}

	cleanShop = file.readAll();
	file.close();
	file.deleteLater();
	cleanShop.replace(reg, "");

	QByteArray result;
	result.append(cleanShop);

	pServer->sendMessageToClient(m_socket, result);
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
			// Check if it is there in inventory
			if (CheckAttributeInRow(m_Client->profile->items, "item", "name", item.name))
			{
				qDebug() << "------------------This item alredy purchased------------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				QByteArray result ("<error type='buy_item_failed' reason = '4'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}

			// Check minimal player level for buy this item
			if (m_Client->profile->lvl < item.minLnl)
			{
				qDebug() << "-----------------Profile level < minimal level----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				QByteArray result("<error type='buy_item_failed' reason = '5'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}

			if (m_Client->profile->money - item.cost >= 0)
			{
				qDebug() << "Client can buy item" << item.name;

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

					QByteArray result;
					result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
					pServer->sendMessageToClient(m_socket, result);
					return;
				}
				else
				{
					qDebug() << "---------------------Can't update profile---------------------";
					qDebug() << "------------------------BUY ITEM FAILED-----------------------";

					QByteArray result ("<error type='buy_item_failed' reason = '3'/>");
					pServer->sendMessageToClient(m_socket, result);
					return;
				}
			}
			else
			{
				qDebug() << "-------------------Insufficient money to buy-----------------";
				qDebug() << "------------------------BUY ITEM FAILED-----------------------";

				QByteArray result("<error type='buy_item_failed' reason = '2'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
		}
		else
		{
			qDebug() << "------------------------Item not found------------------------";
			qDebug() << "------------------------BUY ITEM FAILED-----------------------";

			QByteArray result("<error type='buy_item_failed' reason = '1'/>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}
	}
	else
	{
		qDebug() << "----------------------Profile not found-----------------------";
		qDebug() << "------------------------BUY ITEM FAILED-----------------------";

		QByteArray result("<error type='buy_item_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);

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
		qDebug() << "Wrong packet data! Some values empty!";
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

			QByteArray result("<error type='remove_item_failed' reason = '3'/>");
			pServer->sendMessageToClient(m_socket, result);
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

				QByteArray result("<error type='remove_item_failed' reason = '2'/>");
				pServer->sendMessageToClient(m_socket, result);
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

				QByteArray result;
				result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
			else
			{
				qDebug() << "--------------------Can't update profile--------------------";
				qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

				QByteArray result("<error type='remove_item_failed' reason = '1'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
		}
	}
	else
	{
		qDebug() << "---------------------Error get profile----------------------";
		qDebug() << "---------------------REMOVE ITEM FAILED---------------------";

		QByteArray result("<error type='remove_item_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);
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

			QByteArray result("<error type='invite_failed' reason = '0'/>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}

		QSslSocket* reciverSocket = pServer->GetSocketByUid(friendUID);

		if (reciverSocket != nullptr)
		{
			// Send invite to client
			QByteArray query;
			query.append("<invite type='friend_invite' from='" + m_Client->profile->nickname + "'/>");
			pServer->sendMessageToClient(reciverSocket, query);
			return;
		}
		else
		{
			qDebug() << "----------------------Reciver not online----------------------";
			qDebug() << "---------------------INVITE FRIEND FAILED---------------------";

			QByteArray result("<error type='invite_failed' reason = '1'/>");
			pServer->sendMessageToClient(m_socket, result);
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
		QByteArray result ("<error type='invite_failed' reason = '2'/>");
		pServer->sendMessageToClient(reciverSocket, result);
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
			// Check friend is there in friends list
			if (CheckAttributeInRow(m_Client->profile->friends, "friend", "name", friendProfile->nickname))
			{
				qDebug() << "--------------------This friend alredy added--------------------";
				qDebug() << "------------------------ADD FRIEND FAILED-----------------------";

				QByteArray result ("<error type='add_friend_failed' reason = '4'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}

			// Block add yourself in friends
			if (m_Client->profile->uid == friendUID)
			{
				qDebug() << "----------------Can't add yourself to friends--------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				QByteArray result("<error type='add_friend_failed' reason = '3'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}

			m_Client->profile->friends = m_Client->profile->friends + "<friend name='" + friendProfile->nickname + "' uid='" + QString::number(friendUID) + "' status='0'/>";
			friendProfile->friends = friendProfile->friends + "<friend name='" + m_Client->profile->nickname + "' uid='" + QString::number(m_Client->profile->uid) + "' status='0'/>";

			QSslSocket* friendSocket = pServer->GetSocketByUid(friendUID);

			if (UpdateProfile(m_Client->profile) && UpdateProfile(friendProfile))
			{
				qDebug() << "-----------------------Profile updated-----------------------";
				qDebug() << "---------------------ADD FRIEND COMPLETE---------------------";

				QByteArray result;
				result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
				pServer->sendMessageToClient(m_socket, result);

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					QByteArray friendResult;
					friendResult.append("<result type='profile_data'>" + ProfileToString(friendProfile) + "</result>");
					pServer->sendMessageToClient(friendSocket, friendResult);
				}
				//

				return;
			}
			else
			{
				qDebug() << "-------------------Can't update profile--------------------";
				qDebug() << "---------------------ADD FRIEND FAILED---------------------";

				QByteArray result ("<error type='add_friend_failed' reason = '2'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
		}
		else
		{
			qDebug() << "---------------------Error get profile---------------------";
			qDebug() << "---------------------ADD FRIEND FAILED---------------------";

			QByteArray result ("<error type='add_friend_failed' reason = '1'/>");
			pServer->sendMessageToClient(m_socket, result);
			return;
		}
	}
	else
	{
		qDebug() << "---------------------Friend not found----------------------";
		qDebug() << "---------------------ADD FRIEND FAILED---------------------";

		QByteArray result("<error type='add_friend_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);
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
		qDebug() << "Wrong packet data! Some values empty!";
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

			QByteArray result ("<error type='remove_friend_failed' reason = '2'/>");
			pServer->sendMessageToClient(m_socket, result);
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

				QByteArray result;
				result.append("<result type='profile_data'>" + ProfileToString(m_Client->profile) + "</result>");
				pServer->sendMessageToClient(m_socket, result);

				//Send new info to friend here
				if (friendSocket != nullptr)
				{
					QByteArray friendResult;
					friendResult.append("<result type='profile_data'>" + ProfileToString(friendProfile) + "</result>");
					pServer->sendMessageToClient(friendSocket, friendResult);
				}
				//
				return;
			}
			else
			{
				qDebug() << "---------------------Can't update profile---------------------";
				qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

				QByteArray result ("<error type='remove_friend_failed' reason = '1'/>");
				pServer->sendMessageToClient(m_socket, result);
				return;
			}
		}
	}
	else
	{
		qDebug() << "----------------------Error get profile-----------------------";
		qDebug() << "---------------------REMOVE FRIEND FAILED---------------------";

		QByteArray result("<error type='remove_friend_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);
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
		return;
	}

	if (!m_Client->profile->nickname.isEmpty() && reciver == "all")
	{
		if (gEnv->pSettings->GetVariable("bUseGlobalChat").toBool())
		{
			QByteArray chat;
			chat.append("<chat><message type='global' message='" + message + "' from='" + m_Client->profile->nickname + "'/></chat>");
			pServer->sendGlobalMessage(chat);

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
			QByteArray chat;
			chat.append("<chat><message type='private' message='" + message + "' from='" + m_Client->profile->nickname + "'/></chat>");
			pServer->sendMessageToClient(reciverSocket, chat);
			return;
		}
		else
		{
			qDebug() << "-------------------Reciver not found or offline-------------------";
			qDebug() << "---------------------SEND CHAT MESSAGE FAILED---------------------";
			return;
		}
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

	int gameServersCount = 0;
	gEnv->pRemoteServer->bHaveAdmin ? gameServersCount = gEnv->pRemoteServer->GetClientCount() - 1 : gameServersCount = gEnv->pRemoteServer->GetClientCount();

	if (gameServersCount <= 0)
	{
		qDebug() << "Not any online servers";

		// Send error to client
		QByteArray result ("<error type='get_game_server_failed' reason = '0'/>");
		pServer->sendMessageToClient(m_socket, result);

		return;
	}

	QXmlStreamAttributes attributes = GetAttributesFromArray(bytes);

	QString map = attributes.value("map").toString();
	QString gamerules = attributes.value("gamerules").toString();
	QString serverName = attributes.value("name").toString();

	SGameServer* pGameServer = gEnv->pRemoteServer->GetGameServer(serverName, map, gamerules);

	if (pGameServer != nullptr)
	{
		QByteArray result;
		result.append("<result type='game_server_data'><data name = '" + pGameServer->name +
			"' ip = '" + pGameServer->ip +
			"' port = '" + QString::number(pGameServer->port) +
			"' map = '" + pGameServer->map +
			"' gamerules = '" + pGameServer->gamerules +
			"' online = '" + QString::number(pGameServer->online) +
			"' maxPlayers = '" + QString::number(pGameServer->maxPlayers) + "'/></result>");

		pServer->sendMessageToClient(m_socket, result);

		return;
	}
	else
	{
		qDebug() << "Server not found!";

		// Send error to client
		QByteArray result("<error type='get_game_server_failed' reason = '1'/>");
		pServer->sendMessageToClient(m_socket, result);
	}
}