// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "dbworker.h"
#include "redisconnector.h"
#include "mysqlconnector.h"

#include "Tools/console.h"

#include <QSqlQuery>

DBWorker::DBWorker(QObject *parent) 
	: QObject(parent)
	, pRedis(nullptr)
	, pMySql(nullptr)
{
}

DBWorker::~DBWorker()
{
	SAFE_RELEASE(pRedis);
	SAFE_RELEASE(pMySql);
}

void DBWorker::Clear() const
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
	gEnv->m_ServerStatus.m_DBStatus = "Init";

	// Create Redis connection
	if (gEnv->pConsole->GetBool("bUseRedis"))
	{
		LogInfo("Start Redis service...");
		pRedis = new RedisConnector;
		pRedis->run();
	}

	// Create MySQL connection
	if (gEnv->pConsole->GetBool("bUseMySQL"))
	{
		LogInfo("Start MySql service...");
		gEnv->pConsole->SetVariable("redis_bg_saving", true);
		pMySql = new MySqlConnector;
		pMySql->run();
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
			LogError("Failed found user <%s> in Redis DB, bacause Redis DB not connected!", login.toStdString().c_str());
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
					LogDebug("Login <%s> found in MySql DB", login.toStdString().c_str());
					result = true;
				}
				else
				{
					LogDebug("Login <%s> not found in MySql DB", login.toStdString().c_str());
					return false;
				}
			}
			else
			{
				LogWarning("Failed send query to MySql DB");
				return false;
			}
		}
		else
		{
			LogError("Failed found login <%s> in MySql DB becuse MySql DB not opened!", login.toStdString().c_str());
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
			LogError("Failed found profile <%d> in Redis DB because Redis DB not connected!", uid);
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
					LogDebug("Profile <%d> found in MySql DB", uid);
					result = true;
				}
				else
				{
					LogDebug("Profile <%d> not found in MySql DB", uid);
					return false;
				}
			}
			else
			{
				LogWarning("Failed send query to MySql DB");
				return false;
			}
		}
		else
		{
			LogError("Failed found profile <%d> in MySql DB because MySql DB not opened!", uid);
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
				LogDebug("Nickname <%s> found in Redis DB", nickname.toStdString().c_str());
				result = true;
			}
			else
			{
				LogDebug("Nickname <%s> not found in Redis DB", nickname.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed found nickname <%s> in Redis DB because Redis DB not connected!", nickname.toStdString().c_str());
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
					LogDebug("Nickname <%s> found in MySql DB", nickname.toStdString().c_str());
					result = true;
				}
				else
				{
					LogDebug("Nickname <%s> not found in MySql DB", nickname.toStdString().c_str());
					return false;
				}
			}
			else
			{
				LogWarning("Failed send query to MySql DB");
				return false;
			}
		}
		else
		{
			LogError("Failed found nickname <%s> in MySql DB because MySql DB bacuse MySql DB not opened!", nickname.toStdString().c_str());
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
				LogDebug("Key 'uids' not found! Creating key 'uids'...");

				if (pRedis->SET("uids", "100001"))
				{
					uid = 100001;
					return uid;
				}
				LogError("Error creating key 'uids'!!!");
				return uid;
			}
			int tmp = buff.toInt() + 1;

			LogDebug("Key 'uids' found! Creating new uid = %d", tmp);

			if (pRedis->SET("uids", QString::number(tmp)))
			{
				uid = tmp;
				LogDebug("New uid created = <%d>", uid);
				return uid;
			}
			LogError("Error creating uid!");
			return uid;
		}
		LogError("Failed found free uid in Redis DB because Redis DB not opened!");
		return uid;
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

					LogDebug("Last uid from table = %d", last_uid);

					uid = last_uid + 1;
					return uid;
				}
				LogDebug("Not any users in table, get first uid");

				uid = 100001;
				return uid;
			}
			LogWarning("Failed send query to MySql DB");
			return uid;
		}
		LogWarning("Failed found free uid in MySql DB because MySql DB not opened!");
		return uid;
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
				LogDebug("UID for <%s> found in Redis DB", nickname.toStdString().c_str());

				uid = buff.toInt();
				return uid;
			}
			LogDebug("UID for <%s> not found in Redis DB", nickname.toStdString().c_str());
			return uid;
		}
		LogError("Failed found UID for <%s> in Redis DB because Redis DB not connected!", nickname.toStdString().c_str());
		return uid;
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
					LogDebug("UID for <%s> found in MySql DB", nickname.toStdString().c_str());

					uid = query->value(0).toInt();
					return uid;
				}
				LogDebug("UID for <%s> not found in MySql DB", nickname.toStdString().c_str());
				return uid;
			}
			LogWarning("Failed send query to MySql DB");
			return uid;
		}
		LogError("Failed found UID for <%s> in MySql DB because MySql DB not opened!", nickname.toStdString().c_str());
		return uid;
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
						dbUid = atoi(it->second.c_str());
					else if (it->first == "login")
						dbLogin = it->second.c_str();
					else if (it->first == "password")
						dbPassword = it->second.c_str();
					else if (it->first == "ban")
					{
						dbBanStatus = atoi(it->second.c_str());
						break;
					}
				}

				if (dbUid > 0 && !dbLogin.isEmpty() && !dbPassword.isEmpty())
				{
					LogDebug("User data for <%s> is found in Redis DB", login.toStdString().c_str());

					dbUser->uid = dbUid;
					dbUser->login = dbLogin;
					dbUser->password = dbPassword;

					if (dbBanStatus > 0)
						dbUser->bBanStatus = true;
					else
						dbUser->bBanStatus = false;

					return dbUser;
				}
				LogWarning("Wrong user data for <%s>", login.toStdString().c_str());
				return nullptr;
			}
			LogDebug("User data for <%s> not found in Redis DB", login.toStdString().c_str());
			return nullptr;
		}
		LogError("Failed found login <%s> in Redis DB bacause Redis DB not opened!", login.toStdString().c_str());
		return nullptr;
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
					LogDebug("User data for <%s> is found in MySql DB", login.toStdString().c_str());

					dbUser->uid = query->value("uid").toInt();              // uid
					dbUser->login = query->value("login").toString();       // login
					dbUser->password = query->value("password").toString(); // password
					int dbBanStatus = query->value("ban").toInt();          // ban status

					if (dbBanStatus > 0)
						dbUser->bBanStatus = true;
					else
						dbUser->bBanStatus = false;

					return dbUser;
				}
				LogDebug("User data for <%s> not found in MySql DB", login.toStdString().c_str());
				return nullptr;
			}
			LogWarning("Failed send query to MySql DB");
			return nullptr;
		}
		LogError("Failed found login <%s> in MySql DB because MySql DB not opened!", login.toStdString().c_str());
		return nullptr;
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
#ifndef STEAM_SDK_ENABLED
			QString dbFriends = QString();
