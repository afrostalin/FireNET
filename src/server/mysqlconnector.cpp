#include "mysqlconnector.h"
#include "dbworker.h"
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
	qDebug() << "Connecting to MySql host...(" << gEnv->pDataBases->mySqlHost << ":" << gEnv->pDataBases->mySqlPort << ")";

	m_db = QSqlDatabase::addDatabase("QMYSQL", "mySqlDatabase");
	m_db.setHostName(gEnv->pDataBases->mySqlHost);
	m_db.setPort(gEnv->pDataBases->mySqlPort);
	m_db.setDatabaseName(gEnv->pDataBases->mySqlDbName);
	m_db.setUserName(gEnv->pDataBases->mySqlUsername);
	m_db.setPassword(gEnv->pDataBases->mySqlPassword);

	return m_db.open();
}

QSqlDatabase MySqlConnector::GetDatabase()
{
	return m_db;
}


