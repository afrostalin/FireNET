// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef REMOTECLIENTQUERYS_H
#define REMOTECLIENTQUERYS_H

#include <QObject>
#include <QSslSocket>
#include <QXmlStreamReader>

#include "global.h"

class RemoteClientQuerys : public QObject
{
    Q_OBJECT
public:
    explicit RemoteClientQuerys(QObject *parent = 0);
	~RemoteClientQuerys();
	void SetSocket(QSslSocket* socket) { this->m_socket = socket; }
	void SetClient(SRemoteClient* client);
public:
	// Administration functional
	void onAdminLogining(QByteArray &bytes);
	void onConsoleCommandRecived(QByteArray &bytes);
	// Game server functionality
	void onGameServerRegister(QByteArray &bytes);
	void onGameServerUpdateInfo(QByteArray &bytes);

	void onGameServerGetOnlineProfile(QByteArray &bytes);
	void onGameServerUpdateOnlineProfile(QByteArray &bytes);
private:
	QXmlStreamAttributes GetAttributesFromArray(QByteArray &bytes);
	QString ProfileToString(SProfile* profile);
private:
	QSslSocket* m_socket;
	SRemoteClient* m_client;
};

#endif // REMOTECLIENTQUERYS_H