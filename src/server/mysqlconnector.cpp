// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "mysqlconnector.h"
#include "dbworker.h"
#include "settings.h"

#include <QDebug>

MySqlConnector::MySqlConnector(QObject *parent) : QObject(parent)
{
	connectStatus = false;
	m_thread = nullptr;
}

void MySqlConnector::run()
{
	if (Connect())
	{
		qInfo() << "MySQL connected!";

		m_thread = QThread::currentThread();

		qInfo() << "MySQL work on " << m_thread;

		connectStatus = true;
	}
	else
	{
		qCritical() << "Failed connect to MySQL! MySQL functions not be work!";
		connectStatus = false;
		return;
	}
}

void MySqlConnector::Disconnect()
{
	if (connectStatus)
	{
		qInfo() << "Disconnecting...";
		m_db.close();
	}
}

bool MySqlConnector::Connect()
{
	qDebug() << "Connecting to MySql host...(" << gEnv->pSettings->GetVariable("mysql_host").toString() << ":" << gEnv->pSettings->GetVariable("mysql_port").toInt() << ")";

	m_db = QSqlDatabase::addDatabase("QMYSQL", "mySqlDatabase");
	m_db.setHostName(gEnv->pSettings->GetVariable("mysql_host").toString());
	m_db.setPort(gEnv->pSettings->GetVariable("mysql_port").toInt());
	m_db.setDatabaseName(gEnv->pSettings->GetVariable("mysql_db_name").toString());
	m_db.setUserName(gEnv->pSettings->GetVariable("mysql_username").toString());
	m_db.setPassword(gEnv->pSettings->GetVariable("mysql_password").toString());

	return m_db.open();
}

QSqlDatabase MySqlConnector::GetDatabase()
{
	return m_db;
}


