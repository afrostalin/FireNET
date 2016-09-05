#include "redisconnector.h"
#include "global.h"

RedisConnector::RedisConnector(QObject *parent)
{
	connection = 0; 
    connectStatus = false;
	m_thread = 0;
	m_loop = 0;
}

void RedisConnector::run()
{
	if (Connect())
	{
		qDebug() << "[RedysConnector] Redis connected!";

		m_thread = QThread::currentThread();

		qDebug() << "[RedysConnector] Redis work on " << m_thread;
		//m_loop = new QEventLoop();
		//m_loop->exec();
	}
	else
	{
		qDebug() << "[RedysConnector] Failed connect to redis! Database functions not be work!";
		return;
	}
}

bool RedisConnector::Connect()
{
    qDebug()<< "[RedysConnector] Init qredisclient...";
    initRedisClient();

    qDebug()<< "[RedysConnector] Create connection to local redis...";
    RedisClient::ConnectionConfig config("127.0.0.1");
    connection = new RedisClient::Connection(config);
    connection->connect(false);

    connectStatus = connection->isConnected();

    return connection->isConnected();
}

QString RedisConnector::SendSyncQuery(QString command, QString key, QString value)
{
	RedisClient::Response r;

    if(connectStatus && connection != nullptr)
    {
		if (command == "SET")
		{
			r = connection->commandSync(command, key, value);
			return r.getValue().toString();
		}
		else
		{
			if (command == "GET")
			{
				r = connection->commandSync(command, key);
				return r.getValue().toString();
			}
			else
			{
				r = connection->commandSync(command);
				return r.getValue().toString();
			}
		}
	}
    else
        qDebug() << "[RedysConnector] RedisConnector::SendSyncQuery - Error!";

    return QString();
}
