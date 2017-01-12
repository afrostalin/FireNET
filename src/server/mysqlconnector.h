// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#include <QObject>
#include <QThread>
#include <QSqlDatabase>

class MySqlConnector : public QObject
{
	Q_OBJECT
public:
	explicit MySqlConnector(QObject *parent = 0);
public:
	void run();
	void Disconnect();
private:
	bool Connect();
public:
	bool connectStatus;
	QSqlDatabase GetDatabase();
private:
	QThread *m_thread;
	QSqlDatabase m_db;
};

#endif // MYSQLCONNECTOR_H
