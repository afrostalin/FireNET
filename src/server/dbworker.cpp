#include "dbworker.h"
#include "global.h"
#include "redisconnector.h"
#include "mysqlconnector.h"
#include "clientquerys.h"
#include <QRegExp>
#include <QSqlQuery>

DBWorker::DBWorker(QObject *parent) : QObject(parent)
{

}

bool DBWorker::UserExists(QString login)
{
	bool result = false;
	// Redis
	QString key = "users:" + login;
	QList<QByteArray> rawCmd = { "HEXISTS", key.toUtf8(), "password"};
	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);

	if (buff.toInt() > 0)
	{
		qDebug() << "Login" << login << "finded in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Login" << login << "not found in Redis DB";
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}

bool DBWorker::ProfileExists(int uid)
{
	bool result = false;

	// Redis
	QString key = "profiles:" + QString::number(uid);
	QList<QByteArray> rawCmd = { "HEXISTS", key.toUtf8(), "nickname" };
	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);

	if (buff.toInt() > 0)
	{		
		qDebug() << "Profile" << uid << "finded in Redis DB";
		result = true;	
	}
	else
	{
		qDebug() << "Profile" << uid << "not found in Redis DB";
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}

bool DBWorker::NicknameExists(QString nickname)
{
	bool result = false;

	// Redis
	QString key = "nicknames:" + nickname;
	QString buff = gEnv->pRedis->SendSyncQuery("GET", key);

	if (!buff.isEmpty())
	{
		qDebug() << "Nickname" << nickname << "finded in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Nickname" << nickname << "not found in Redis DB";
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}

int DBWorker::GetFreeUID()
{
	RedisConnector* pRedis = gEnv->pRedis;
	int uid = -1;

	// Get uids row and create new uid if uids row are empty
	QString buff = pRedis->SendSyncQuery("GET", "uids");

	if (buff.isEmpty())
	{
		qDebug() << "Key 'uids' not found! Creating key 'uids'...";

		buff = pRedis->SendSyncQuery("SET", "uids", "100001");

		if (buff == "OK")
		{
			uid = 100001;
			return uid;
		}
		else
		{
			qCritical() << "Error creating key 'uids'!!!";
			return uid;
		}

		buff.clear();
	}
	else
	{
		int tmp = buff.toInt() + 1;

		qDebug() << "Key 'uids' found! Creating new uid = " << tmp;
		buff = pRedis->SendSyncQuery("SET", "uids", QString::number(tmp));

		if (buff == "OK")
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

int DBWorker::GetUIDbyNick(QString nickname)
{
	RedisConnector* pRedis = gEnv->pRedis;
	int uid = -1;

	// Get uids row and create new uid if uids row are empty
	QString buff = pRedis->SendSyncQuery("GET", "nicknames:" + nickname);

	if (!buff.isEmpty())
	{
		qDebug() << "UID for" << nickname << "found in Redis DB";

		uid = buff.toInt();
		return uid;
	}
	else
	{	
		qDebug() << "UID for" << nickname << "not found in Redis DB";
	}

	return uid;
}

SUser* DBWorker::GetUserData(QString login)
{
	SUser *dbUser = new SUser;
	// Redis
	QString key = "users:" + login;
	QList<QByteArray> rawCmd = { "HGETALL", key.toUtf8()};
	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);

	int dbUid = GetValueFromRawString("uid", buff).toInt();
	QString dbLogin = GetValueFromRawString("login", buff);
	QString dbPassword = GetValueFromRawString("password", buff);
	int dbBanStatus = GetValueFromRawString("ban", buff).toInt();
	
	if (dbUid > 0 && !dbLogin.isEmpty() && !dbPassword.isEmpty())
	{
		qDebug() << "User data for"<< login << "is found in Redis DB";

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
		qDebug() << "User data for" << login << "not found in Redis DB";
	}

	return nullptr;
}

SProfile* DBWorker::GetUserProfile(int uid)
{
	SProfile *dbProfile = new SProfile;
	// Redis
	QString key = "profiles:" + QString::number(uid);

	QList<QByteArray> rawCmd = { "HGETALL", key.toUtf8() };
	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);

	int dbUid = GetValueFromRawString("uid", buff).toInt();
	QString dbNickname = GetValueFromRawString("nickname", buff);
	QString dbModel = GetValueFromRawString("model", buff);
	int dbLvl = GetValueFromRawString("lvl", buff).toInt();
	int dbXp = GetValueFromRawString("xp", buff).toInt();
	int dbMoney = GetValueFromRawString("money", buff).toInt();
	QString dbItems = GetValueFromRawString("items", buff);
	QString dbFriends = GetValueFromRawString("friends", buff);
	QString dbAchievements = GetValueFromRawString("achievements", buff);
	QString dbStats = GetValueFromRawString("stats", buff);

	if (dbUid > 0 && !dbNickname.isEmpty() && !dbModel.isEmpty())
	{
		qDebug() << "Profile" << uid << "is found in Redis DB";

		dbProfile->uid = dbUid;
		dbProfile->nickname = dbNickname;
		dbProfile->model = dbModel;
		dbProfile->lvl = dbLvl;
		dbProfile->xp = dbXp;
		dbProfile->money = dbMoney;
		dbProfile->items = dbItems;
		dbProfile->friends = dbFriends;
		dbProfile->achievements = dbAchievements;
		dbProfile->stats = dbStats;

		return dbProfile;
	}
	else
	{
		qDebug() << "Profile" << uid << "not found in Redis DB";
	}

	return nullptr;
}

bool DBWorker::CreateUser(int uid, QString login, QString password)
{
	bool result = false;

	QString key = "users:" + login;
	QList<QByteArray> rawCmd = {"HMSET", key.toUtf8(), "uid", QString::number(uid).toUtf8(), "login", login.toUtf8(), "password", password.toUtf8()};
	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);

	if (buff == "OK")
	{
		qDebug() << "User" << login << "created in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Failed create user" << login << " in Redis DB";
		return false;
	}
	

	// MySql
	if (gEnv->bUseMySql && result && gEnv->pMySql != nullptr)
	{
		if (gEnv->pMySql->connectStatus)
		{
			QSqlQuery *query = new QSqlQuery(gEnv->pMySql->GetDatabase());
			query->prepare("INSERT INTO users (uid, login, password) "
				"VALUES (:uid, :login, :password)");
			query->bindValue(":uid", uid);
			query->bindValue(":login", login);
			query->bindValue(":password", password);

			if (query->exec())
			{
				qDebug() << "User" << login << "created in MySql DB";
				result = true;
			}
			else
			{
				qDebug() << "Failed create user" << login << "in MySql DB";
				result = false;
			}
		}
		else
		{
			qDebug() << "Failed create user" << login << "in MySql DB because MySql DB not opened!";
			result = false;
		}
	}

	// Redis background saving
	if (gEnv->bRedisBackgroundSave && result)
	{
		QString save_buff = gEnv->pRedis->SendSyncQuery("BGSAVE");
		if (!save_buff.isEmpty())
		{
			qDebug() << "Redis background saving complete";
			result = true;
		}
		else
		{
			qDebug() << "Redis background saving failed";
			result = false;
		}
	}

	return result;
}

