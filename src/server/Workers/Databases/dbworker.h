// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>

#include "global.h"

class RedisConnector;
class MySqlConnector;

class DBWorker : public QObject
{
    Q_OBJECT
public:
    explicit DBWorker(QObject *parent = 0);
	~DBWorker();
public:
	void            Init();
	void            Clear() const;
public:
	bool            UserExists(const QString &login);
	bool            ProfileExists(int uid);
	bool            NicknameExists(const QString &nickname);
public:
	int             GetFreeUID();
	int             GetUIDbyNick(const QString &nickname);
	SUser*          GetUserData(const QString &login);
	SProfile*       GetUserProfile(int uid);
public:
	bool            CreateUser(int uid, const QString &login, const QString &password);
	// If password = nullptr - used password from user data
	bool            UpdateUser(const QString &login, const QString &password, bool banned);
	bool            CreateProfile(SProfile *profile);
	bool            UpdateProfile(SProfile *profile);
public:
	RedisConnector* pRedis;
	MySqlConnector* pMySql;
};