// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "global.h"
#include "clientquerys.h"
#include "redisconnector.h"
#include "tcpserver.h"

TcpServer* pServer;
RedisConnector* pRedis;
QVector <SClient> vClients;
