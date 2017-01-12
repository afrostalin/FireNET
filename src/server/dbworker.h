// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef DBWORKER_H
#define DBWORKER_H

#include <QObject>
#include "global.h"

class RedisConnector;
class MySqlConnector;
class HttpConnector;

class DBWorker : public QObject
{
    Q_OBJECT
public:
    explicit DBWorker(QObject *parent = 0);
public:
	void Init();
	void Clear();
public slots:
	void Update();
public:
	// Check if login is there in database
	bool UserExists(QString login);
	// Check if profile is there in database
	bool ProfileExists(int uid);
	// Check if nickname is there in database
	bool NicknameExists(QString nickname);

public:
	// Get unique id for new user
	int GetFreeUID();
	// Get user uid by nickname
	int GetUIDbyNick(QString nickname);
	// Get user data
	SUser* GetUserData(QString login);
	// Get user profile
	SProfile* GetUserProfile(int uid);

public:
	// Create new user
	bool CreateUser(int uid, QString login, QString password);
	// Create new profile
	bool CreateProfile(SProfile *profile);

public:
	// Update profile
	bool UpdateProfile(SProfile *profile);

private:
	QString GetValueFromRawString(const char* valuename, QString rawString);
public:
	RedisConnector* pRedis;
	MySqlConnector* pMySql;
	HttpConnector* pHTTP;
};

#endif // DBWORKER_H