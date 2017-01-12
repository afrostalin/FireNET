// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef REMOTECONNECTION_H
#define REMOTECONNECTION_H

#include <QObject>
#include <QSslSocket>

#include "remoteclientquerys.h"

class RemoteConnection : public QObject
{
    Q_OBJECT
public:
    explicit RemoteConnection(QObject *parent = 0);
	~RemoteConnection();
public slots:
	void accept(qint64 socketDescriptor);
	void close();
	//
	void connected();
	void disconnected();
	void readyRead();
	void bytesWritten(qint64 bytes);

	void socketSslErrors(const QList<QSslError> list);
	void socketError(QAbstractSocket::SocketError error);
signals:
	void finished();
private:
	QSslSocket* m_socket;
	RemoteClientQuerys* pQuerys;
	SRemoteClient m_Client;
	bool bConnected;
};

#endif // REMOTECONNECTION_H