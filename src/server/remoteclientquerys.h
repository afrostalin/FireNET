// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef REMOTECLIENTQUERYS_H
#define REMOTECLIENTQUERYS_H

#include <QObject>
#include <QSslSocket>

class RemoteClientQuerys : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClientQuerys(QObject *parent = 0);
	~RemoteClientQuerys();
	void SetSocket(QSslSocket* socket) { this->m_socket = socket; }
public:
	void onAdminLogining(QByteArray &bytes);
	void onConsoleCommandRecived(QByteArray &bytes);
private:
	QSslSocket* m_socket;
public:
	bool isAdmin;
	bool isGameServer;
};

#endif // REMOTECLIENTQUERYS_H