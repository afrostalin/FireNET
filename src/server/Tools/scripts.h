// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

#include <QObject>
#include "global.h"

class Scripts : public QObject
{
    Q_OBJECT
public:
    explicit Scripts(QObject *parent = nullptr);
	~Scripts();
public:
	void                        Clear();
	void                        ReloadAll();
	void                        LoadShopScript();
	void                        LoadTrustedServerList();
	void                        LoadGameMaps();
public:
	std::vector<SShopItem>      GetShop() const;
	std::vector<STrustedServer> GetTrustedList() const;
	std::vector<SGameMap>       GetGameMaps() const;
private:
	std::vector<SShopItem>      m_shop;
	std::vector<STrustedServer> m_trustedServers;
	std::vector<SGameMap>       m_GameMaps;
};