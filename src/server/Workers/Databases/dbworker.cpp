// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "dbworker.h"
#include "redisconnector.h"
#include "mysqlconnector.h"
#include "httpconnector.h"

#include "Workers/Packets/clientquerys.h"
#include "Tools/settings.h"

#include <QRegExp>
#include <QSqlQuery>

DBWorker::DBWorker(QObject *parent) : QObject(parent),
	pRedis(nullptr),
	pMySql(nullptr),
	pHTTP(nullptr)
{
}

DBWorker::~DBWorker()
{
	qDebug() << "~DBWorker";
	SAFE_RELEASE(pRedis);
	SAFE_RELEASE(pMySql);
	SAFE_RELEASE(pHTTP);
}

void DBWorker::Clear()
{
	if (pRedis != nullptr)
	{
		pRedis->Disconnect();
	}
	if (pMySql != nullptr)
	{
		pMySql->Disconnect();
	}
}

void DBWorker::Init()
{
	gEnv->m_ServerStatus.m_DBStatus = "init";

	// Create Redis connection
	if (gEnv->pSettings->GetVariable("bUseRedis").toBool())
	{
		qInfo() << "Start Redis service...";
		pRedis = new RedisConnector;
		pRedis->run();
	}

	// Create MySQL connection
	if (gEnv->pSettings->GetVariable("bUseMySQL").toBool())
	{
		qInfo() << "Start MySql service...";
		gEnv->pSettings->SetVariable("redis_bg_saving", true);
		pMySql = new MySqlConnector;
		pMySql->run();
	}

	// Create HTTP connector
	if (gEnv->pSettings->GetVariable("bUseHttpAuth").toBool())
	{
		qWarning() << "Authorization mode set to HTTP, this can slows server";
		pHTTP = new HttpConnector;
	}
}

bool DBWorker::UserExists(const QString &login)
{
	bool result = false;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "users:" + login;
			result = pRedis->HEXISTS(key, "password");
		}
		else
		{
			qCritical() << "Failed found user" << login << "in Redis DB because Redis DB not connected!";
			return false;
		}
	}	

	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM users WHERE login=:login");
			query->bindValue(":login", login);

			if (query->exec())
			{
				if (query->next())
				{
					qDebug() << "Login" << login << "finded in MySql DB";
					result = true;
				}
				else
				{
					qDebug() << "Login" << login << "not found in MySql DB";
					return false;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed found user" << login << "in MySql DB because MySql DB not opened!";
			return false;
		}

	}
	
	return result;
}

bool DBWorker::ProfileExists(int uid)
{
	bool result = false;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "profiles:" + QString::number(uid);
			result = pRedis->HEXISTS(key, "nickname");
		}
		else
		{
			qCritical() << "Failed found profile" << uid << "in Redis DB because Redis DB not connected!";
			return false;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM profiles WHERE uid=:uid");
			query->bindValue(":uid", uid);

			if (query->exec())
			{
				if (query->next())
				{
					qDebug() << "Profile" << uid << "finded in MySql DB";
					result = true;
				}
				else
				{
					qDebug() << "Profile" << uid << "not found in MySql DB";
					return false;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed found profile" << uid << "in MySql DB because MySql DB not opened!";
			return false;
		}
	}
	
	return result;
}

