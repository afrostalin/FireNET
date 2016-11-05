// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

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
protected:
    virtual void connected();
    virtual void disconnected();
    virtual void readyRead();
    virtual void bytesWritten(qint64 bytes);
    virtual void stateChanged(QAbstractSocket::SocketState socketState);

signals:
    void finished();

public slots:
    virtual void accept(qint64 socketDescriptor);
    virtual void close();

	void socketSslErrors(const QList<QSslError> list);
	void socketError(QAbstractSocket::SocketError error);
private:
    ClientQuerys* pQuery;
	QSslSocket* m_Socket;
	SClient m_Client;
};

#endif // TCPCONNECTION_H
