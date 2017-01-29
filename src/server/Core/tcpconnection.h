// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QObject>
#include <QSslSocket>
#include <QTime>

#include <queue>

#include "global.h"
#include "netpacket.h"

class ClientQuerys;

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = 0);
    ~TcpConnection();

	void SendMessage(NetPacket& packet);
signals:
	void opened();
	void closed();

	void received();
	void sended();
public slots:
	virtual void quit();
    virtual void accept(qint64 socketDescriptor);
	virtual void connected();
	virtual void disconnected();
	virtual void readyRead();
	virtual void bytesWritten(qint64 bytes);
	virtual void stateChanged(QAbstractSocket::SocketState socketState);

	virtual void socketSslErrors(const QList<QSslError> list);
	virtual void socketError(QAbstractSocket::SocketError error);

	void Update();
private:
	QSslSocket* CreateSocket();
	void CalculateStatistic();
private:
    ClientQuerys* pQuery;
	QSslSocket* m_Socket;
	SClient m_Client;

	std::queue<NetPacket> m_Packets;
private:
	int m_maxPacketSize;
	int m_maxBadPacketsCount;
	int m_BadPacketsCount;

	QTime m_Time;
	int   m_InputPacketsCount;
	int   m_PacketsSpeed;
	int   m_maxPacketSpeed;

	bool bConnected;
	bool bIsQuiting;
	bool bLastMsgSended;
};

#endif // TCPCONNECTION_H
