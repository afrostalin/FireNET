#ifndef MYSQLCONNECTOR_H
#define MYSQLCONNECTOR_H

#include <QObject>
#include <QThread>

class MySqlConnector : public QObject
{
	Q_OBJECT
public:
	explicit MySqlConnector(QObject *parent = 0);
public:
	void run();
private:
	bool Connect();
public:
	bool connectStatus;
private:
	QThread *m_thread;
};

#endif // MYSQLCONNECTOR_H