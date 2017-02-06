// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#include <QObject>
#include <QSqlDatabase>

class MySqlConnector : public QObject
{
	Q_OBJECT
public:
	explicit MySqlConnector(QObject *parent = nullptr);
	~MySqlConnector();
public:
	void         run();
	void         Disconnect();
	QSqlDatabase GetDatabase();
	bool         IsConnected() { return bConnectStatus; }
private:
	bool	     Connect();
private:
	QSqlDatabase m_db;
	bool         bConnectStatus;
};

#endif // MYSQLCONNECTOR_H