bool DBWorker::NicknameExists(const QString &nickname)
{
	bool result = false;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "nicknames:" + nickname;
			QString buff = pRedis->GET(key);

			if (!buff.isEmpty())
			{
				qDebug() << "Nickname" << nickname << "finded in Redis DB";
				result = true;
			}
			else
			{
				qDebug() << "Nickname" << nickname << "not found in Redis DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed found nickname" << nickname << "in Redis DB because Redis DB not connected!";
			return false;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM profiles WHERE nickname=:nickname");
			query->bindValue(":nickname", nickname);

			if (query->exec())
			{
				if (query->next())
				{
					qDebug() << "Nickname" << nickname << "finded in MySql DB";
					result = true;
				}
				else
				{
					qDebug() << "Nickname" << nickname << "not found in MySql DB";
					return false;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed found nickname" << nickname << "in MySql DB because MySql DB not opened!";
			return false;
		}
	}

	return result;
}

int DBWorker::GetFreeUID()
{
	int uid = -1;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			// Get uids row and create new uid if uids row are empty
			QString buff = pRedis->GET("uids");

			if (buff.isEmpty())
			{
				qDebug() << "Key 'uids' not found! Creating key 'uids'...";

				if (pRedis->SET("uids", "100001"))
				{
					uid = 100001;
					return uid;
				}
				else
				{
					qCritical() << "Error creating key 'uids'!!!";
					return uid;
				}
			}
			else
			{
				int tmp = buff.toInt() + 1;

				qDebug() << "Key 'uids' found! Creating new uid = " << tmp;

				if (pRedis->SET("uids", QString::number(tmp)))
				{
					uid = tmp;
					qDebug() << "New uid created =" << uid;
					return uid;
				}
				else
				{
					qCritical() << "Error creating uid!";
					return uid;
				}
			}
		}
		else
		{
			qCritical() << "Failed found free uid in Redis DB because Redis DB not opened!";
			return uid;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM users WHERE uid=(SELECT MAX(uid) FROM users)");

			if (query->exec())
			{
				if (query->next())
				{
					int last_uid = query->value(0).toInt();

					qDebug() << "Last uid from table = " << last_uid;
					uid = last_uid + 1;
					return uid;
				}
				else
				{
					qDebug() << "Not any users in table, get first uid";
					uid = 100001;
					return uid;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return uid;
			}
		}
		else
		{
			qCritical() << "Failed found free uid in MySql DB because MySql DB not opened!";
			return uid;
		}
	}
	
	return uid;
}

int DBWorker::GetUIDbyNick(const QString &nickname)
{
	int uid = -1;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "nicknames:" + nickname;		
			QString buff = pRedis->GET(key);

			if (!buff.isEmpty())
			{
				qDebug() << "UID for" << nickname << "found in Redis DB";

				uid = buff.toInt();
				return uid;
			}
			else
			{
				qDebug() << "UID for" << nickname << "not found in Redis DB";
				return uid;
			}
		}
		else
		{
			qCritical() << "Failed found UID for " << nickname << "in Redis DB because Redis DB not connected!";
			return uid;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM profiles WHERE nickname=:nickname");
			query->bindValue(":nickname", nickname);

			if (query->exec())
			{
				if (query->next())
				{
					qDebug() << "UID for" << nickname << "found in MySql DB";
					uid = query->value(0).toInt();
					return uid;
				}
				else
				{
					qDebug() << "UID for" << nickname << "not found in MySql DB";
					return uid;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return uid;
			}
		}
		else
		{
			qCritical() << "Failed found UID for " << nickname << "in MySql DB because MySql DB not opened!";
			return uid;
		}
	}

	return uid;
}

SUser* DBWorker::GetUserData(const QString &login)
{
	SUser *dbUser = new SUser;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "users:" + login;
			QVector<std::pair<std::string, std::string>> result = pRedis->HGETALL(key);

			int dbUid = 0;
			QString dbLogin = QString();
			QString dbPassword = QString();
			int dbBanStatus = 0;

			if (result.size() > 0)
			{
				for (auto it = result.begin(); it!= result.end(); ++it)
				{
					if (it->first == "uid")
						dbUid = std::atoi(it->second.c_str());
					else if (it->first == "login")
						dbLogin = it->second.c_str();
					else if (it->first == "password")
						dbPassword = it->second.c_str();
					else if (it->first == "ban")
					{
						dbBanStatus = std::atoi(it->second.c_str());
						break;
					}
				}

				if (dbUid > 0 && !dbLogin.isEmpty() && !dbPassword.isEmpty())
				{
					qDebug() << "User data for" << login << "is found in Redis DB";

					dbUser->uid = dbUid;
					dbUser->login = dbLogin;
					dbUser->password = dbPassword;

					if (dbBanStatus > 0)
						dbUser->bBanStatus = true;
					else
						dbUser->bBanStatus = false;

					return dbUser;
				}
				else
				{
					qWarning() << "Wrong user data for" << login;
					return nullptr;
				}
			}
			else
			{
				qDebug() << "User data for" << login << "not found in Redis DB";
				return nullptr;
			}		
		}
		else
		{
			qCritical() << "Failed found login" << login << "in Redis DB because Redis DB not opened!";
			return nullptr;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM users WHERE login=:login");
			query->bindValue(":login", login);

			if (query->exec())
			{
				if (query->next())
				{
					qDebug() << "User data for" << login << "is found in MySql DB";
					dbUser->uid = query->value("uid").toInt(); // uid
					dbUser->login = query->value("login").toString(); // login
					dbUser->password = query->value("password").toString(); // password
					int dbBanStatus = query->value("ban").toInt(); // ban status

					if (dbBanStatus > 0)
						dbUser->bBanStatus = true;
					else
						dbUser->bBanStatus = false;

					return dbUser;
				}
				else
				{
					qDebug() << "User data for" << login << "not found in MySql DB";
					return nullptr;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return nullptr;
			}
		}
		else
		{
			qCritical() << "Failed found login" << login << "in MySql DB because MySql DB not opened!";
			return nullptr;
		}
	}
	
	return nullptr;
}

