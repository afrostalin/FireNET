#include "mysqlconnector.h"
#include "global.h"
#include <QDebug>

MySqlConnector::MySqlConnector(QObject *parent)
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

bool MySqlConnector::Connect()
{
	qDebug() << "Connecting to MySql host...(" << gEnv->mySqlHost << ":" << gEnv->mySqlPort << ")";

	m_db = QSqlDatabase::addDatabase("QMYSQL", "mySqlDatabase");
	m_db.setHostName(gEnv->mySqlHost);
	m_db.setPort(gEnv->mySqlPort);
	m_db.setDatabaseName(gEnv->mySqlDbName);
	m_db.setUserName(gEnv->mySqlUsername);
	m_db.setPassword(gEnv->mySqlPassword);

	return m_db.open();
}

QSqlDatabase MySqlConnector::GetDatabase()
{
	return m_db;
}


