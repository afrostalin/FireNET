// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef CLIENTQUERYS_H
#define CLIENTQUERYS_H

#include <QObject>
#include <QDebug>
#include <QSslSocket>
#include <QXmlStreamReader>
#include <QFile>

#include "redisconnector.h"
#include "global.h"

class ClientQuerys : public QObject
{
    Q_OBJECT
public:
    explicit ClientQuerys(QObject *parent = 0);

public:
	void SetSocket(QSslSocket* socket) { this->m_socket = socket; }
	void SetClient(SClient* client);
    // Auth system
    void onLogin(QByteArray &bytes);
    void onRegister(QByteArray &bytes);
    // Profile querys
    void onCreateProfile(QByteArray &bytes);
    void onGetProfile();
    // Shop querys
    void onGetShopItems();
    void onBuyItem(QByteArray &bytes);
	void onRemoveItem(QByteArray &bytes);
    // Friend system
    void onAddFriend(QByteArray &bytes);
    void onRemoveFriend(QByteArray &bytes);
	// Chat system
	void onChatMessage(QByteArray &bytes);
	// Invite system
	void onInvite(QByteArray &bytes);
	void onDeclineInvite(QByteArray &bytes);
	// Matchmaking system
	void onGetGameServer(QByteArray &bytes);
private:
	QXmlStreamAttributes GetAttributesFromArray(QByteArray &bytes);

    bool UpdateProfile(SProfile* profile);
	QString ProfileToString(SProfile* profile);

	SShopItem GetShopItemByName(QString name);

	bool CheckAttributeInRow(QString source, QString tag, QString attribute, QString checkAttribute);
	QString RemoveElementFromRow(QString source, QString element);	
private:
	QSslSocket* m_socket;
	SClient* m_Client;

	int startMoney;
};



#endif // CLIENTQUERYS_H
