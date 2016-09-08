#include "clientquerys.h"
#include <QRegExp>
#include "tcpserver.h"

#if !defined (QT_CREATOR_FIX_COMPILE)
#include "helper.cpp"
#endif

void ClientQuerys::onLogin(QByteArray &bytes)
{
	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString login = attributes.value("login").toString();
			QString password = attributes.value("password").toString();

			if (login.isEmpty() || password.isEmpty())
			{
				qDebug() << "Some values empty. Login = " << login << "Password = " << password;
				return;
			}

			QString key = "users:" + login;
			QString buff = pRedis->SendSyncQuery("GET", key, "");

			if (buff.isEmpty())
			{
				qDebug() << "[ClientQuerys] -----------------------Login not found------------------------";
				qDebug() << "[ClientQuerys] ---------------------AUTHORIZATION FAILED---------------------";

				QString result = "<error type='auth_failed' reason = '0'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
			else
			{

				QXmlStreamAttributes attr = GetAttributesFromString(buff);

				QString uid = attr.value("uid").toString();
				QString pass = attr.value("password").toString();
				QString ban = attr.value("ban").toString();

                // Check ban status
				if (ban == "1")
				{
					qDebug() << "[ClientQuerys] -----------------------Account blocked------------------------";
					qDebug() << "[ClientQuerys] ---------------------AUTHORIZATION FAILED---------------------";

					QString result = "<error type='auth_failed' reason = '1'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}

                // Check passwords
				if (password == pass)
				{
					SProfile* dbProfile = GetProfileByUid(uid.toInt());

					if (dbProfile != nullptr)
					{
						clientProfile = dbProfile;
						clientStatus = 1;
						AcceptProfileToGlobalList(m_socket, clientProfile, clientStatus);

						qDebug() << "[ClientQuerys] -------------------------Profile found--------------------------";
						qDebug() << "[ClientQuerys] ---------------------AUTHORIZATION COMPLETE---------------------";

						QString result = "<result type='auth_complete'><data uid='" + uid + "'/></result>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						result.clear();
						result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}
					else
					{
						qDebug() << "[ClientQuerys] -----------------------Profile not found------------------------";
						qDebug() << "[ClientQuerys] ---------------------AUTHORIZATION COMPLETE---------------------";

						QString result = "<result type='auth_complete'><data uid='" + uid + "'/></result>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());


						clientProfile->uid = uid.toInt();
						clientStatus = 0;
						AcceptProfileToGlobalList(m_socket, clientProfile, clientStatus);

						return;
					}

				}
				else
				{
					qDebug() << "[ClientQuerys] ----------------------Incorrect password----------------------";
					qDebug() << "[ClientQuerys] ---------------------AUTHORIZATION FAILED---------------------";

					QString result = "<error type='auth_failed' reason = '2'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
			}
			break;
		}
	}
}

void ClientQuerys::onRegister(QByteArray &bytes)
{
	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString login = attributes.value("login").toString();
			QString password = attributes.value("password").toString();
			QString uid, buff;

            // Check if login is there in database. If it is not - register new user
			QString key = "users:" + login;
			buff = pRedis->SendSyncQuery("GET", key, "");

			if (buff.isEmpty() && !login.isEmpty() && !password.isEmpty())
			{
                // Get uids row and create new uid if uids row are empty
				buff = pRedis->SendSyncQuery("GET", "uids", "");

				if (buff.isEmpty())
				{
                    //qDebug() << "[ClientQuerys] Key 'uids' not found! Creating key 'uids'!";
					buff = pRedis->SendSyncQuery("SET", "uids", "100001");

					if (buff == "OK")
					{
						uid = "100001";
					}
					else
					{
                        qDebug() << "[ClientQuerys] Error creating key 'uids'!!!";
                        // do smth
						return;
					}

					buff.clear();
				}
				else
				{
					int tmp = buff.toInt() + 1;
                    //qDebug() << "[ClientQuerys] Key 'uids' found! Creating new uid = " << tmp;
					buff = pRedis->SendSyncQuery("SET", "uids", QString::number(tmp));

					if (buff == "OK")
					{
						uid = QString::number(tmp);
						//qDebug() << "[ClientQuerys] New uid created = " << uid;
					}
					else
					{
						qDebug() << "[ClientQuerys] Error creating uid!";
                        //do smth
						return;
					}
				}

				buff.clear();

                // Create new database row
				QString value = "<data uid = '" + uid + "' login='" + login + "' password = '" + password + "' ban='0'/>";
				buff = pRedis->SendSyncQuery("SET", key, value);
				if (buff == "OK")
				{
					qDebug() << "[ClientQuerys] ---------------------REGISTRATION COMPLETE---------------------";

					QString result = "<result type='reg_complete'><data uid='" + uid + "'/></result>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
				else
				{
					qDebug() << "[ClientQuerys] --------------Can't create account in database!--------------";
					qDebug() << "[ClientQuerys] ---------------------REGISTRATION FAILED---------------------";

					QString result = "<error type='reg_failed' reason = '1'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
			}
			else
			{
				qDebug() << "[ClientQuerys] ----------Login alredy register or some values empty!--------";
				qDebug() << "[ClientQuerys] ---------------------REGISTRATION FAILED---------------------";
				QString result = "<error type='reg_failed' reason = '0'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			break;
		}
	}
}

void ClientQuerys::onCreateProfile(QByteArray &bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't create profile without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString nickname = attributes.value("nickname").toString();
			QString model = attributes.value("model").toString();

			if (nickname.isEmpty() || model.isEmpty())
			{
                qDebug() << "[ClientQuerys] Some values empty!!! Nickname = " << nickname << " Model = " << model << " Uid = " << uid;
				return;
			}

			if (!clientProfile->nickname.isEmpty())
			{
				qDebug() << "[ClientQuerys] ------------------Client alredy have profile-------------------";
				qDebug() << "[ClientQuerys] ---------------------CREATE PROFILE FAILED---------------------";

				QString result = "<error type='create_profile_failed' reason = '2'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}

			QString key = "nicknames:" + nickname;
			QString buff = pRedis->SendSyncQuery("GET", key, "");

            // Check if nickname is there in databasse. If it is not - create new profile
			if (buff.isEmpty())
			{
				clientProfile->nickname = nickname;
				clientProfile->model = model;
				clientProfile->money = startMoney;
				clientProfile->xp = 0;
				clientProfile->lvl = 0;
				clientProfile->items = "";
				clientProfile->achievements = "";
				clientProfile->stats = "<stats kills='0' deaths='0' kd='0'/>";

				key.clear(); buff.clear();

                // Create key for profile and for nickname
				key = "profiles:" + uid;
				QString key2 = "nicknames:" + nickname;

				buff = pRedis->SendSyncQuery("SET", key, ProfileToString(clientProfile));
				QString buff2 = pRedis->SendSyncQuery("SET", key2, uid);

				if (buff == "OK" && buff2 == "OK")
				{
					qDebug() << "[ClientQuerys] ---------------------CREATE PROFILE COMPLETE---------------------";

					QString result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());


					clientStatus = 1;
					AcceptProfileToGlobalList(m_socket, clientProfile, clientStatus);

					return;
				}
				else
				{
					qDebug() << "[ClientQuerys] ---------------------Database return error---------------------";
					qDebug() << "[ClientQuerys] ---------------------CREATE PROFILE FAILED---------------------";

					QString result = "<error type='create_profile_failed' reason = '0'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
			}
			else
			{
				qDebug() << "[ClientQuerys] -------------------Nickname alredy registered!-------------------";
				qDebug() << "[ClientQuerys] ---------------------CREATE PROFILE FAILED---------------------";

				QString result = "<error type='create_profile_failed' reason = '1'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}


			break;
		}
	}
}

void ClientQuerys::onGetProfile(QByteArray &bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't get profile without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	if (!clientProfile->nickname.isEmpty())
	{
		qDebug() << "[ClientQuerys] -------------------------Profile found--------------------------";
		qDebug() << "[ClientQuerys] ----------------------GET PROFILE COMPLETE----------------------";

		QString result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
	}
	else
	{
		qDebug() << "[ClientQuerys] ----------------------Profile not found-----------------------";
		qDebug() << "[ClientQuerys] ----------------------GET PROFILE FAILED----------------------";

		QString result = "<error type='get_profile_failed' reason = '0'/>";
		pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
	}
}

void ClientQuerys::onGetShopItems(QByteArray &bytes)
{
	QFile file("shop.xml");
	QString cleanShop;
	QRegExp reg("\r\n");

	if (!file.open(QIODevice::ReadOnly))
	{
		qDebug() << "[ClientQuerys] Can't get shop.xml!!!";

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
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't buy item without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);


	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString itemName = attributes.value("item").toString();

			if (itemName.isEmpty())
			{
                qDebug() << "[ClientQuerys] Some values empty!!! Item = " << itemName << " Uid = " << uid;
				return;
			}

			SShopItem item = GetShopItemByName(itemName);

			if (!clientProfile->nickname.isEmpty())
			{
				if (!item.name.isEmpty())
				{
                    // Check if it is there in inventory
					if (CheckAttributeInRow(clientProfile->items, "item", "name", item.name))
					{
						qDebug() << "[ClientQuerys] ------------------This item alredy purchased------------------";
						qDebug() << "[ClientQuerys] ------------------------BUY ITEM FAILED-----------------------";

						QString result = "<error type='buy_item_failed' reason = '4'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}

                    // Check minimal player level for buy this item
					if (clientProfile->lvl < item.minLnl)
					{
						qDebug() << "[ClientQuerys] -----------------Profile level < minimal level----------------";
						qDebug() << "[ClientQuerys] ------------------------BUY ITEM FAILED-----------------------";

						QString result = "<error type='buy_item_failed' reason = '5'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}

					if (clientProfile->money - item.cost >= 0)
					{
                        //qDebug() << "[ClientQuerys] Client can buy this item";

                        // Add item and update money
						clientProfile->money = clientProfile->money - item.cost;
						clientProfile->items = clientProfile->items + "<item name='" + item.name +
							"' icon='" + item.icon +
							"' description='" + item.description + "'/>";

                        // Update profile
						if (UpdateProfile(m_socket, clientProfile))
						{
							qDebug() << "[ClientQuerys] ----------------------Profile updated----------------------";
							qDebug() << "[ClientQuerys] ---------------------BUI ITEM COMPLETE---------------------";

							QString result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
							pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
							return;
						}
						else
						{
							qDebug() << "[ClientQuerys] ---------------------Can't update profile---------------------";
							qDebug() << "[ClientQuerys] ------------------------BUY ITEM FAILED-----------------------";

							QString result = "<error type='buy_item_failed' reason = '3'/>";
							pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
							return;
						}
					}
					else
					{
						qDebug() << "[ClientQuerys] -------------------Insufficient money to buy-----------------";
						qDebug() << "[ClientQuerys] ------------------------BUY ITEM FAILED-----------------------";

						QString result = "<error type='buy_item_failed' reason = '2'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}
				}
				else
				{
					qDebug() << "[ClientQuerys] ------------------------Item not found------------------------";
					qDebug() << "[ClientQuerys] ------------------------BUY ITEM FAILED-----------------------";

					QString result = "<error type='buy_item_failed' reason = '1'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
			}
			else
			{
				qDebug() << "[ClientQuerys] ----------------------Profile not found-----------------------";
				qDebug() << "[ClientQuerys] ------------------------BUY ITEM FAILED-----------------------";

				QString result = "<error type='buy_item_failed' reason = '0'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());

				return;
			}
			break;
		}
	}
}

void ClientQuerys::onRemoveItem(QByteArray &bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't remove item without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString itemName = attributes.value("name").toString();

			if (itemName.isEmpty())
			{
                qDebug() << "[ClientQuerys] Some values empty!!! Item = " << itemName << " Uid = " << uid;
				return;
			}

			if (!clientProfile->nickname.isEmpty())
			{
                // Check item if it is there in item list
				if (!CheckAttributeInRow(clientProfile->items, "item", "name", itemName))
				{
                    qDebug() << "[ClientQuerys] -------------------Item not found in inventory--------------------";
					qDebug() << "[ClientQuerys] ------------------------REMOVE ITEM FAILED-----------------------";

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
                        qDebug() << "[ClientQuerys] -------------------Item not found in shop list-------------------";
						qDebug() << "[ClientQuerys] ------------------------REMOVE ITEM FAILED-----------------------";

						QString result = "<error type='remove_item_failed' reason = '2'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}

                    // Remove item
					QString removeItem = "<item name='" + item.name + "' icon='" + item.icon + "' description='" + item.description + "'/>";
					clientProfile->items = RemoveElementFromRow(clientProfile->items, removeItem);

                    // Update profile
					if (UpdateProfile(m_socket, clientProfile))
					{
						qDebug() << "[ClientQuerys] -----------------------Profile updated------------------------";
						qDebug() << "[ClientQuerys] ---------------------REMOVE ITEM COMPLETE---------------------";

						QString result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}
					else
					{
						qDebug() << "[ClientQuerys] --------------------Can't update profile--------------------";
						qDebug() << "[ClientQuerys] ---------------------REMOVE ITEM FAILED---------------------";

						QString result = "<error type='remove_item_failed' reason = '1'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}
				}
			}
			else
			{
				qDebug() << "[ClientQuerys] ---------------------Error get profile----------------------";
				qDebug() << "[ClientQuerys] ---------------------REMOVE ITEM FAILED---------------------";

				QString result = "<error type='remove_item_failed' reason = '0'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
	}
}

void ClientQuerys::onInvite(QByteArray & bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't send invite without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString inviteType = attributes.value("invite_type").toString();
			QString reciver = attributes.value("to").toString();

			if (clientProfile->nickname.isEmpty() || reciver.isEmpty() || inviteType.isEmpty())
			{
                qDebug() << "[ClientQuerys] Some values empty!!! Invite type = " << inviteType << "Client = " << clientProfile->nickname << "Reciver = " << reciver;
				return;
			}

            // Friend invite
			if (inviteType == "friend_invite")
			{
				QString key = "nicknames:" + reciver;
				QString reciverUid = pRedis->SendSyncQuery("GET", key, "");

				if (reciverUid.isEmpty())
				{
					qDebug() << "[ClientQuerys] User not found!";

					QString result = "<error type='invite_failed' reason = '0'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}

				QSslSocket* reciverSocket = GetSocketByUid(reciverUid.toInt());

				if (reciverSocket != nullptr)
				{
                    // Send invite to client
					QString query = "<invite type='friend_invite' from='" + clientProfile->nickname + "'/>";
					pServer->sendMessageToClient(reciverSocket, query.toStdString().c_str());
					return;
				}
				else
				{
					qDebug() << "[ClientQuerys] Client can't send invite, because reciver not online!";

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
	}
}

void ClientQuerys::onDeclineInvite(QByteArray & bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't decline invite without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if (xml.name() == "data")
        {
            QXmlStreamAttributes attributes = xml.attributes();
            // We don't need know invite type for this operation, but we can get it if we need it
            QString inviteType = attributes.value("invite_type").toString();
            QString reciver = attributes.value("to").toString();

            if (clientProfile->nickname.isEmpty() || reciver.isEmpty() || inviteType.isEmpty())
            {
                qDebug() << "[ClientQuerys] Some values empty!!! Invite type = " << inviteType << "Client = " << clientProfile->nickname << "Reciver = " << reciver;
                return;
            }

            QString key = "nicknames:" + reciver;
            QString reciverUid = pRedis->SendSyncQuery("GET", key, "");

            if (reciverUid.isEmpty())
            {
                qDebug() << "[ClientQuerys] User not found!";
                return;
            }

            QSslSocket* reciverSocket = GetSocketByUid(reciverUid.toInt());

            if (reciverSocket != nullptr)
            {
                // Send decline invite to invite sender
                QString result = "<error type='invite_failed' reason = '2'/>";
                pServer->sendMessageToClient(reciverSocket, result.toStdString().c_str());
                return;
            }
            else
            {
                qDebug() << "[ClientQuerys] Client can't decline  invite, because reciver not online!";
                return;
            }
        }
    }
}

void ClientQuerys::onAddFriend(QByteArray &bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't add friend without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString friendName = attributes.value("name").toString();

			if (friendName.isEmpty())
			{
				qDebug() << "[ClientQuerys] Some values empty!!! Friend = " << friendName << " Uid = " << uid;
				return;
			}

			QString key = "nicknames:" + friendName;
			QString friendUid = pRedis->SendSyncQuery("GET", key, "");

			if (!friendUid.isEmpty())
			{
				SProfile* friendProfile = GetProfileByUid(friendUid.toInt());

				if (!clientProfile->nickname.isEmpty() && friendProfile != nullptr)
				{
					// Check friend is there in friends list
					if (CheckAttributeInRow(clientProfile->friends, "friend", "name", friendName))
					{
						qDebug() << "[ClientQuerys] --------------------This friend alredy added--------------------";
						qDebug() << "[ClientQuerys] ------------------------ADD FRIEND FAILED-----------------------";

						QString result = "<error type='add_friend_failed' reason = '4'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}

					// Block add yourself in friends
					if (clientProfile->nickname == friendName)
					{
						qDebug() << "[ClientQuerys] ----------------Can't add yourself to friends--------------";
						qDebug() << "[ClientQuerys] ---------------------ADD FRIEND FAILED---------------------";

						QString result = "<error type='add_friend_failed' reason = '3'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}

					clientProfile->friends = clientProfile->friends + "<friend name='" + friendName + "' uid='" + friendUid + "' status='0'/>";
					friendProfile->friends = friendProfile->friends + "<friend name='" + clientProfile->nickname + "' uid='" + QString::number(clientProfile->uid) + "' status='0'/>";

					QSslSocket* friendSocket = GetSocketByUid(friendUid.toInt());

					if (UpdateProfile(m_socket, clientProfile) && UpdateProfile(friendSocket, friendProfile))
					{
						qDebug() << "[ClientQuerys] -----------------------Profile updated-----------------------";
						qDebug() << "[ClientQuerys] ---------------------ADD FRIEND COMPLETE---------------------";

						QString result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
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
						qDebug() << "[ClientQuerys] -------------------Can't update profile--------------------";
						qDebug() << "[ClientQuerys] ---------------------ADD FRIEND FAILED---------------------";

						QString result = "<error type='add_friend_failed' reason = '2'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}
				}
				else
				{
					qDebug() << "[ClientQuerys] ---------------------Error get profile---------------------";
					qDebug() << "[ClientQuerys] ---------------------ADD FRIEND FAILED---------------------";

					QString result = "<error type='add_friend_failed' reason = '1'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
			}
			else
			{
				qDebug() << "[ClientQuerys] ---------------------Friend not found----------------------";
				qDebug() << "[ClientQuerys] ---------------------ADD FRIEND FAILED---------------------";

				QString result = "<error type='add_friend_failed' reason = '0'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
	}
}

void ClientQuerys::onRemoveFriend(QByteArray &bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't remove friend without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);

	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString friendName = attributes.value("name").toString();

			if (friendName.isEmpty())
			{
				qDebug() << "[ClientQuerys] Some values epmty!!! Friend = " << friendName << " Uid = " << uid;
				return;
			}

			QString key = "nicknames:" + friendName;
			QString friendUid = pRedis->SendSyncQuery("GET", key, "");

			SProfile* friendProfile = GetProfileByUid(friendUid.toInt());

			if (!clientProfile->nickname.isEmpty() && friendProfile != nullptr)
			{
				// Check friend is there in friends list
				if (!CheckAttributeInRow(clientProfile->friends, "friend", "name", friendName))
				{
					qDebug() << "[ClientQuerys] --------------------------Friend not found-------------------------";
					qDebug() << "[ClientQuerys] ------------------------REMOVE FRIEND FAILED-----------------------";

					QString result = "<error type='remove_friend_failed' reason = '2'/>";
					pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
					return;
				}
				else
				{
					// Delete friend from client's profile
					QString removeFriend = "<friend name='" + friendName + "' uid='" + friendUid + "' status='0'/>";
					clientProfile->friends = RemoveElementFromRow(clientProfile->friends, removeFriend);
					// Delete client from friend's profile
					removeFriend.clear();
					removeFriend = "<friend name='" + clientProfile->nickname + "' uid='" +uid + "' status='0'/>";
					friendProfile->friends = RemoveElementFromRow(friendProfile->friends, removeFriend);

					QSslSocket* friendSocket = GetSocketByUid(friendUid.toInt());

					if (UpdateProfile(m_socket, clientProfile) && UpdateProfile(friendSocket, friendProfile))
					{
						qDebug() << "[ClientQuerys] ------------------------Profile updated-------------------------";
						qDebug() << "[ClientQuerys] ---------------------REMOVE FRIEND COMPLETE---------------------";

						QString result = "<result type='profile_data'>" + ProfileToString(clientProfile) + "</result>";
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
						qDebug() << "[ClientQuerys] ---------------------Can't update profile---------------------";
						qDebug() << "[ClientQuerys] ---------------------REMOVE FRIEND FAILED---------------------";

						QString result = "<error type='remove_friend_failed' reason = '1'/>";
						pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
						return;
					}
				}
			}
			else
			{
				qDebug() << "[ClientQuerys] ----------------------Error get profile-----------------------";
				qDebug() << "[ClientQuerys] ---------------------REMOVE FRIEND FAILED---------------------";

				QString result = "<error type='remove_friend_failed' reason = '0'/>";
				pServer->sendMessageToClient(m_socket, result.toStdString().c_str());
				return;
			}
		}
	}
}

void ClientQuerys::onChatMessage(QByteArray &bytes)
{
	if (clientProfile->uid <= 0)
	{
		qDebug() << "[ClientQuerys] Client can't remove friend without authorization!!!";
		return;
	}

	QString uid = QString::number(clientProfile->uid);


	QXmlStreamReader xml(bytes);
	xml.readNext();
	while (!xml.atEnd() && !xml.hasError())
	{
		xml.readNext();

		if (xml.name() == "data")
		{
			QXmlStreamAttributes attributes = xml.attributes();
			QString message = attributes.value("message").toString();
			QString reciver = attributes.value("to").toString();

			if (message.isEmpty() || reciver.isEmpty())
			{
				qDebug() << "[ClientQuerys] Some values emty!!! Message = " << message << " Reciver = " << reciver;
				return;
			}

			if (reciver == clientProfile->nickname)
			{
				qDebug() << "[ClientQuerys] Client can't send message to himself!";
				return;
			}

			if (!clientProfile->nickname.isEmpty() && reciver == "all")
			{
				// We don't need to use global chat because this operation needs big resources (network + cpu time)

				/*QString chat = "<chat><message type='global' message='" + message + "' from='" + clientProfile->nickname + "'/></chat>";

				pServer->sendGlobalMessage(chat.toStdString().c_str());*/
				return;
			}
			else
			{
				int reciverUid = GetUidByName(reciver);

				QSslSocket* reciverSocket = GetSocketByUid(reciverUid);

				if (reciverSocket != nullptr)
				{
					QString chat = "<chat><message type='private' message='" + message + "' from='" + clientProfile->nickname + "'/></chat>";
					pServer->sendMessageToClient(reciverSocket, chat.toStdString().c_str());
				}
				else
				{
					qDebug() << "[ClientQuerys] Client can't send chat message, because reciver not found!";
				}

				return;
			}
		}
	}
}
