// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QObject>
#include <QSslSocket>
#include <QTime>

#include "Workers/Packets/clientquerys.h"

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = 0);
    ~TcpConnection();

	int IdleTime();

signals:
	void opened();
	void closed();
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
protected:
	void Active();
	QSslSocket* CreateSocket();

	QTime m_activity;
    ClientQuerys* pQuery;
	QSslSocket* m_Socket;
	SClient m_Client;

	bool bConnected;
};

#endif // TCPCONNECTION_H
