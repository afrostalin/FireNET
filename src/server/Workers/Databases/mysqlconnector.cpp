// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QThread>

#include "global.h"
#include "mysqlconnector.h"

#include "Tools/console.h"

MySqlConnector::MySqlConnector(QObject *parent) 
	: QObject(parent)
	, bConnectStatus(false)
{
}

MySqlConnector::~MySqlConnector()
{
	m_db.close();
}

void MySqlConnector::run()
{
	gEnv->m_ServerStatus.m_DBStatus = "connecting";

	if (Connect())
	{
		LogInfo("MySQL connected. Work on <%p>", QThread::currentThread());
		bConnectStatus = true;
		gEnv->m_ServerStatus.m_DBStatus = "online";
	}
	else
	{
		LogError("Failed connect to MySQL! MySQL functions not be work!");
		gEnv->m_ServerStatus.m_DBStatus = "offline";
		return;
	}
}

void MySqlConnector::Disconnect()
{
	gEnv->m_ServerStatus.m_DBStatus = "offline";

	if (bConnectStatus)
	{
		LogDebug("Disconnecting...");
		m_db.close();
	}
}

bool MySqlConnector::Connect()
{
	LogDebug("Connecting to MySql host ... (%s : %d)", gEnv->pConsole->GetString("mysql_host").c_str(), gEnv->pConsole->GetInt("mysql_port"));

	m_db = QSqlDatabase::addDatabase("QMYSQL", "mySqlDatabase");
	m_db.setHostName(gEnv->pConsole->GetVariable("mysql_host").toString());
	m_db.setPort(gEnv->pConsole->GetVariable("mysql_port").toInt());
	m_db.setDatabaseName(gEnv->pConsole->GetVariable("mysql_db_name").toString());
	m_db.setUserName(gEnv->pConsole->GetVariable("mysql_username").toString());
	m_db.setPassword(gEnv->pConsole->GetVariable("mysql_password").toString());

	return m_db.open();
}

QSqlDatabase MySqlConnector::GetDatabase() const
{
	return m_db;
}
