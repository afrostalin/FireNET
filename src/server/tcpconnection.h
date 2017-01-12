// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QObject>
#include <QThread>
#include <QSslSocket>
#include <QList>
#include "clientquerys.h"

class TcpConnection : public QObject
{
    Q_OBJECT
public:
    explicit TcpConnection(QObject *parent = 0);
    ~TcpConnection();
signals:
    void finished();

public slots:
    virtual void accept(qint64 socketDescriptor);
    virtual void close();

	void connected();
	void disconnected();
	void readyRead();
	void bytesWritten(qint64 bytes);
	void stateChanged(QAbstractSocket::SocketState socketState);

	void socketSslErrors(const QList<QSslError> list);
	void socketError(QAbstractSocket::SocketError error);
private:
    ClientQuerys* pQuery;
	QSslSocket* m_Socket;
	SClient m_Client;
	bool bConnected;
};

#endif // TCPCONNECTION_H