#endif


			if (result.size() > 0)
			{
				for (auto it = result.begin(); it != result.end(); ++it)
				{
					if (it->first == "uid")
						dbUid = atoi(it->second.c_str());
					else if (it->first == "nickname")
						dbNickname = it->second.c_str();
					else if (it->first == "fileModel")
						dbModel = it->second.c_str();
					else if (it->first == "lvl")
						dbLvl = atoi(it->second.c_str());
					else if (it->first == "xp")
						dbXp = atoi(it->second.c_str());
					else if (it->first == "money")
						dbMoney = atoi(it->second.c_str());
					else if (it->first == "items")
						dbItems = it->second.c_str();
#ifndef STEAM_SDK_ENABLED
					else if (it->first == "friends")
					{
						dbFriends = it->second.c_str();
						break;
					}
#endif
				}

				if (dbUid > 0 && !dbNickname.isEmpty() && !dbModel.isEmpty())
				{
					LogDebug("Profile <%d> is found in Redis DB", uid);

					dbProfile->uid = dbUid;
					dbProfile->nickname = dbNickname;
					dbProfile->fileModel = dbModel;
					dbProfile->lvl = dbLvl;
					dbProfile->xp = dbXp;
					dbProfile->money = dbMoney;
					dbProfile->items = dbItems;
#ifndef STEAM_SDK_ENABLED
					dbProfile->friends = dbFriends;
#endif

					return dbProfile;
				}
				LogWarning("Wrong profile data for <%d>", uid);
				return nullptr;
			}
			LogDebug("Profile <%d> not found in Redis DB", uid);
			return nullptr;
		}
		LogError("Failed found profile <%d> in Redis DB because Redis DB not connected!", uid);
		return nullptr;
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
					LogDebug("Profile <%d> is found in MySql DB", uid);

					dbProfile->uid = query->value("uid").toInt();
					dbProfile->nickname = query->value("nickname").toString();
					dbProfile->fileModel = query->value("fileModel").toString();
					dbProfile->lvl = query->value("lvl").toInt();
					dbProfile->xp = query->value("xp").toInt();
					dbProfile->money = query->value("money").toInt();
					dbProfile->items = query->value("items").toString();