bool DBWorker::CreateProfile(SProfile *profile)
{
	bool result = false;

	// Redis
	QString key = "profiles:" + QString::number(profile->uid);

	QList<QByteArray> rawCmd = { "HMSET", key.toUtf8(), 
	"uid", QString::number(profile->uid).toUtf8(),
	"nickname", profile->nickname.toUtf8(),
	"model", profile->model.toUtf8(),
	"lvl", QString::number(profile->lvl).toUtf8(),
	"xp", QString::number(profile->xp).toUtf8(),
	"money", QString::number(profile->money).toUtf8(),
	"items", profile->items.toUtf8(),
	"friends", profile->friends.toUtf8(),
	"achievements", profile->achievements.toUtf8(),
	"stats", profile->stats.toUtf8()};

	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);
	QString buff2 = gEnv->pRedis->SendSyncQuery("SET", "nicknames:" + profile->nickname, QString::number(profile->uid));

	if (buff == "OK" && buff2 == "OK")
	{
		qDebug() << "Profile" << profile->nickname << "created in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Failed create"<< profile->nickname <<" profile in Redis DB";
		return false;
	}

	// MySql
	if (gEnv->bUseMySql && result && gEnv->pMySql != nullptr)
	{
		if (gEnv->pMySql->connectStatus)
		{
			QSqlQuery *query = new QSqlQuery(gEnv->pMySql->GetDatabase());
			query->prepare("INSERT INTO profiles (uid, nickname, model, lvl, xp, money, items, friends, achievements, stats) "
				"VALUES (:uid, :nickname, :model, :lvl, :xp, :money, :items, :friends, :achievements, :stats)");
			query->bindValue(":uid", profile->uid);
			query->bindValue(":nickname", profile->nickname);
			query->bindValue(":model", profile->model);
			query->bindValue(":lvl", profile->lvl);
			query->bindValue(":xp", profile->xp);
			query->bindValue(":money", profile->money);
			query->bindValue(":items", profile->items);
			query->bindValue(":friends", profile->friends);
			query->bindValue(":achievements", profile->achievements);
			query->bindValue(":stats", profile->stats);


			if (query->exec())
			{
				qDebug() << "Profile" << profile->nickname << "created in MySql DB";
				result = true;
			}
			else
			{
				qDebug() << "Failed create profile" << profile->nickname << "in MySql DB";
				result = false;
			}
		}
		else
		{
			qDebug() << "Failed create profile" << profile->nickname << "in MySql DB because MySql DB not opened!";
			result = false;
		}
	}

	// Redis background saving
	if (gEnv->bRedisBackgroundSave && result)
	{
		QString save_buff = gEnv->pRedis->SendSyncQuery("BGSAVE");
		if (!save_buff.isEmpty())
		{
			qDebug() << "Redis background saving complete";
			result = true;
		}
		else
		{
			qDebug() << "Redis background saving failed";
			result = false;
		}
	}

	return result;
}

