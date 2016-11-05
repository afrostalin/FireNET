// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

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
