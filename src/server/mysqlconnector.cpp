#include "mysqlconnector.h"
#include "global.h"
#include <QDebug>
#include <QSqlDatabase>

MySqlConnector::MySqlConnector(QObject *parent)
{
	connectStatus = false;
	m_thread = 0;
}

void MySqlConnector::run()
{
	if (Connect())
	{
		qInfo() << "MySQL connected!";

		m_thread = QThread::currentThread();

		qInfo() << "MySQL work on " << m_thread;
	}
	else
	{
		qCritical() << "Failed connect to MySQL! MySQL functions not be work!";
		return;
	}
}

bool MySqlConnector::Connect()
{
	qDebug() << "Connecting to MySql host...(" << gEnv->mySqlHost << ":" << gEnv->mySqlPort << ")";

	QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "mySqlDatabase");
	db.setHostName(gEnv->mySqlHost);
	db.setPort(gEnv->mySqlPort);
	db.setDatabaseName(gEnv->mySqlDbName);
	db.setUserName(gEnv->mySqlUsername);
	db.setPassword(gEnv->mySqlPassword);
	
	return db.open();
}
