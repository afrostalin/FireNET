#include "dbworker.h"
#include "global.h"
#include "redisconnector.h"
#include "clientquerys.h"

DBWorker::DBWorker(QObject *parent) : QObject(parent)
{

}

bool DBWorker::UserExists(QString &login)
{
	bool result = false;
	// Redis
	QString key = "users:" + login;
	QString buff = gEnv->pRedis->SendSyncQuery("GET", key, "");

	if (!buff.isEmpty())
	{
		qDebug() << "Login " << login << "finded in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Login " << login << "not found in Redis DB";
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}

bool DBWorker::ProfileExists(QString &nickname)
{
	bool result = false;
	// Redis
	QString key = "profiles:*:" + nickname;
	QString buff = gEnv->pRedis->SendSyncQuery("keys", key, "");

	if (!buff.isEmpty())
	{		
		qDebug() << "Profile " << nickname << "finded in Redis DB";
		result = true;	
	}
	else
	{
		qDebug() << "Profile " << nickname << "not found in Redis DB";
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
	QString buff = pRedis->SendSyncQuery("GET", "uids", "");

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
			qDebug() << "New uid created = " << uid;
			return uid;
		}
		else
		{
			qCritical() << "Error creating uid!";
			return uid;
		}
	}
}

QString DBWorker::GetUserData(QString &login)
{
	// Redis
	QString key = "users:" + login;
	QString buff = gEnv->pRedis->SendSyncQuery("GET", key, "");

	if (!buff.isEmpty())
	{
		qDebug() << "User data for "<< login << "is found in Redis DB";
		return buff;
	}
	else
	{
		qDebug() << "User data for " << login << "not found in Redis DB";
	}

	return QString();
}

QString DBWorker::GetUserProfile(int uid)
{
	// Redis
	QString keys = "profiles:" + QString::number(uid) + ":*";
	QString key = gEnv->pRedis->SendSyncQuery("keys", keys, "");

	if (!key.isEmpty())
	{
		QString buff = gEnv->pRedis->SendSyncQuery("GET", key, "");

		if (!buff.isEmpty())
		{
			qDebug() << "Profile " << uid << "is found in Redis DB";
			return buff;
		}
		else
		{
			qDebug() << "Profile" << uid << "not found in Redis DB";
		}
	}

	return QString();
}

int DBWorker::GetUIDbyNickname(QString nickname)
{
	// Redis
	QString key = "profiles:*:" + nickname;
	QString buff = gEnv->pRedis->SendSyncQuery("keys", key, "");

	if (!buff.isEmpty())
	{
		qDebug() << "Profile " << nickname << "finded in Redis DB";
		qDebug() << buff;
		buff.remove("profiles:");
		buff.remove(":" + nickname);

		int uid = buff.toInt();
		qDebug() << uid;
		
		return uid;
	}
	else
	{
		qDebug() << "Profile " << nickname << "not found in Redis DB";
	}

	return -1;
}

bool DBWorker::CreateUser(int uid, QString &login, QString &password)
{
	bool result = false;

	// Redis
	QString key = "users:" + login;
	QString value = "<data uid = '" + QString::number(uid) + "' login='" + login + "' password = '" + password + "' ban='0'/>";
	QString buff = gEnv->pRedis->SendSyncQuery("SET", key, value);

	if (buff == "OK")
	{
		qDebug() << "User " << login << "created in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Failed create new user in Redis DB";
		return false;
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}

bool DBWorker::CreateProfile(QString &stringProfile, SProfile &profile)
{
	bool result = false;

	// Redis
	QString key = "profiles:" + QString::number(profile.uid) + ":" + profile.nickname;
	QString buff = gEnv->pRedis->SendSyncQuery("SET", key, stringProfile);

	if (buff == "OK")
	{
		qDebug() << "User " << profile.nickname << "created in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Failed create new user in Redis DB";
		return false;
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}

bool DBWorker::UpdateProfile(QString & stringProfile, SProfile & profile)
{
	bool result = false;

	// Redis
	QString key = "profiles:" + QString::number(profile.uid) + ":" + profile.nickname;
	QString buff = gEnv->pRedis->SendSyncQuery("SET", key, stringProfile);

	if (buff == "OK")
	{
		qDebug() << "User " << profile.nickname << "updated in Redis DB";
		result = true;
	}
	else
	{
		qDebug() << "Failed update " << profile.nickname << " in Redis DB";
		return false;
	}

	// MySql
	if (gEnv->bUseMySql && result)
	{
		// do smth
		//result = false;
	}

	return result;
}
