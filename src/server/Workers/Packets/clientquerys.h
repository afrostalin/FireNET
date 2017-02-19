// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef CLIENTQUERYS_H
#define CLIENTQUERYS_H

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
	
	void           onLogin(CTcpPacket &packet);
	void           onRegister(CTcpPacket &packet);
	
	void           onCreateProfile(CTcpPacket &packet);
	void           onGetProfile();
	
	void           onGetShopItems();
	void           onBuyItem(CTcpPacket &packet);
	void           onRemoveItem(CTcpPacket &packet);
	
	void           onAddFriend(CTcpPacket &packet);
	void           onRemoveFriend(CTcpPacket &packet);

	void           onChatMessage(CTcpPacket &packet);

	void           onInvite(CTcpPacket &packet);
	void           onDeclineInvite(CTcpPacket &packet);
	
	void           onGetGameServer(CTcpPacket &packet);
private:
	bool           UpdateProfile(SProfile* profile);
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
#endif // CLIENTQUERYS_H