#ifndef STEAM_SDK_ENABLED
					dbProfile->friends = query->value("friends").toString();
#endif

					return dbProfile;
				}
				LogDebug("Profile <%d> not found in MySql DB", uid);
				return nullptr;
			}
			LogWarning("Failed send query to MySql DB");
			return nullptr;
		}
		LogError("Failed found profile <%d> in MySql DB because MySql DB not opened!", uid);
		return nullptr;
	}
	
	return nullptr;
}

bool DBWorker::CreateUser(int uid, const QString &login, const QString &password)
{
	CConsole* pConsole = gEnv->pConsole;
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
			LogError("Failed create user <%s> in Redis DB because Redis DB not connected!", login.toStdString().c_str());
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
				LogDebug("User <%s> created in MySql DB", login.toStdString().c_str());
				result = true;
			}
			else
			{
				LogDebug("Failed create user <%s> in MySql DB", login.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed create user <%s> in MySql DB because MySql DB not opened!", login.toStdString().c_str());
			return false;
		}
	}
	
	// Redis background saving
	if (pRedis && pConsole->GetBool("redis_bg_saving") && result)
		pRedis->BGSAVE();

	return result;
}

bool DBWorker::UpdateUser(const QString & login, const QString & password, bool banned)
{
	CConsole* pConsole = gEnv->pConsole;
	bool result = false;

	// Get user data before
	SUser* pUserData = GetUserData(login);
	if (pUserData == nullptr)
	{
		LogError("Failed update user <%s> because can't get user data!", login.toStdString().c_str());
	}

	// Redis
	if (pRedis)
	{
		if (pRedis->IsConnected())
		{
			QString key = "users:" + login;
			std::vector<std::pair<std::string, std::string>> field;

			std::pair<std::string, std::string> row_uid;
			row_uid.first = "uid";
			row_uid.second = std::to_string(pUserData->uid);

			std::pair<std::string, std::string> row_login;
			row_login.first = "login";
			row_login.second = login.toStdString();

			std::pair<std::string, std::string> row_password;
			if (!password.isEmpty())
			{
				row_password.first = "password";
				row_password.second = password.toStdString();
			}
			else
			{
				row_password.first = "password";
				row_password.second = pUserData->password.toStdString();
			}

			std::pair<std::string, std::string> row_ban;
			row_ban.first = "ban";
			row_ban.second = std::to_string(banned ? 1 : 0);

			field.push_back(row_uid);
			field.push_back(row_login);
			field.push_back(row_password);
			field.push_back(row_ban);

			result = pRedis->HMSET(key, field);
		}
		else
		{
			LogError("Failed update user <%s> in Redis DB because Redis DB not conneted!", login.toStdString().c_str());
			return false;
		}
	}

	// MySql
	if (pMySql)
	{
		if (pMySql->IsConnected())
		{
			QSqlQuery *query = new QSqlQuery(pMySql->GetDatabase());
			query->prepare("UPDATE users SET login=:login, password=:password, ban=:ban WHERE uid=:uid");
			query->bindValue(":uid", pUserData->uid);
			query->bindValue(":login", login);
			if (!password.isEmpty())
			{
				query->bindValue(":password", password);
			}
			else
			{
				query->bindValue(":password", pUserData->password);
			}
			query->bindValue(":ban", banned ? 1 : 0);

			if (query->exec())
			{
				LogDebug("User <%s> updated in MySql DB", login.toStdString().c_str());
				result = true;
			}
			else
			{
				LogWarning("Failed update user <%s> in MySql DB", login.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed update user <%s> in MySql DB because MySql DB not opened!", login.toStdString().c_str());
			return false;
		}
	}

	// Redis background saving
	if (pRedis && pConsole->GetVariable("redis_bg_saving").toBool() && result)
	{
		pRedis->BGSAVE();
	}

	return result;
}

bool DBWorker::CreateProfile(SProfile *profile)
{
	CConsole* pConsole = gEnv->pConsole;
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
#ifndef STEAM_SDK_ENABLED
			std::pair<std::string, std::string> row_friends;
			row_friends.first = "friends";
			row_friends.second = profile->friends.toStdString();
#endif

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
#ifndef STEAM_SDK_ENABLED
			field.push_back(row_friends);
#endif

			if (pRedis->HMSET(key, field) && pRedis->SET(key2, QString::number(profile->uid)))
			{
				LogDebug("Profile <%s> created in Redis DB", profile->nickname.toStdString().c_str());
				result = true;
			}
			else
			{
				LogDebug("Failed create <%s> profile in Redis DB", profile->nickname.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed create profile <%s> in Redis DB because Redis DB not opened!", profile->nickname.toStdString().c_str());
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
#ifndef STEAM_SDK_ENABLED
			query->bindValue(":friends", profile->friends);
#endif

			if (query->exec())
			{
				LogDebug("Profile <%s> created in MySql DB", profile->nickname.toStdString().c_str());
				result = true;
			}
			else
			{
				LogWarning("Failed create profile <%s> in MySql DB", profile->nickname.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed create profile <%s> in MySql DB because MySql DB not opened!", profile->nickname.toStdString().c_str());
			return false;
		}
	}
	
	// Redis background saving
	if (pRedis && pConsole->GetVariable("redis_bg_saving").toBool() && result)
		pRedis->BGSAVE();

	return result;
}

bool DBWorker::UpdateProfile(SProfile *profile)
{
	CConsole* pConsole = gEnv->pConsole;
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
#ifndef STEAM_SDK_ENABLED
			std::pair<std::string, std::string> row_friends;
			row_friends.first = "friends";
			row_friends.second = profile->friends.toStdString();
#endif

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
#ifndef STEAM_SDK_ENABLED
			field.push_back(row_friends);
#endif

			if (pRedis->HMSET(key, field))
			{
				LogDebug("Profile <%s> updated in Redis DB", profile->nickname.toStdString().c_str());
				result = true;
			}
			else
			{
				LogDebug("Failed update profile <%s> in Redis DB", profile->nickname.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed update profile <%s> in Redis DB because Redis DB not connected!", profile->nickname.toStdString().c_str());
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
#ifndef STEAM_SDK_ENABLED
			query->bindValue(":friends", profile->friends);
#endif

			if (query->exec())
			{
				LogDebug("Profile <%s> updated in MySql DB", profile->nickname.toStdString().c_str());
				result = true;
			}
			else
			{
				LogWarning("Failed update profile <%s> in MySql DB", profile->nickname.toStdString().c_str());
				return false;
			}
		}
		else
		{
			LogError("Failed update profile <%s> in MySql DB because MySql DB not opened!", profile->nickname.toStdString().c_str());
			return false;
		}
	}
	
	// Redis background saving
	if (pRedis && pConsole->GetBool("redis_bg_saving") && result)
		pRedis->BGSAVE();

	return result;
}