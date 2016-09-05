#include "global.h"
#include "clientquerys.h"
#include "redisconnector.h"
#include "tcpserver.h"

TcpServer* pServer;
RedisConnector* pRedis;
QVector <SClient> vClients;
//QMutex m_globalMutex;
