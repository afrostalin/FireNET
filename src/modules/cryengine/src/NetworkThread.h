// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _Network_H_
#define _Network_H_

#include <QObject>
#include <QSslSocket>
#include <QCoreApplication>

#include "NetPacket.h"

class CNetwork : public QObject
{
	Q_OBJECT
public:
	explicit CNetwork(QObject *parent = 0);
public slots:
	void onConnectedToServer();
	void onReadyRead();
	void onBytesWritten(qint64 bytes);
	void onDisconnected();
private:
	QSslSocket* m_socket;
public:
	void Init();
	void ConnectToServer();
	void CloseConnection();
public:
	bool SendQuery(NetPacket &packet);
	bool SendSyncQuery(NetPacket &packet, float timeout);

#ifdef DEDICATED_SERVER
public:
	bool RegisterGameServer();
	bool UpdateGameServer();
#endif

public:
	bool isInit() { return bInit; }
private:
	bool bInit; 
};

#endif