SProfile* DBWorker::GetUserProfile(int uid)
{
	SProfile *dbProfile = new SProfile;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "profiles:" + QString::number(uid);
			QVector<std::pair<std::string, std::string>> result = pRedis->HGETALL(key);

			int dbUid = 0;
			QString dbNickname = QString();
			QString dbModel = QString();
			int dbLvl = 0;
			int dbXp = 0;
			int dbMoney = 0;
			QString dbItems = QString();
			QString dbFriends = QString();


			if (result.size() > 0)
			{
				for (auto it = result.begin(); it != result.end(); ++it)
				{
					if (it->first == "uid")
						dbUid = std::atoi(it->second.c_str());
					else if (it->first == "nickname")
						dbNickname = it->second.c_str();
					else if (it->first == "fileModel")
						dbModel = it->second.c_str();
					else if (it->first == "lvl")
						dbLvl = std::atoi(it->second.c_str());
					else if (it->first == "xp")
						dbXp = std::atoi(it->second.c_str());
					else if (it->first == "money")
						dbMoney = std::atoi(it->second.c_str());
					else if (it->first == "items")
						dbItems = it->second.c_str();
					else if (it->first == "friends")
					{
						dbFriends = it->second.c_str();
						break;
					}
				}

				if (dbUid > 0 && !dbNickname.isEmpty() && !dbModel.isEmpty())
				{
					qDebug() << "Profile" << uid << "is found in Redis DB";

					dbProfile->uid = dbUid;
					dbProfile->nickname = dbNickname;
					dbProfile->fileModel = dbModel;
					dbProfile->lvl = dbLvl;
					dbProfile->xp = dbXp;
					dbProfile->money = dbMoney;
					dbProfile->items = dbItems;
					dbProfile->friends = dbFriends;

					return dbProfile;
				}
				else
				{
					qWarning() << "Wrong profile data for" << uid;
					return nullptr;
				}
			}
			else
			{
				qDebug() << "Profile" << uid << "not found in Redis DB";
				return nullptr;
			}		
		}
		else
		{
			qCritical() << "Failed found profile" << uid << "in Redis DB because Redis DB not connected!";
			return nullptr;
		}
	}

	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("SELECT * FROM profiles WHERE uid=:uid");
			query->bindValue(":uid", uid);

			if (query->exec())
			{
				if (query->next())
				{
					qDebug() << "Profile" << uid << "is found in MySql DB";

					dbProfile->uid = query->value("uid").toInt();
					dbProfile->nickname = query->value("nickname").toString();
					dbProfile->fileModel = query->value("fileModel").toString();
					dbProfile->lvl = query->value("lvl").toInt();
					dbProfile->xp = query->value("xp").toInt();
					dbProfile->money = query->value("money").toInt();
					dbProfile->items = query->value("items").toString();
					dbProfile->friends = query->value("friends").toString();

					return dbProfile;
				}
				else
				{
					qDebug() << "Profile" << uid << "not found in MySql DB";
					return nullptr;
				}
			}
			else
			{
				qWarning() << "Failed send query to MySql DB";
				return nullptr;
			}
		}
		else
		{
			qCritical() << "Failed found profile" << uid << "in MySql DB because MySql DB not opened!";
			return nullptr;
		}
	}
	
	return nullptr;
}