bool DBWorker::UpdateProfile(SProfile *profile)
{
	bool result = false;

	// Redis
	QString key = "profiles:" + QString::number(profile->uid);

	QList<QByteArray> rawCmd = { "HMSET", key.toUtf8(),
		"uid", QString::number(profile->uid).toUtf8(),
		"nickname", profile->nickname.toUtf8(),
		"model", profile->model.toUtf8(),
		"lvl", QString::number(profile->lvl).toUtf8(),
		"xp", QString::number(profile->xp).toUtf8(),
		"money", QString::number(profile->money).toUtf8(),
		"items", profile->items.toUtf8(),
		"friends", profile->friends.toUtf8(),
		"achievements", profile->achievements.toUtf8(),
		"stats", profile->stats.toUtf8() };

	QString buff = gEnv->pRedis->SendSyncQuery(rawCmd);

	if (buff == "OK")
	{
		qDebug() << "Profile" << profile->nickname << "updated in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Failed update profile" << profile->nickname << " in Redis DB";
		return false;
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	// Redis background saving
	if (gEnv->bRedisBackgroundSave && result)
	{
		QString save_buff = gEnv->pRedis->SendSyncQuery("BGSAVE");
		if (!save_buff.isEmpty())
		{
			qDebug() << "Redis background saving complete";
			result = true;
		}
		else
		{
			qDebug() << "Redis background saving failed";
			result = false;
		}
	}

	return result;
}

QString DBWorker::GetValueFromRawString(const char * valuename, QString rawString)
{
	if (rawString.isEmpty())
		return QString();

	QRegExp RN("\r\n");
	QStringList strList;
	strList = rawString.split(RN);

	for (int i = 0; i < strList.size(); ++i)
	{
		QString value =  strList[i];

		if (value == valuename)
		{
			QString result = strList[i + 2];
			return result;
		}
	}

	return QString();
}