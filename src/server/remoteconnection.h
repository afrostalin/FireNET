// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

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

protected:
	virtual void connected();
	virtual void disconnected();
	virtual void readyRead();
	virtual void bytesWritten(qint64 bytes);
public slots:
	virtual void accept(qint64 socketDescriptor);
	virtual void close();
signals:
	void finished();
private:
	QSslSocket* m_socket;
	RemoteClientQuerys* pQuerys;
	SRemoteClient m_Client;
};

#endif // REMOTECONNECTION_H