// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>

#include "global.h"

class CTcpPacket;
class TcpConnection;

class ClientQuerys : public QObject
{
    Q_OBJECT
public:
    explicit ClientQuerys(QObject *parent = nullptr);
	~ClientQuerys();
public:
	void           SetSocket(QSslSocket* socket) { this->m_socket = socket; }
	void           SetClient(SClient* client);
	void           SetConnection(TcpConnection* connection) { this->m_Connection = connection; }

	bool           ReadPacket(CTcpPacket &packet);
	bool           ReadQuery(CTcpPacket &packet);
	
	void           onLogin(CTcpPacket &packet);
	void           onRegister(CTcpPacket &packet);
	
	void           onCreateProfile(CTcpPacket &packet);
	void           onGetProfile() const;
	
	void           onGetShopItems();
	void           onBuyItem(CTcpPacket &packet);
	void           onRemoveItem(CTcpPacket &packet);
	
	void           onAddFriend(CTcpPacket &packet);
	void           onRemoveFriend(CTcpPacket &packet);

	void           onChatMessage(CTcpPacket &packet);

	void           onInvite(CTcpPacket &packet);
	void           onDeclineInvite(CTcpPacket &packet);
	
	void           onGetGameServer(CTcpPacket &packet) const;
private:
	bool           UpdateProfile(SProfile* profile) const;
	// Depricated. TODO - Remove this
	SShopItem      GetShopItemByName(const QString &name);
private:
	QSslSocket*    m_socket;
	SClient*       m_Client;
	TcpConnection* m_Connection;
	bool           bAuthorizated;
	bool           bRegistered;
	bool           bProfileCreated;
};