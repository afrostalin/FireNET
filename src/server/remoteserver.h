// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <QObject>
#include <QThread>
#include <QTcpServer>
#include <QSslSocket>

class RemoteConnection;

class RemoteServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit RemoteServer(QObject *parent = 0);
public:
	virtual void run();
private:
	bool CreateServer();
public:
    virtual void incomingConnection(qintptr socketDescriptor);
	void sendMessageToRemoteClient(QSslSocket* socket, QByteArray data);
public:
	QString serverIp;
	int serverPort;
	int clientCount;
	bool bHaveAdmin;
private:
	QTcpServer* m_server;
	QVector<RemoteConnection*> m_connections;
};

#endif // REMOTESERVER_H