bool DBWorker::CreateUser(int uid, const QString &login, const QString &password)
{
	SettingsManager* pSettings = gEnv->pSettings;
	bool result = false;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "users:" + login;
			std::vector<std::pair<std::string, std::string>> field;

			std::pair<std::string, std::string> row_uid;
			row_uid.first = "uid";
			row_uid.second = std::to_string(uid);

			std::pair<std::string, std::string> row_login;
			row_login.first = "login";
			row_login.second = login.toStdString();
			
			std::pair<std::string, std::string> row_password;
			row_password.first = "password";
			row_password.second = password.toStdString();

			std::pair<std::string, std::string> row_ban;
			row_ban.first = "ban";
			row_ban.second = "0";

			field.push_back(row_uid);
			field.push_back(row_login);
			field.push_back(row_password);
			field.push_back(row_ban);

			result = pRedis->HMSET(key, field);
		}
		else
		{
			qCritical() << "Failed create user" << login << "in Redis DB because Redis DB not connected!";
			return false;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("INSERT INTO users (uid, login, password, ban) VALUES (:uid, :login, :password, :ban)");
			query->bindValue(":uid", uid);
			query->bindValue(":login", login);
			query->bindValue(":password", password);
			query->bindValue(":ban", 0);

			if (query->exec())
			{
				qDebug() << "User" << login << "created in MySql DB";
				result = true;
			}
			else
			{
				qWarning() << "Failed create user" << login << "in MySql DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed create user" << login << "in MySql DB because MySql DB not opened!";
			return false;
		}
	}
	
	// Redis background saving
	if (pRedis && pSettings->GetVariable("redis_bg_saving").toBool() && result)
		pRedis->BGSAVE();

	return result;
}

bool DBWorker::CreateProfile(SProfile *profile)
{
	SettingsManager* pSettings = gEnv->pSettings;
	bool result = false;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			std::pair<std::string, std::string> row_uid;
			row_uid.first = "uid";
			row_uid.second = std::to_string(profile->uid);

			std::pair<std::string, std::string> row_nickname;
			row_nickname.first = "nickname";
			row_nickname.second = profile->nickname.toStdString();

			std::pair<std::string, std::string> row_filemodel;
			row_filemodel.first = "fileModel";
			row_filemodel.second = profile->fileModel.toStdString();

			std::pair<std::string, std::string> row_lvl;
			row_lvl.first = "lvl";
			row_lvl.second = std::to_string(profile->lvl);

			std::pair<std::string, std::string> row_xp;
			row_xp.first = "xp";
			row_xp.second = std::to_string(profile->xp);

			std::pair<std::string, std::string> row_money;
			row_money.first = "money";
			row_money.second = std::to_string(profile->money);

			std::pair<std::string, std::string> row_items;
			row_items.first = "items";
			row_items.second = profile->items.toStdString();

			std::pair<std::string, std::string> row_friends;
			row_friends.first = "friends";
			row_friends.second = profile->friends.toStdString();

			QString key = "profiles:" + QString::number(profile->uid);
			QString key2 = "nicknames:" + profile->nickname;

			std::vector<std::pair<std::string, std::string>> field;

			field.push_back(row_uid);
			field.push_back(row_nickname);
			field.push_back(row_filemodel);
			field.push_back(row_lvl);
			field.push_back(row_xp);
			field.push_back(row_money);
			field.push_back(row_items);
			field.push_back(row_friends);

			if (pRedis->HMSET(key, field) && pRedis->SET(key2, QString::number(profile->uid)))
			{
				qDebug() << "Profile" << profile->nickname << "created in Redis DB";
				result = true;
			}
			else
			{
				qDebug() << "Failed create" << profile->nickname << " profile in Redis DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed create profile" << profile->nickname << "in Redis DB because Redis DB not opened!";
			return false;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("INSERT INTO profiles (uid, nickname, fileModel, lvl, xp, money, items, friends) "
				"VALUES (:uid, :nickname, :fileModel, :lvl, :xp, :money, :items, :friends)");
			query->bindValue(":uid", profile->uid);
			query->bindValue(":nickname", profile->nickname);
			query->bindValue(":fileModel", profile->fileModel);
			query->bindValue(":lvl", profile->lvl);
			query->bindValue(":xp", profile->xp);
			query->bindValue(":money", profile->money);
			query->bindValue(":items", profile->items);
			query->bindValue(":friends", profile->friends);


			if (query->exec())
			{
				qDebug() << "Profile" << profile->nickname << "created in MySql DB";
				result = true;
			}
			else
			{
				qWarning() << "Failed create profile" << profile->nickname << "in MySql DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed create profile" << profile->nickname << "in MySql DB because MySql DB not opened!";
			return false;
		}
	}
	
	// Redis background saving
	if (pRedis && pSettings->GetVariable("redis_bg_saving").toBool() && result)
		pRedis->BGSAVE();

	return result;
}

