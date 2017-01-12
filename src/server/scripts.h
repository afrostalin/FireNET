// Copyright (Ñ) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#ifndef SCRIPTS_H
#define SCRIPTS_H

#include <QObject>
#include "global.h"

class Scripts : public QObject
{
    Q_OBJECT
public:
    explicit Scripts(QObject *parent = 0);
	void Clear();
public:
	void LoadShopScript();
	void LoadTrustedServerList();
public:
	QVector<SShopItem> GetShop();
	QVector<STrustedServer> GetTrustedList();
private:
	QVector<SShopItem> m_shop;
	QVector<STrustedServer> m_trustedServers;
};

#endif // SCRIPTS_H