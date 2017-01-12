// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef CLIENTQUERYS_H
#define CLIENTQUERYS_H

#include <QObject>

#include "redisconnector.h"
#include "global.h"
#include "netpacket.h"

class ClientQuerys : public QObject
{
    Q_OBJECT
public:
    explicit ClientQuerys(QObject *parent = 0);

public:
	void SetSocket(QSslSocket* socket) { this->m_socket = socket; }
	void SetClient(SClient* client);
    // Auth system
    void onLogin(NetPacket &packet);
    void onRegister(NetPacket &packet);
    // Profile querys
    void onCreateProfile(NetPacket &packet);
    void onGetProfile();
    // Shop querys
    void onGetShopItems();
    void onBuyItem(NetPacket &packet);
	void onRemoveItem(NetPacket &packet);
    // Friend system
    void onAddFriend(NetPacket &packet);
    void onRemoveFriend(NetPacket &packet);
	// Chat system
	void onChatMessage(NetPacket &packet);
	// Invite system
	void onInvite(NetPacket &packet);
	void onDeclineInvite(NetPacket &packet);
	// Matchmaking system
	void onGetGameServer(NetPacket &packet);
private:
	// Profile
    bool UpdateProfile(SProfile* profile);
	// Depricated. TODO - Remove this
	SShopItem GetShopItemByName(QString name);
private:
	QSslSocket* m_socket;
	SClient* m_Client;

	int startMoney;
private:
	bool bAuthorizated;
	bool bRegistered;
	bool bProfileCreated;
};



#endif // CLIENTQUERYS_H