bool DBWorker::UpdateProfile(SProfile *profile)
{
	SettingsManager* pSettings = gEnv->pSettings;
	bool result = false;

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			std::pair<std::string, std::string> row_uid;
			row_uid.first = "uid";
			row_uid.second = std::to_string(profile->uid);

			std::pair<std::string, std::string> row_nickname;
			row_nickname.first = "nickname";
			row_nickname.second = profile->nickname.toStdString();

			std::pair<std::string, std::string> row_filemodel;
			row_filemodel.first = "fileModel";
			row_filemodel.second = profile->fileModel.toStdString();

			std::pair<std::string, std::string> row_lvl;
			row_lvl.first = "lvl";
			row_lvl.second = std::to_string(profile->lvl);

			std::pair<std::string, std::string> row_xp;
			row_xp.first = "xp";
			row_xp.second = std::to_string(profile->xp);

			std::pair<std::string, std::string> row_money;
			row_money.first = "money";
			row_money.second = std::to_string(profile->money);

			std::pair<std::string, std::string> row_items;
			row_items.first = "items";
			row_items.second = profile->items.toStdString();

			std::pair<std::string, std::string> row_friends;
			row_friends.first = "friends";
			row_friends.second = profile->friends.toStdString();

			QString key = "profiles:" + QString::number(profile->uid);
			QString key2 = "nicknames:" + profile->nickname;

			std::vector<std::pair<std::string, std::string>> field;

			field.push_back(row_uid);
			field.push_back(row_nickname);
			field.push_back(row_filemodel);
			field.push_back(row_lvl);
			field.push_back(row_xp);
			field.push_back(row_money);
			field.push_back(row_items);
			field.push_back(row_friends);

			if (pRedis->HMSET(key, field))
			{
				qDebug() << "Profile" << profile->nickname << "updated in Redis DB";
				result = true;
			}
			else
			{
				qDebug() << "Failed update profile" << profile->nickname << " in Redis DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed update profile" << profile->nickname << "in Redis DB because Redis DB not connected!";
			return false;
		}
	}
	
	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("UPDATE profiles SET nickname=:nickname, fileModel=:fileModel, lvl=:lvl, xp=:xp, money=:money, items=:items, friends=:friends WHERE uid=:uid");
			query->bindValue(":uid", profile->uid);
			query->bindValue(":nickname", profile->nickname);
			query->bindValue(":fileModel", profile->fileModel);
			query->bindValue(":lvl", profile->lvl);
			query->bindValue(":xp", profile->xp);
			query->bindValue(":money", profile->money);
			query->bindValue(":items", profile->items);
			query->bindValue(":friends", profile->friends);

			if (query->exec())
			{
				qDebug() << "Profile" << profile->nickname << "updated in MySql DB";
				result = true;
			}
			else
			{
				qWarning() << "Failed update profile" << profile->nickname << "in MySql DB";
				return false;
			}
		}
		else
		{
			qCritical() << "Failed update profile" << profile->nickname << "in MySql DB because MySql DB not opened!";
			return false;
		}
	}
	
	// Redis background saving
	if (pRedis && pSettings->GetVariable("redis_bg_saving").toBool() && result)
		pRedis->BGSAVE();

	return result;
}